-- Copyright (c) 2012-2026 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT

-- Based on Toyful Games' car physics tutorial: https://www.youtube.com/watch?v=CdPYlj5uZeI

require "core/lua/class"

Vehicle = class(Vehicle)

local LOOP_RPM = { 1746, 2616, 3492, 4400, 5232, 6222, 6984 }
local TIRE_SCREECH_HANDOFF = { 1.39, 1.47, 1.79, 1.59 }
local GEAR_RATIOS = { 3.66, 2.02, 1.40, 1.08, 0.87, 0.745, 0.675, 0.59 }
local FINAL_DRIVE_RATIO = 5
local MAX_ENGINE_TORQUE = 180
local TORQUE_CURVE = { 0.55, 0.62, 0.68, 0.75, 0.83, 0.90, 0.95, 1, 1, 0.95, 0.85 }
local IDLE_RPM = 900
local MAX_RPM = 7000
local LAUNCH_LOCK_SPEED = 4
local UPSHIFT_RPM = 6700
local HIGH_REV_RPM = 6500
local HIGH_REV_HOLD = { 0.22, 0.20, 0.18 }
local SHIFT_INTERVAL = 0.45
local SHIFT_DURATION = 0.40
local SHIFT_RELEASE_TIME = 0.05
local SHIFT_SYNC_TIME = 0.15
local SHIFT_ENGAGE_TIME = 0.17
local SHIFT_GAIN_FADE_START = 25
local SHIFT_GAIN_FADE_END = 50
local TIRE_GRIP_REFERENCE_HZ = 120
local MIN_PITCH = 0.45
local MAX_PITCH = 2.5

local function evaluate_curve(curve, value)
	local t = math.max(0, math.min(1, value)) * (#curve - 1)
	local index = math.floor(t) + 1
	local a = curve[index]
	local b = curve[index + 1] or a
	return a + (b - a) * (t % 1)
end

local function reset_drivetrain(self)
	self._gear = 1
	self._rpm = IDLE_RPM
	self._shift_time = 0
	self._high_rev_time = 0
	self._engine_load = 0
end

local function shift_gear(self, gear, wheel_speed)
	self._shift_from_gear = self._gear
	self._gear = gear
	self._shift_time = SHIFT_INTERVAL
	self._shift_rpm = self._rpm
	self._shift_speed = wheel_speed
end

local function update_drivetrain(self, dt, wheel_speed, coupling, throttle, at_rest)
	local wheel_rpm = math.abs(wheel_speed) * self._wheel_rpm_per_speed
	local coupled_rpm = wheel_rpm * GEAR_RATIOS[self._gear] * FINAL_DRIVE_RATIO
	local upshift_rpm = math.min(self._rpm, coupled_rpm)
	local engaged = coupling >= 0.5
	local load = math.abs(throttle)

	-- Hold low gears at high revs.
	self._shift_time = math.max(0, self._shift_time - dt)
	local can_shift = engaged and self._shift_time == 0
	local hold_time = load > 0.5 and HIGH_REV_HOLD[self._gear] or 0
	if hold_time > 0 and can_shift and wheel_speed > 0 and upshift_rpm >= HIGH_REV_RPM then
		self._high_rev_time = self._high_rev_time + dt
	else
		self._high_rev_time = 0
	end

	-- Select a gear from engine and wheel speed.
	if at_rest or (engaged and wheel_speed < -0.5) then
		self._gear = 1
		self._shift_time = 0
	elseif can_shift then
		if throttle > 0 and wheel_speed > 0 and self._gear < #GEAR_RATIOS
			and upshift_rpm >= UPSHIFT_RPM and self._high_rev_time >= hold_time
		then
			shift_gear(self, self._gear + 1, wheel_speed)
		elseif self._rpm < 3000 and coupled_rpm < 3000 and self._gear > 1 then
			shift_gear(self, self._gear - 1, wheel_speed)
		end
	end

	local response = 1 - math.exp(-12 * dt)
	local engine_load = load * (0.55 + 0.45 * coupling)
	self._engine_load = self._engine_load + (engine_load - self._engine_load) * response
	if at_rest and load == 0 then
		self._rpm = self._rpm + (IDLE_RPM - self._rpm) * response
		return 0, 1
	end

	-- Estimate free and wheel-driven engine RPM.
	local drive_ratio = GEAR_RATIOS[self._gear] * FINAL_DRIVE_RATIO
	coupled_rpm = math.max(IDLE_RPM + load * 600, math.min(MAX_RPM, wheel_rpm * drive_ratio))
	local free_target = IDLE_RPM + load * (MAX_RPM - IDLE_RPM)
	local free_rate = free_target > self._rpm and 4 or 1.1
	local free_rpm = self._rpm + (free_target - self._rpm) * (1 - math.exp(-free_rate * dt))

	-- Match engine revs to the new gear.
	local shift_elapsed = SHIFT_INTERVAL - self._shift_time
	local shift_gain = 1
	local shift_clutch = 1
	local driven_rpm
	if shift_elapsed < SHIFT_DURATION then
		local sync = math.max(0, math.min(1, (shift_elapsed - SHIFT_RELEASE_TIME) / SHIFT_SYNC_TIME))
		local blend = sync * sync * (3 - 2 * sync)
		driven_rpm = self._shift_rpm + (coupled_rpm - self._shift_rpm) * blend

		local disengaged = math.min(1, shift_elapsed / SHIFT_RELEASE_TIME, (SHIFT_DURATION - shift_elapsed) / SHIFT_ENGAGE_TIME)
		shift_clutch = 1 - disengaged
		if shift_elapsed < SHIFT_RELEASE_TIME then
			drive_ratio = GEAR_RATIOS[self._shift_from_gear] * FINAL_DRIVE_RATIO
		end

		-- Lower engine volume during the shift.
		local speed_blend = (self._shift_speed - SHIFT_GAIN_FADE_START) / (SHIFT_GAIN_FADE_END - SHIFT_GAIN_FADE_START)
		local dip = (0.25 + 0.25 * self._engine_load) * (1 - 0.8 * math.max(0, math.min(1, speed_blend)))
		shift_gain = 1 - disengaged * dip
	else
		driven_rpm = self._rpm + (coupled_rpm - self._rpm) * response
	end

	-- Let the clutch slip when pulling away.
	local launch_clutch = 1
	if self._gear == 1 then
		local slip = math.max(0, 1 - math.abs(wheel_speed) / LAUNCH_LOCK_SPEED)
		launch_clutch = 1 - 0.6 * load * slip
	end
	self._rpm = free_rpm + (driven_rpm - free_rpm) * launch_clutch * coupling

	-- Convert engine torque to force at each tire.
	local normalized_rpm = (self._rpm - IDLE_RPM) / (MAX_RPM - IDLE_RPM)
	local engine_torque = evaluate_curve(TORQUE_CURVE, normalized_rpm) * MAX_ENGINE_TORQUE * throttle
	local wheel_force = engine_torque * drive_ratio * launch_clutch * shift_clutch / (#self._wheels * self._tire_radius)
	return wheel_force, shift_gain
end

local function update_engine_sound(self, position, shift_gain)
	local volume = (0.5 + 0.5 * self._engine_load) * shift_gain

	-- Fade each loop to silence at its neighbors' RPMs.
	for i, sound in ipairs(self._engine_sounds) do
		local rpm = LOOP_RPM[i]
		local lower, upper = LOOP_RPM[i - 1], LOOP_RPM[i + 1]
		local weight = 1
		if lower and self._rpm < rpm then
			weight = (self._rpm - lower) / (rpm - lower)
		elseif upper and self._rpm > rpm then
			weight = 1 - (self._rpm - rpm) / (upper - rpm)
		end
		local gain = math.sqrt(math.max(0, weight))
		World.set_sound_position(self._world, sound, position)
		World.set_sound_pitch(self._world, sound, math.max(MIN_PITCH, math.min(MAX_PITCH, self._rpm / rpm)))
		World.set_sound_volume(self._world, sound, gain * volume)
	end
end

function Vehicle:init(world, unit, camera)
	self._world = world
	self._unit = unit
	self._camera = camera
	self._physics_world = World.physics_world(world)
	self._scene_graph = World.scene_graph(world)
	local actor = PhysicsWorld.actor_instance(self._physics_world, unit)
	local pose = PhysicsWorld.actor_world_pose(self._physics_world, actor)
	self._spawn_pose = Matrix4x4Box(pose)
	self._camera_yaw = nil
	self._steer_angle = 0
	self._previous_velocity = Vector3Box(Vector3.zero())
	self._previous_angular_velocity = Vector3Box(Vector3.zero())
	self._crash_impact = 0
	self._crash_timer = 0
	self._grounded_wheels = 0
	self._wheels = {}
	self._left_wheel_rotation = QuaternionBox(Quaternion(Vector3.up(), math.pi))

	-- Tire and suspension settings.
	self._tire_mass = 200
	self._tire_radius = 0.34
	self._suspension_rest_distance = 0.94
	self._spring_strength = 18000
	self._spring_damper = 2200
	self._bump_stop_travel = 0.03
	self._bump_stop_strength = 100000
	self._bump_stop_damper = 2000
	self._top_speed = 75
	self._reverse_speed = 10
	self._brake_acceleration = 24
	self._engine_brake_acceleration = 3
	self._max_steer_angle = math.rad(42)

	-- Front and rear grip as sideways slip increases.
	self._front_grip_curve = { 0.150, 0.070, 0.058, 0.051, 0.046, 0.042, 0.039, 0.036, 0.034, 0.032, 0.031 }
	self._rear_grip_curve = { 0.120, 0.060, 0.053, 0.049, 0.046, 0.043, 0.041, 0.039, 0.037, 0.035, 0.034 }

	self._engine_sounds = {}
	self._wheel_rpm_per_speed = 60 / (2 * math.pi * self._tire_radius)
	local position = Matrix4x4.translation(pose)
	SoundWorld.set_group_volume(World.sound_world(world), "vehicle_engine", 1.6)
	self._tire_squeal_volume = 0
	self._tire_squeal_time = 0
	for i, rpm in ipairs(LOOP_RPM) do
		self._engine_sounds[i] = World.play_sound(world, "sfx/engine_" .. rpm, true, 0, 70, position, "vehicle_engine")
	end
	reset_drivetrain(self)

	-- Wheel mounts in chassis space.
	local mounts = {
		Vector3(-0.7960,  1.3891, 0),
		Vector3( 0.7913,  1.3891, 0),
		Vector3(-0.7863, -1.3891, 0),
		Vector3( 0.7911, -1.3891, 0),
	}
	local chassis_transform = SceneGraph.instance(self._scene_graph, unit)
	for i, mount in ipairs(mounts) do
		local front = mount.y > 0
		local wheel_unit = World.spawn_unit(world, "units/vehicle/wheel")
		local wheel_transform = SceneGraph.instance(self._scene_graph, wheel_unit)
		local wheel_offset = mount - Vector3.up() * (self._suspension_rest_distance - self._tire_radius)
		SceneGraph.link(self._scene_graph, chassis_transform, wheel_transform, wheel_offset)
		self._wheels[i] = {
			unit = wheel_unit,
			mount = Vector3Box(mount),
			front = front,
			left = mount.x < 0,
			min_distance = front and 0.80 or 0.77,
			spin = 0
		}
	end
end

function Vehicle:speed()
	local actor = PhysicsWorld.actor_instance(self._physics_world, self._unit)
	local velocity = PhysicsWorld.actor_linear_velocity(self._physics_world, actor)
	return math.sqrt(velocity.x * velocity.x + velocity.y * velocity.y)
end

function Vehicle:gear_and_rpm()
	return self._gear, self._rpm
end

function Vehicle:grounded_wheels()
	return self._grounded_wheels
end

function Vehicle:collision(actor, position, normal)
	if self._crash_impact == 0 and self._crash_timer > 0 then
		return
	end

	local pw = self._physics_world
	local offset = position - PhysicsWorld.actor_center_of_mass(pw, actor)
	if Vector3.dot(offset, normal) > 0 then
		normal = -normal
	end
	local velocity_change = PhysicsWorld.actor_linear_velocity(pw, actor) - self._previous_velocity:unbox()
	local angular_change = PhysicsWorld.actor_angular_velocity(pw, actor) - self._previous_angular_velocity:unbox()
	local point_change = velocity_change + Vector3.cross(angular_change, offset)
	local impact = math.max(Vector3.dot(velocity_change, normal), Vector3.dot(point_change, normal))
	if impact < 0.25 then
		return
	end

	-- Combine contacts from the same impact.
	if self._crash_impact == 0 then
		self._crash_timer = 0.025
	end
	self._crash_impact = math.max(self._crash_impact, impact)
end

function Vehicle:reset()
	reset_drivetrain(self)
	local actor = PhysicsWorld.actor_instance(self._physics_world, self._unit)
	local pose = self._spawn_pose:unbox()
	PhysicsWorld.actor_teleport_world_pose(self._physics_world, actor, pose)
	PhysicsWorld.actor_set_linear_velocity(self._physics_world, actor, Vector3.zero())
	PhysicsWorld.actor_set_angular_velocity(self._physics_world, actor, Vector3.zero())
	self._previous_velocity:store(Vector3.zero())
	self._previous_angular_velocity:store(Vector3.zero())
	self._crash_impact = 0
	self._crash_timer = 0
	local transform = SceneGraph.instance(self._scene_graph, self._unit)
	SceneGraph.set_local_pose(self._scene_graph, transform, pose)
	self._steer_angle = 0
	self._camera_yaw = nil
end

function Vehicle:update(dt, throttle, steering, brake, handbrake)
	if dt <= 0 then
		return
	end

	local pw = self._physics_world
	local actor = PhysicsWorld.actor_instance(pw, self._unit)
	local pose = PhysicsWorld.actor_world_pose(pw, actor)
	local transform = SceneGraph.instance(self._scene_graph, self._unit)
	local position = Matrix4x4.translation(pose)
	local rotation = Matrix4x4.rotation(pose)
	local up = Matrix4x4.z(pose)
	local center = PhysicsWorld.actor_center_of_mass(pw, actor)
	local velocity = PhysicsWorld.actor_linear_velocity(pw, actor)
	local angular_velocity = PhysicsWorld.actor_angular_velocity(pw, actor)
	self._previous_velocity:store(velocity)
	self._previous_angular_velocity:store(angular_velocity)
	self._crash_timer = math.max(0, self._crash_timer - dt)
	if self._crash_impact > 0 and self._crash_timer == 0 then
		local sound = "sfx/crash_hard_" .. math.random(4)
		local volume = 0.5 * math.min(1, 0.15 + self._crash_impact / 8)
		World.play_sound(self._world, sound, false, volume, 70, position)
		self._crash_impact = 0
		self._crash_timer = 0.1
	end
	local chassis_forward = Matrix4x4.y(pose)
	local longitudinal_speed = Vector3.dot(velocity, chassis_forward)
	local speed_magnitude = Vector3.length(velocity)

	-- Reduce steering range at high speed.
	local steer_speed = speed_magnitude / 15
	local high_speed = math.max(0, speed_magnitude - 10)
	local steer_limit = self._max_steer_angle / ((1 + steer_speed * steer_speed) * (1 + high_speed * 0.015))
	local target_steer = math.max(-1, math.min(1, steering)) * steer_limit
	self._steer_angle = self._steer_angle + (target_steer - self._steer_angle) * (1 - math.exp(-10 * dt))

	-- Brake before changing direction.
	throttle = math.max(-1, math.min(1, throttle))
	local braking = brake or throttle * longitudinal_speed < -0.5
	local top_speed = throttle > 0 and self._top_speed or self._reverse_speed
	local can_drive = throttle ~= 0 and math.abs(longitudinal_speed) < top_speed
	local upright = up.z > 0.5
	local rolling_speed = 0
	local contact_weight = 0
	local squeal = 0
	local grounded_wheels = {}
	local steer_rotation = Quaternion(Vector3.up(), -self._steer_angle)
	local front_rotation = Quaternion.multiply(rotation, steer_rotation)
	local ray_length = self._suspension_rest_distance

	for _, wheel in ipairs(self._wheels) do
		local wheel_handbrake = handbrake and not wheel.front
		local mount_offset = wheel.mount:unbox()
		local mount = Matrix4x4.transform(pose, mount_offset)
		local tire_rotation = wheel.front and front_rotation or rotation
		local right = Quaternion.right(tire_rotation)
		local forward = Quaternion.forward(tire_rotation)
		local tire_velocity = velocity + Vector3.cross(angular_velocity, mount - center)
		local forward_speed = Vector3.dot(tire_velocity, forward)

		-- Raycast from the mount to measure suspension travel.
		local distance = ray_length
		local hit, _, normal, time, _, hit_actor = PhysicsWorld.cast_ray(pw, mount, -up, ray_length)
		if hit and hit_actor ~= actor then
			distance = time * ray_length
			local supports_wheel = Vector3.dot(normal, up) >= 0.5
			local compression = ray_length - distance
			local spring_speed = Vector3.dot(tire_velocity, up)
			local spring_force = compression * self._spring_strength - spring_speed * self._spring_damper

			-- Stiffen the suspension near full compression.
			local bump_travel = wheel.min_distance + self._bump_stop_travel - distance
			if bump_travel > 0 and supports_wheel and PhysicsWorld.actor_is_static(pw, hit_actor) then
				local engagement = math.min(1, bump_travel / self._bump_stop_travel)
				local bump_force = bump_travel * self._bump_stop_strength - math.min(0, spring_speed) * self._bump_stop_damper
				spring_force = spring_force + bump_force * engagement
			end

			-- Discount slipping tires when estimating rolling speed.
			local sideways_speed = Vector3.dot(tire_velocity, right)
			local slip = math.abs(sideways_speed) / math.max(Vector3.length(tire_velocity), 0.1)
			if upright and not wheel_handbrake and supports_wheel then
				local weight = math.max(0, 1 - 1.5 * slip)
				rolling_speed = rolling_speed + forward_speed * weight
				contact_weight = contact_weight + weight
			end

			-- Use tire grip to resist sideways motion.
			local grip = evaluate_curve(wheel.front and self._front_grip_curve or self._rear_grip_curve, slip)
			if wheel_handbrake then
				grip = grip * 0.1
			end
			grip = 1 - (1 - grip) ^ (dt * TIRE_GRIP_REFERENCE_HZ)
			local sideways_force = -sideways_speed * grip / dt * self._tire_mass

			-- Measure tire slip on supporting contacts.
			if upright and supports_wheel and spring_force > 0 then
				local tire_squeal
				if wheel_handbrake then
					local slide_speed = math.sqrt(sideways_speed * sideways_speed + forward_speed * forward_speed)
					tire_squeal = (slide_speed - 1.5) / 7
				else
					local speed_gain = math.min(1, math.max(0, (math.abs(sideways_speed) - 0.35) / 1.15))
					tire_squeal = (slip - 0.08) / 0.22 * speed_gain
				end
				squeal = math.max(squeal, tire_squeal)
			end

			-- Use engine braking when coasting.
			local brake_force = 0
			local wheel_braking = braking or wheel_handbrake
			if wheel_braking or throttle == 0 then
				local brake_acceleration = wheel_braking and self._brake_acceleration or self._engine_brake_acceleration
				local acceleration = math.max(-brake_acceleration, math.min(brake_acceleration, -forward_speed / dt))
				brake_force = acceleration * self._tire_mass
			end

			grounded_wheels[#grounded_wheels + 1] = {
				mount = mount,
				force = up * spring_force + right * sideways_force + forward * brake_force,
				drive_direction = can_drive and not wheel_braking and forward
			}
		end

		-- Animate wheel spin and suspension travel.
		if not wheel_handbrake then
			wheel.spin = (wheel.spin - forward_speed / self._tire_radius * dt) % (2 * math.pi)
		end
		local spin_rotation = Quaternion(Vector3(1, 0, 0), wheel.spin)
		local wheel_rotation = wheel.front and Quaternion.multiply(steer_rotation, spin_rotation) or spin_rotation
		if wheel.left then
			wheel_rotation = Quaternion.multiply(wheel_rotation, self._left_wheel_rotation:unbox())
		end
		local wheel_transform = SceneGraph.instance(self._scene_graph, wheel.unit)
		local wheel_distance = math.max(wheel.min_distance, distance) - self._tire_radius
		SceneGraph.set_local_position(self._scene_graph, wheel_transform, mount_offset - Vector3.up() * wheel_distance)
		SceneGraph.set_local_rotation(self._scene_graph, wheel_transform, wheel_rotation)
	end

	-- Feed rolling speed and traction back to the drivetrain.
	self._grounded_wheels = #grounded_wheels
	local wheel_speed = contact_weight > 0 and rolling_speed / contact_weight or 0
	local coupling = contact_weight / #self._wheels
	if brake or (braking and coupling >= 0.5) then
		throttle = 0
	end
	local wheel_force, shift_gain = update_drivetrain(self, dt, wheel_speed, coupling, throttle, speed_magnitude < 0.25)

	-- Apply each wheel's force to the chassis.
	for _, wheel in ipairs(grounded_wheels) do
		local force = wheel.force
		if wheel.drive_direction then
			force = force + wheel.drive_direction * wheel_force
		end
		PhysicsWorld.actor_push_at(pw, actor, force, 1, wheel.mount)
	end
	update_engine_sound(self, position, shift_gain)

	self._tire_squeal_volume = self._tire_squeal_volume
		+ (math.min(1, squeal) - self._tire_squeal_volume) * (1 - math.exp(-36 * dt))
	local volume = self._tire_squeal_volume * 0.3

	local sound = self._tire_squeal
	if sound and not SoundWorld.is_playing(World.sound_world(self._world), sound) then
		sound = nil
		self._tire_squeal = nil
	end
	self._tire_squeal_time = math.max(0, self._tire_squeal_time - dt)
	if sound and self._tire_squeal_volume < 0.01 then
		World.stop_sound(self._world, sound)
		self._tire_squeal = nil
	elseif self._tire_squeal_volume > 0.02 and (not sound or self._tire_squeal_time <= 0) then
		-- Start the next clip over the fade-out.
		local clip = math.random(#TIRE_SCREECH_HANDOFF)
		self._tire_squeal = World.play_sound(self._world, "sfx/tire_screech_" .. clip, false, volume, 40, position)
		self._tire_squeal_time = TIRE_SCREECH_HANDOFF[clip]
	elseif sound then
		World.set_sound_position(self._world, sound, position)
		World.set_sound_volume(self._world, sound, volume)
	end

	-- Ease the chase camera into turns.
	if self._camera then
		local render_pose = SceneGraph.world_pose(self._scene_graph, transform)
		local chassis_forward = Matrix4x4.y(render_pose)
		chassis_forward.z = 0
		if Vector3.length(chassis_forward) > 0.001 then
			Vector3.normalize(chassis_forward)
		else
			chassis_forward = Vector3.forward()
		end
		local heading = math.atan2(chassis_forward.y, chassis_forward.x)
		local yaw = self._camera_yaw or heading
		local angle = (heading - yaw + math.pi) % (2 * math.pi) - math.pi
		self._camera_yaw = yaw + angle * (1 - math.exp(-5 * dt))
		local camera_forward = Vector3(math.cos(self._camera_yaw), math.sin(self._camera_yaw), 0)
		local target = Matrix4x4.translation(render_pose) + Vector3(0, 0, 0.2)
		local desired_position = target - camera_forward * 5.5 + Vector3(0, 0, 1.6)
		local camera_transform = SceneGraph.instance(self._scene_graph, self._camera:unit())
		local view = Vector3.normalize(target - desired_position)
		SceneGraph.set_local_position(self._scene_graph, camera_transform, desired_position)
		SceneGraph.set_local_rotation(self._scene_graph, camera_transform, Quaternion.look(view))
	end

	if position.z < -10 then
		self:reset()
	end
end

function Vehicle:destroy()
	for _, sound in ipairs(self._engine_sounds) do
		World.stop_sound(self._world, sound)
	end
	if self._tire_squeal and SoundWorld.is_playing(World.sound_world(self._world), self._tire_squeal) then
		World.stop_sound(self._world, self._tire_squeal)
	end
	self._engine_sounds = {}
	for _, wheel in ipairs(self._wheels) do
		World.destroy_unit(self._world, wheel.unit)
	end
	self._wheels = {}
end

return Vehicle

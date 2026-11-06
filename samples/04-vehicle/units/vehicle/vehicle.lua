-- Copyright (c) 2012-2026 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT

local Vehicle = require "vehicle"

UnitsVehicleVehicle = UnitsVehicleVehicle or { data = {} }
local data = UnitsVehicleVehicle.data

function UnitsVehicleVehicle.spawned(world, units)
	local world_data = data[world] or {}
	data[world] = world_data
	for _, unit in pairs(units) do
		local vehicle = Vehicle(world, unit, Game.camera)
		world_data[unit] = { vehicle = vehicle }
		if unit == Game.vehicle_unit then
			Game.vehicle = vehicle
		end
	end
end

function UnitsVehicleVehicle.update(world, dt)
	local world_data = data[world]
	if world_data == nil then
		return
	end

	local steering = Keyboard.button(Keyboard.button_id("d")) - Keyboard.button(Keyboard.button_id("a"))
	local throttle = Keyboard.button(Keyboard.button_id("w")) - Keyboard.button(Keyboard.button_id("s"))
	local stick = Pad1.axis(Pad1.axis_id("left"))
	steering = steering + (math.abs(stick.x) > 0.15 and stick.x or 0)
	throttle = throttle + Pad1.axis(Pad1.axis_id("trigger_right")).z - Pad1.axis(Pad1.axis_id("trigger_left")).z
	local brake = Keyboard.button(Keyboard.button_id("ctrl_left")) ~= 0
		or Keyboard.button(Keyboard.button_id("ctrl_right")) ~= 0
		or Pad1.button(Pad1.button_id("b")) ~= 0
	local handbrake = Keyboard.button(Keyboard.button_id("space")) ~= 0 or Pad1.button(Pad1.button_id("a")) ~= 0
	local touch_pad = GameBase.virtual_joypad
	if touch_pad then
		local movement = touch_pad:axis(touch_pad:axis_id("left"))
		steering = steering + movement.x
		throttle = throttle + movement.y
		brake = brake or touch_pad:button(touch_pad:button_id("b")) ~= 0
		handbrake = handbrake or touch_pad:button(touch_pad:button_id("a")) ~= 0
	end

	for _, unit_data in pairs(world_data) do
		unit_data.vehicle:update(dt, throttle, steering, brake, handbrake)
	end
end

function UnitsVehicleVehicle.unspawned(world, units)
	local world_data = data[world]
	if world_data == nil then
		return
	end
	for _, unit in pairs(units) do
		local unit_data = world_data[unit]
		if unit_data then
			if Game.vehicle == unit_data.vehicle then
				Game.vehicle = nil
			end
			unit_data.vehicle:destroy()
			world_data[unit] = nil
		end
	end
	if next(world_data) == nil then
		data[world] = nil
	end
end

return UnitsVehicleVehicle

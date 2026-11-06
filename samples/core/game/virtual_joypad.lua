require "core/lua/class"

VirtualJoypad = class(VirtualJoypad)

local AXIS_IDS = { left = 0, right = 1, trigger_left = 2 }
local BUTTON_IDS = { a = 0, b = 1 }
local BUTTON_POSITIONS = { { 252, 156 }, { 156, 220 } }
local STICK_RADIUS = 64
local OUTER_RADIUS = 96
local STICK_CENTER = 180

function VirtualJoypad:init(controls)
	self.visible = false
	self._axes = {}
	for _, name in ipairs(controls.axes) do
		self._axes[AXIS_IDS[name]] = {}
	end
	self._buttons = {}
	for _, name in ipairs(controls.buttons or {}) do
		self._buttons[BUTTON_IDS[name]] = { name = name, down = false }
	end
	self._pointers = {}
	for i = 0, 3 do
		local name = "pointer_" .. i
		self._pointers[i + 1] = { button = Touch.button_id(name), axis = Touch.axis_id(name) }
	end
	self._width, self._height = Device.resolution()
	self:reset()
end

function VirtualJoypad:reset()
	self._stick_x, self._stick_y = 0, 0
	for _, axis in pairs(self._axes) do
		axis.x, axis.y, axis.z = 0, 0, 0
		axis.pointer = nil
	end
	for _, button in pairs(self._buttons) do
		button.released = button.down
		button.down = false
		button.pressed = false
		button.pointer = nil
	end
	for _, pointer in ipairs(self._pointers) do
		pointer.control = nil
	end
end

function VirtualJoypad:update()
	self._stick_x, self._stick_y = 0, 0
	for _, axis in pairs(self._axes) do
		axis.x, axis.y, axis.z = 0, 0, 0
	end
	for _, button in pairs(self._buttons) do
		button.pressed = false
		button.released = false
	end
	local width, height = Device.resolution()
	if width ~= self._width or height ~= self._height or not Window.has_focus() then
		self._width, self._height = width, height
		self:reset()
		return false
	end

	local left = self._axes[AXIS_IDS.left]
	local right = self._axes[AXIS_IDS.right]
	local trigger_left = self._axes[AXIS_IDS.trigger_left]
	local scale = math.min(width, height) / 600
	local radius = STICK_RADIUS * scale
	local center = STICK_CENTER * scale
	local button_radius = 40 * scale
	local visible = self.visible

	for _, pointer in ipairs(self._pointers) do
		local control = pointer.control
		if control and (Touch.button(pointer.button) == 0 or Touch.pressed(pointer.button)) then
			control.pointer = nil
			if control.down ~= nil then
				control.released = control.down
				control.down = false
			end
			pointer.control = nil
		end
	end

	for _, pointer in ipairs(self._pointers) do
		local pressed = Touch.pressed(pointer.button)
		local down = Touch.button(pointer.button) ~= 0
		if pressed or down then
			local position = Touch.axis(pointer.axis)
			local x, y = position.x, height - position.y
			if pressed then
				self.visible = true
				local control
				if left and (x - center)^2 + (y - center)^2 <= radius^2 then
					control = left
				else
					for id = 0, 1 do
						local offset = BUTTON_POSITIONS[id + 1]
						local bx, by = width - offset[1] * scale, offset[2] * scale
						if self._buttons[id] and (x - bx)^2 + (y - by)^2 <= button_radius^2 then
							control = self._buttons[id]
							break
						end
					end
					if not control and x >= width / 2 then
						control = right
					end
				end
				if control and not control.pointer then
					pointer.control = down and control or nil
					control.pointer = down and pointer or nil
					pointer.origin_x, pointer.origin_y = x, y
					if control.down ~= nil then
						control.down = down
						control.pressed = true
						control.released = control.released or not down
					end
				end
			end

			local axis = pointer.control
			if axis and (axis == left or axis == right) then
				local cx, cy = center, center
				if axis == right then
					cx, cy = pointer.origin_x, pointer.origin_y
				end
				local dx, dy = (x - cx) / radius, (y - cy) / radius
				local length = math.sqrt(dx*dx + dy*dy)
				if axis == left then
					if trigger_left then
						trigger_left.z = length >= OUTER_RADIUS / STICK_RADIUS and 1 or 0
					end
					if length > 0 then
						local stick_length = math.min(length, OUTER_RADIUS / STICK_RADIUS)
						self._stick_x, self._stick_y = dx / length * stick_length, dy / length * stick_length
					end
				end
				if length > 0.15 then
					local magnitude = (math.min(length, 1) - 0.15) / 0.85
					axis.x, axis.y = dx / length * magnitude, dy / length * magnitude
				end
			end
		end
	end
	return self.visible and not visible
end

function VirtualJoypad:axis_id(name)
	local id = AXIS_IDS[name]
	return self._axes[id] and id
end

function VirtualJoypad:axis(id)
	local axis = self._axes[id]
	return Vector3(axis.x, axis.y, axis.z)
end

function VirtualJoypad:button_id(name)
	local id = BUTTON_IDS[name]
	return self._buttons[id] and id
end

function VirtualJoypad:button(id)
	return self._buttons[id].down and 1 or 0
end

function VirtualJoypad:pressed(id)
	return self._buttons[id].pressed
end

function VirtualJoypad:released(id)
	return self._buttons[id].released
end

local function circle(gui, x, y, radius, color, depth, thickness)
	local inner = thickness and radius - thickness or 0
	for i = 0, 23 do
		local a = i * math.pi / 12
		local b = (i + 1) * math.pi / 12
		local ax, ay = math.cos(a), math.sin(a)
		local bx, by = math.cos(b), math.sin(b)
		local p = Vector2(x + ax * inner, y + ay * inner)
		Gui.triangle(gui, p, Vector2(x + ax * radius, y + ay * radius)
			, Vector2(x + bx * radius, y + by * radius), color, depth)
		if thickness then
			Gui.triangle(gui, p, Vector2(x + bx * radius, y + by * radius)
				, Vector2(x + bx * inner, y + by * inner), color, depth)
		end
	end
end

function VirtualJoypad:draw(gui)
	if not self.visible then
		return
	end
	local scale = math.min(self._width, self._height) / 600
	local center = STICK_CENTER * scale
	local radius = STICK_RADIUS * scale
	if self._axes[AXIS_IDS.left] then
		local trigger_left = self._axes[AXIS_IDS.trigger_left]
		if trigger_left then
			local color = trigger_left.z ~= 0 and Color4(255, 255, 255, 150) or Color4(255, 255, 255, 70)
			circle(gui, center, center, OUTER_RADIUS * scale, color, -1, 2 * scale)
		end
		circle(gui, center, center, radius, Color4(255, 255, 255, 55), -1)
		circle(gui, center + self._stick_x * radius, center + self._stick_y * radius
			, 24 * scale, Color4(255, 255, 255, 120), -2)
	end
	local right = self._axes[AXIS_IDS.right]
	if right and right.pointer then
		local x, y = right.pointer.origin_x, right.pointer.origin_y
		circle(gui, x, y, radius, Color4(255, 255, 255, 55), -1)
		circle(gui, x + right.x * radius, y + right.y * radius
			, 24 * scale, Color4(255, 255, 255, 120), -2)
	end
	local font = "core/game/hud/debug"
	local size = math.max(1, math.floor(32 * scale))
	for id = 0, 1 do
		local button = self._buttons[id]
		if button then
			local offset = BUTTON_POSITIONS[id + 1]
			local bx, by = self._width - offset[1] * scale, offset[2] * scale
			local color = button.down and Color4(255, 255, 255, 160) or Color4(255, 255, 255, 70)
			circle(gui, bx, by, 40 * scale, color, -1)
			local label = string.upper(button.name)
			local extents = Gui.text_extents(gui, size, label, font)
			Gui.text(gui, Vector3(bx - extents.x / 2, by - size / 2, -2)
				, size, label, font, font, Color4.white())
		end
	end
end

return VirtualJoypad

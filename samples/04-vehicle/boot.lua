-- Copyright (c) 2012-2026 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT

require "core/game/boot"

local step = 1/120
local accumulator = 0

function update(dt)
	if GameBase.virtual_joypad and GameBase.virtual_joypad:update() then
		GameBase.show_help = false
	end

	accumulator = accumulator + math.min(dt, 0.1)
	while accumulator >= step do
		local nv, nq, nm = Device.temp_count()
		World.update(GameBase.world, step)
		Device.set_temp_count(nv, nq, nm)
		accumulator = accumulator - step
	end
	Game.update(dt)
end

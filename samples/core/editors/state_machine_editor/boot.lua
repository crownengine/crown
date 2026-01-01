-- Copyright (c) 2012-2026 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT

function init()
	Device.enable_resource_autoload(true)
	require "core/editors/state_machine_editor/state_machine_editor"
	LevelEditor:init()
end

function update(dt)
	LevelEditor:update(dt)
end

function render(dt)
	LevelEditor:render(dt)
end

function shutdown()
	LevelEditor:shutdown()
end

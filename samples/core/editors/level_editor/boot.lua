require "core/editors/level_editor/level_editor"

function init()
	Device.enable_resource_autoload(true)
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

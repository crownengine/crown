require "core/editors/unit_preview/unit_preview"

function init()
	Device.enable_resource_autoload(true)
	UnitPreview:init()
end

function update(dt)
	UnitPreview:update(dt)
end

function render(dt)
	UnitPreview:render(dt)
end

function shutdown()
	UnitPreview:shutdown()
end

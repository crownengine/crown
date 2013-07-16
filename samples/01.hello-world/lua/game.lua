function init()
	-- Set the title of the main window
	Window.set_title("Hello world!")
end

function frame(dt)
	-- Stop the engine when any key is released
	if Keyboard.key_released(Keyboard.KC_ESCAPE) then
		Device.stop()
	end
end

function shutdown()
end
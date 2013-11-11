function init()
	-- Set the title of the main window
	Window.set_title("Hello world!")
end

function frame(dt)
	-- Stop the engine when the 'ESC' key is released
	if Keyboard.button_released(Keyboard.ESCAPE) then
		Device.stop()
	end
end

function shutdown()
end
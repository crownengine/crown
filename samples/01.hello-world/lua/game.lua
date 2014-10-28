function init()
	-- Set the title of the main window
	Window.set_title("Hello world!")
end

function update(dt)
	-- Stop the engine when the 'ESC' key is released
	if Keyboard.button_released(Keyboard.ESCAPE) then
		Device.quit()
	end
end

function render(dt)
	-- Render nothing
end

function shutdown()
end
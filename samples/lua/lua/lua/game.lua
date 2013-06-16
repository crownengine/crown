function init()
	print("Hello from lua!!!")
end

function shutdown()
end

function frame(dt)
	if Keyboard.key_pressed(65) then
		Device.stop()
	end
end

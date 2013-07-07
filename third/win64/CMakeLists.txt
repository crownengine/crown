cmake_minimum_required(VERSION 2.8)

#install (FILES 	luajit/include/luajit-2.0/lua.hpp
#				luajit/include/luajit-2.0/lauxlib.h
#				luajit/include/luajit-2.0/lua.h
#				luajit/include/luajit-2.0/luaconf.h
#				luajit/include/luajit-2.0/luajit.h
#				luajit/include/luajit-2.0/lualib.h DESTINATION include/crown/luajit)
				
if (WINDOWS)
	install (FILES luajit/lib/lua51.dll DESTINATION bin)
	# HACK !!!
	# Don't know why, but crown-win.exe need this...
	install (FILES luajit/lib/luajit-5.1.dll DESTINATION bin)	
endif (WINDOWS)

install (PROGRAMS luajit/bin/luajit.exe DESTINATION tools)
install (DIRECTORY luajit/share/luajit-2.0.1/jit DESTINATION tools)

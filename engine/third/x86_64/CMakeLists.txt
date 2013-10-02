cmake_minimum_required(VERSION 2.8)

#install (FILES 	luajit/include/luajit-2.0/lua.hpp
#				luajit/include/luajit-2.0/lauxlib.h
#				luajit/include/luajit-2.0/lua.h
#				luajit/include/luajit-2.0/luaconf.h
#				luajit/include/luajit-2.0/luajit.h
#				luajit/include/luajit-2.0/lualib.h DESTINATION include/crown/luajit)
				
#install (DIRECTORY luajit/lib/pkgconfig DESTINATION lib/crown)
install (FILES luajit/lib/libluajit-5.1.so.2.0.1 
			   luajit/lib/libluajit-5.1.so.2 DESTINATION bin)

install (PROGRAMS luajit/bin/luajit-2.0.1 DESTINATION bin)
install (FILES luajit/bin/luajit DESTINATION bin)
install (DIRECTORY luajit/share/luajit-2.0.1/jit DESTINATION bin)

install (FILES luajit/lib/libluajit-5.1.so.2.0.1 
			   luajit/lib/libluajit-5.1.so.2 DESTINATION test)

install (PROGRAMS luajit/bin/luajit-2.0.1 DESTINATION test)
install (FILES luajit/bin/luajit DESTINATION test)
install (DIRECTORY luajit/share/luajit-2.0.1/jit DESTINATION test)
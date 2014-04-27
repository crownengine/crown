cmake_minimum_required(VERSION 2.8)

install (FILES luajit/lib/libluajit-5.1.so.2.0.1 
			   luajit/lib/libluajit-5.1.so.2 DESTINATION bin)

install (PROGRAMS luajit/bin/luajit-2.0.1 DESTINATION tools)
install (FILES luajit/bin/luajit DESTINATION tools)
install (DIRECTORY luajit/share/luajit-2.0.1/jit DESTINATION tools)

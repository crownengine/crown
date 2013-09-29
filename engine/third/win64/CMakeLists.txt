cmake_minimum_required(VERSION 2.8)
				
install (FILES luajit/lib/lua51.dll DESTINATION bin)
install (FILES luajit/lib/luajit-5.1.dll DESTINATION bin)
install (FILES oggvorbis/lib/libogg.dll DESTINATION bin)
install (FILES oggvorbis/lib/libvorbis.dll DESTINATION bin)
install (FILES oggvorbis/lib/libvorbisfile.dll DESTINATION bin)

install (FILES luajit/lib/lua51.dll DESTINATION test)
install (FILES luajit/lib/luajit-5.1.dll DESTINATION test)
install (FILES oggvorbis/lib/libogg.dll DESTINATION bin)
install (FILES oggvorbis/lib/libvorbis.dll DESTINATION bin)
install (FILES oggvorbis/lib/libvorbisfile.dll DESTINATION bin)	

install (PROGRAMS luajit/bin/luajit.exe DESTINATION bin)
install (DIRECTORY luajit/share/luajit-2.0.1/jit DESTINATION bin)

install (PROGRAMS luajit/bin/luajit.exe DESTINATION test)
install (DIRECTORY luajit/share/luajit-2.0.1/jit DESTINATION test)
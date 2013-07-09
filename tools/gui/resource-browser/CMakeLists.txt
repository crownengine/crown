cmake_minimum_required(VERSION 2.8)

install (FILES resource-browser.py PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
	GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE DESTINATION tools)

install (FILES ui/resource-browser.glade DESTINATION tools/ui)

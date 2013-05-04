local src = arg[1]
local tmp = src .. ".tmp"


local chunk = string.dump(loadfile(src), true)

local f,err = io.open(tmp, "w")

if not f then 
	return print(err) 
end

f:write(chunk)

f:close()








local root_path = arg[2]
local dest_path = arg[4]

-------------------------------------------------------------
function get_files(path)

	local files = {}
	local tmp_file = 'tmp.txt'

	os.execute('ls -1 ' .. path .. '*.lua > ' .. tmp_file)

	local f = io.open(tmp_file)
	if not f then
		return
	end

	local i = 1;

	for line in f:lines() do
		files[i] = line
		i = i + 1
	end

	f.close()

	return files

end

--------------------------------------------------------------
function prepare_destination(files)
	-- Portable bytecode
	local ext = ".script"
	local raws =  {}

	for i,s in ipairs(files) do
		raws[i] = string.sub(s, 1 + string.len(root_path))
		local len = string.len(raws[i])
		raws[i] = string.sub(raws[i], 1, len -4)
		raws[i] = raws[i] .. ext
		raws[i] = dest_path .. raws[i]
	end

	return raws
end



--------------------------------------------------------------
function generate_bytecode(src, dest)

	local chunk = string.dump(loadfile(src), true)

	local f,err = io.open(dest, "w")

	if not f then 
		return print(err) 
	end

	f:write(chunk)

	f:close()

end

--------------------------------------------------------------

local src  = get_files(root_path)

local dest = prepare_destination(src)

for i,s in ipairs(src) do
	generate_bytecode(s, dest[i])
end




function fs_enumerate(directory)
  local i, t, popen = 0, {}, io.popen
  local pfile = popen('ls -p "'..directory..'"')
  for filename in pfile:lines() do
      i = i + 1
      t[i] = filename
  end
  pfile:close()
  return t
end

function output_source_files(dir, extension)
  local files = fs_enumerate(dir)
  for _, file in ipairs(files) do
    if file:sub(-1) == '/' then
      output_source_files(dir .. file, extension)
    elseif file:sub(-#extension) == extension then
      io.write('\'' .. dir .. file .. '\',\n')
    end 
  end
end

local OUTPUT_PATH = "meson.build"
io.output(OUTPUT_PATH)
io.write('project(\'level_editor\', [\'vala\', \'c\'])\n')
io.write('deps = [\n')
io.write('  dependency(\'gdk-3.0\'),\n')
io.write('	dependency(\'gee-0.8\'),\n')
io.write('	dependency(\'gio-2.0\'),\n')
io.write('	dependency(\'glib-2.0\'),\n')
io.write('	dependency(\'gtk+-3.0\')\n')
io.write(']\n')
io.write('src = files([\n')

output_source_files('./tools/', '.vala')

io.write('])\n')
io.write('\n')
io.write('gnome = import(\'gnome\')\n')
io.write('\n')
io.write('resources_xml = files(\'tools\\\\level_editor\\\\resources\\\\resources.gresource.xml\')\n')
io.write('resources = gnome.compile_resources(\'resources\', resources_xml, source_dir: \'.\\\\tools\\\\level_editor\\\\resources\\\\\')\n')
io.write('\n')
io.write('vala_args = [ \n')
io.write('  \'--target-glib=2.38\', # minimum glib version required for composite templates\n')
io.write('  \'--define=CROWN_PLATFORM_WINDOWS\',\n')
io.write('  \'--gresources\', resources_xml,\n')
io.write(']\n')
io.write('\n')
io.write('executable(\'app\',\n')
io.write('           [src, resources],\n')
io.write('           dependencies: deps,\n')
io.write('           vala_args: [vala_args])\n')
io.close()
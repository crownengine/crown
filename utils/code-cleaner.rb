# Copyright (c) 2013 Daniele Bartolini, Michele Rossi
# Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto
# 
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following
# conditions:
# 
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
# OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

require 'optparse'
require 'ostruct'

$extension = [".cpp", ".h"]

$files_list = Array.new

#------------------------------------------------------------------------------
def validate_command_line(args)

	if args.length != 2
		return false
	end

	if args[0] != "--path"
		return false
	end

	return true
end

#------------------------------------------------------------------------------
def parse_command_line(args)

	banner = "Usage: crown-android.rb --target <android-target> --name <project-name> --path <project-path>\n"

	if not validate_command_line(args)
		print banner
		exit
	end

	options = OpenStruct.new

	OptionParser.new do |opts|
		opts.banner = banner

		opts.on("-p", "--path PATH", "Code Path to clean") do |p|
			options.path = p
		end

	    opts.on_tail("-h", "--help", "Show this message") do
	    	puts opts
	      	exit
	    end
	end.parse!(args)

	return options

end
	
#------------------------------------------------------------------------------
def lists_files(path)

	files = Dir.entries(path)

	for entry in files
		if entry == "." || entry == ".."
			next
		end

		tmp = path + '/' + entry

		if File.directory?(tmp)
			lists_files(tmp)
		elsif File.file?(tmp)
			ext = File.extname(tmp)
			if ext == $extension[0] || ext == $extension[1] || ext == $extension[2]
				$files_list.push(tmp);
			end
		end
	end
	
end

#------------------------------------------------------------------------------
def clean_file(file_name) 
	lines = []
	# Copies each file's line
	File.open(file_name) do |file|
		file.each_line do |line|
			lines << line
		end
	end

	clean = true
	lines.each_index do |index|
		line = lines[index]
		#match one or more spaces/tabs at the end of the line -> nothing
		new_line = line.gsub(/[ \t]+$/, "")
		if(new_line != line)
			if(clean)
	    		puts "Cleaning '#{file_name}'"
	    		clean = false
	  		end

	  		puts "#{index+1}"

	  		lines[index] = new_line
		end
	end

	if(!clean)
		File.open(file_name, 'w') do |file|
	  		lines.each { |line| file.write(line) }
		end
	end
end

opts = parse_command_line(ARGV)

lists_files(opts.path)

$files_list.each { |file| clean_file(file) }
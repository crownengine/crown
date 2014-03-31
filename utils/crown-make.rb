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
require 'fileutils'

# README
# LINUX ONLY RIGHT NOW

#------------------------------------------------------------------------------
def validate_command_line(args)

	# if args.length < 8
	# 	return false 
	# end
	# if args[0] != "--build"
	# 	return false
	# end
	# if args[2] != "--path"
	# 	return false
	# end

	# return true
end

#------------------------------------------------------------------------------
def parse_command_line(args)

	banner = "Usage: crown-make.rb --build BUILD --path PATH\n"

	# if not validate_command_line(args)
	# 	print banner
	# 	exit
	# end

	options = OpenStruct.new

	OptionParser.new do |opts|
		opts.banner = banner

		opts.on("-m", "--mode MODE", "Crown build mode(debug, development, release, all)") do |m|
			options.mode = m
		end

		opts.on("-e", "--engine-path ENGINE_PATH", "Engine path") do |e|
			options.engine_path = e
		end

		opts.on("-b", "--build-path BUILD_PATH", "Build path") do |b|
			options.build_path = b
		end

		opts.on("-i", "--install-path PATH", "Install path") do |i|
			options.install_path = i
		end

	    opts.on_tail("-h", "--help", "Show this message") do
	    	puts opts
	      	exit
	    end
	end.parse!(args)

	return options
end

#------------------------------------------------------------------------------
def build_linux_debug(engine_path, build_path, install_path)
	Dir.chdir(build_path)
	FileUtils.mkdir_p("debug")

	Dir.chdir(build_path + "/debug")
	if not system("cmake " + engine_path + " -DCMAKE_INSTALL_PREFIX=" + install_path + "/debug" + " -DCROWN_BUILD=linux-debug-64")
		print ("Unable to run cmake")
		return
	end
	if not system("make install -j 8")
		print ("Unable to compile")
		return
	end
end

#------------------------------------------------------------------------------
def build_linux_development(engine_path, build_path, install_path)
	Dir.chdir(build_path)
	FileUtils.mkdir_p("development")

	Dir.chdir(build_path + "/development")
	if not system("cmake " + engine_path + " -DCMAKE_INSTALL_PREFIX=" + install_path + "/development" + " -DCROWN_BUILD=linux-development-64")
		print ("Unable to run cmake")
		return
	end
	if not system("make install -j 8")
		print ("Unable to compile")
		return
	end
end

#------------------------------------------------------------------------------
def build_linux_release(engine_path, build_path, install_path)
	Dir.chdir(build_path)
	FileUtils.mkdir_p("release")

	Dir.chdir(build_path + "/release")
	if not system("cmake " + engine_path + " -DCMAKE_INSTALL_PREFIX=" + install_path + "/release" + " -DCROWN_BUILD=linux-release-64")
		print ("Unable to run cmake")
		return
	end
	if not system("make install -j 8")
		print ("Unable to compile")
		return
	end
end

#------------------------------------------------------------------------------
opts = parse_command_line(ARGV)

case opts.mode
when "debug"
	build_linux_debug(opts.engine_path, opts.build_path, opts.install_path)
when "development"
	build_linux_development(opts.engine_path, opts.build_path, opts.install_path)
when "release"
	build_linux_release(opts.engine_path, opts.build_path, opts.install_path)
when "all"
	build_linux_debug(opts.engine_path, opts.build_path, opts.install_path)
	build_linux_development(opts.engine_path, opts.build_path, opts.install_path)
	build_linux_release(opts.engine_path, opts.build_path, opts.install_path)
end
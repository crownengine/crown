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

$android_create 	= "android create project"
$android_update 	= "android update project"
$activity			= "CrownActivity"
$package			= "crown.android"

$engine_src 		= "../engine/."
$android_src		= "../engine/os/android/java/."
$config_src			= "../engine/os/android/Config.h"
$manifest			= "../engine/os/android/AndroidManifest.xml"

$luajit				= "../engine/third/ARMv7/luajit"
$oggvorbis			= "../engine/third/ARMv7/oggvorbis"

#------------------------------------------------------------------------------
def validate_command_line(args)

	if args.length != 8
		return false
	end

	if args[0] != "--target"
		return false
	end

	if args[2] != "--name"
		return false
	end

	if args[4] != "--res"
		return false
	end

	if args[6] != "--path"
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

		opts.on("-t", "--target TARGET", "Android target") do |t|
			options.target = t
		end

		opts.on("-n", "--name NAME", "Android project name") do |n|
			options.name = n
		end

		opts.on("-r", "--res RES", "Android project compiled resources") do |r|
			options.res = r
		end

		opts.on("-p", "--path PATH", "Android project path") do |p|
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
def create_android_project(target, name, path)

	engine_dest 	= path + "/jni"
	android_dest	= path + "/src/crown/android"
	assets_dest		= path + "/assets"

	# Creates path if not exists
	if not Dir.exists?(path)
		FileUtils.mkdir_p(path)
	end

	# Project path is empty
	if Dir[path + "/*"].empty?
		# Creates android project
		system($android_create + " --target " + target + " --name " + name + " --path " + path + 
				" --activity " + $activity + " --package " + $package)
	else
		# Updates android project
		system($android_update + " --target " + target + " --name " + name + " --path " + path)
	end
	
	# if jni dir does not exists, create it!
	if not Dir.exists?(engine_dest)
		FileUtils.mkdir_p(engine_dest)
		print "Created directory " + engine_dest + "\n"
	end

	# if assets dir does not exists, create it!
	if not Dir.exists?(assets_dest)
		FileUtils.mkdir_p(assets_dest)
		print "Created directory " + assets_dest + "\n"
	end
end

#------------------------------------------------------------------------------
def fill_android_project(res, path)

	engine_dest 	= path + "/jni"
	android_dest	= path + "/src/crown/android"
	resources_dest	= path + "/assets"

	# Copy Engine files
	FileUtils.cp_r($engine_src, engine_dest, :remove_destination => true)
	print "Copied Engine to " + engine_dest + "\n"

	# Copy android Config.h
	FileUtils.cp($config_src, engine_dest)
	print "Copied Config.h to " + engine_dest + "\n"

	# Copy luajit lib
	FileUtils.cp($luajit + "/lib/libluajit-5.1.so.2.0.2", engine_dest + "/libluajit-5.1.so")
	print "Copied luajit lib to " + engine_dest + "\n"

	# Copy oggvorbis lib
	FileUtils.cp($oggvorbis + "/lib/libogg.a", engine_dest + "/libogg.a")
	FileUtils.cp($oggvorbis + "/lib/libvorbis.a", engine_dest + "/libvorbis.a")
	print "Copied oggvorbis libs to " + engine_dest + "\n"

	# Copy Java files
	FileUtils.cp_r(Dir.glob($android_src), android_dest, :remove_destination => true)
	print "Copied Java files to " + android_dest + "\n"

	# Copy Android Manifest
	FileUtils.cp($manifest, path)
	print "Copied Android Manifest to " + path  + "\n"

	#Copy resources
	FileUtils.cp_r(res + "/.", resources_dest, :remove_destination => true)
	print "Resources copied to " + resources_dest  + "\n"

end

#------------------------------------------------------------------------------
def build_android_project(path)
	# Move to root directory of Android project
	Dir.chdir(path)
	# Build libraries
	if not system("ndk-build")
		print "Critical error: Unable to build crown libraries"
		return
	end
	# Build apk
	if not system("ant debug")
		print "Critical error: Unable to build crown project"
		return
	end
end

#------------------------------------------------------------------------------
opts = parse_command_line(ARGV)

create_android_project(opts.target, opts.name, opts.path)
fill_android_project(opts.res, opts.path)
build_android_project(opts.path)
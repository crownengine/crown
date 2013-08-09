#!/usr/bin/ruby

require 'optparse'
require 'ostruct'
require 'fileutils'

$android_create 	= "android create project"
$android_update 	= "android update project"
$activity			= "CrownActivity"
$package			= "crown.android"

$engine_src 		= "../engine/."
$android_src		= "../engine/os/android/*.java"
$manifest			= "../engine/os/android/AndroidManifest.xml"


def parse_command_line(args)
	banner = "Usage: crown-android.rb --target <android-target> --name <project-name> --path <project-path>\n"
	if args.length != 6
		print banner
		exit
	end

	options = OpenStruct.new

    options.library = []
    options.inplace = false
    options.encoding = "utf8"
    options.transfer_type = :auto
    options.verbose = false

	OptionParser.new do |opts|
		opts.banner = banner

		opts.on("-t", "--target TARGET", "Android target") do |t|
			options.target = t
		end

		opts.on("-n", "--name NAME", "Android project name") do |n|
			options.name = n
		end

		opts.on("-p", "--path PATH", "Android project path") do |p|
			options.path 	= p
			$engine_dest 	= p + "/jni"
			$android_dest	= p + "/src/crown/android"
		end
	    opts.on_tail("-h", "--help", "Show this message") do
	    	puts opts
	      	exit
	    end
	end.parse!(args)

	return options
end

def create_android_project(target, name, path)

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

	engine_dest 	= path + "/jni"
	android_dest	= path + "/src/crown/android"
	
	# if jni dir does not exists, create it!
	if not Dir.exists?(engine_dest)
		FileUtils.mkdir_p(engine_dest)
		print "Created directory " + path + "/jni\n"
	end

	# Copy Engine files
	FileUtils.cp_r($engine_src, engine_dest, :remove_destination => true)
	print "Copied engine to " + engine_dest + "/jni\n"

	# Copy Java files
	FileUtils.cp_r(Dir.glob($android_src), android_dest, :remove_destination => true)
	print "Copied Java files to " + android_dest + "\n"

	# Copy Android Manifest
	FileUtils.cp($manifest, path)
	print "Copied Android Manifest to " + path  + "\n"
end



opts = parse_command_line(ARGV)

create_android_project(opts.target, opts.name, opts.path)
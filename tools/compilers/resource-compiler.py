#!/usr/bin/python

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

import sys
import os

from crown.resources import Repository
from crown.resources import Compiler

DEST_PATH_SUFFIX = "_compiled"
PERFECT_SEED_FILE = "seed.ini"

# Helper for compiling resources
class CompilerHelper:
	def __init__(self, root_path):
		self.m_root_path = root_path

		# Define destination path name
		dest = os.path.dirname(root_path) + "/" + os.path.basename(root_path) + DEST_PATH_SUFFIX
		self.m_dest_path = os.path.normpath(dest)

		# Repository needs a root path
		self.m_repository = Repository.Repository(root_path)

		self.m_compiler = Compiler.Compiler(self.m_repository, self.m_dest_path)

	# Returns the root path where the resources are read from
	def root_path(self):
		return self.m_root_path

	# Returns the destination folder where the compiled resources
	# will be put
	def dest_path(self):
		return self.m_dest_path

	# Creates the destination folder
	def create_dest_folder(self):
		if not os.path.exists(self.m_dest_path):
			os.makedirs(self.m_dest_path)

	# Write the perfect seed into the destination folder
	def write_perfect_seed(self):
		seed_file_name = self.dest_path() + "/" + PERFECT_SEED_FILE
		seed_file_name = os.path.normpath(seed_file_name)

		file = open(seed_file_name, "w")
		file.write(str(self.m_compiler.perfect_seed()))
		file.close()

 	# Compiles the resources into the destination folder
	def compile(self):
		# Scan the repository
		print("Scanning folder:", self.m_repository.root_path())
		self.m_repository.scan()
		print("Resources:", len(self.m_repository.all_resources()))

		# Create the output folder
		print("Creating destination folder:", self.dest_path())
		self.create_dest_folder()

		print("Compiling resources...")
		self.m_compiler.compile_all()

		print("Writing perfect seed...")
		self.write_perfect_seed()

		print("Done.")

#------------------------------------------------------------------------------
def print_help_message():
	print("usage: resource-compiler <root-path>")

#------------------------------------------------------------------------------
def main():
	root_path = ""

	if (len(sys.argv) != 2):
		print("Usage: resource-compiler <root-path>")
		sys.exit(-1)

	root_path = sys.argv[1];

	root_path = os.path.abspath(root_path)

	if not os.path.exists(root_path):
		print("The path does not exist.")
		sys.exit(-1)

	if (os.path.islink(root_path)):
		print("The path is a symbolic link.")
		sys.exit(-1)

	if not os.path.isdir(root_path):
		print("The path has to be a directory.")
		sys.exit(-1)

	helper = CompilerHelper(root_path)

	helper.compile()

main()


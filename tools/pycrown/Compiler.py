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

import subprocess
import os

LUAJIT = "luajit"
OPTION = "-b"
COMPILER_NAME = "resource-compiler"
RES_H = "resource-hash"

ROOT_P = "--root-path"
DEST_P = "--dest-path"
RES_IN = "--resource-in"
SEED = "--seed"

# Compiler is an helper for compiling resources in a repository
# either one by one or in a whole
class Compiler:
	def __init__(self, repository, dest_path):
		self.m_repository = repository
		self.m_perfect_seed = -1
		self.m_dest_path = dest_path

	# Returns the destination path where the compiled
	# resources will be put
	def dest_path(self):
		return self.m_dest_path

	# Returns the perfect seed
	def perfect_seed(self):
		return self.m_perfect_seed

	# Computes a perfect seed for the resources contained
	# in the repository
	def compute_perfect_seed(self):
		# Obtain resources from repository
		resources = self.m_repository.all_resources()

		resource_hashes = []

		seed = 0

		# Calculate the hash resource by resource
		# Note: we can speed-up this process by modifying resource-hash to
		# calculate more than a hash at time...
		while True:
			for res in resources:
				p = subprocess.check_output([RES_H, RES_IN, res, SEED, str(seed)])
				resource_hashes.append(str(p))

			if len(resource_hashes) == len(set(resource_hashes)):
				self.m_perfect_seed = seed
				return

			resource_hashes.clear()
			seed = seed + 1;

	# Compiles all the texture resources in the repository
	def compile_textures(self):
		self.compile(self.m_repository.texture_resources());

	# Compiles all the mesh resources in the repository
	def compile_meshes(self):
		self.compile(self.m_repository.mesh_resources());

	# Compiles all the text resources in the repository
	def compile_texts(self):
		self.compile(self.m_repository.text_resources());

	# Compiles all the script resources in the repository
	def compile_scripts(self):
		lua_resources = self.m_repository.script_resources()
		root_path = self.m_repository.root_path()

		for res in lua_resources:
			path_in = os.path.normpath(root_path + "/" + res)
			path_out = os.path.normpath(os.path.dirname(self.m_dest_path + "/" + res))

			if not os.path.exists(path_out):
				os.makedirs(path_out)

			head, out_filename = os.path.split(res)

			f = subprocess.call([LUAJIT, OPTION, path_in, path_out + "/" + out_filename + ".script"]);
	
	# Compiles all the vertex shader resources in the repository
	def compile_vertex_shaders(self):
		self.compile(self.m_repository.vertex_shader_resources());

	# Compiles all the vertex shader resources in the repository
	def compile_pixel_shaders(self):
		self.compile(self.m_repository.pixel_shader_resources());

	# Compiles all the resources in the repository
	def compile_all(self):
		print("Compiling textures...")
		self.compile_textures()

		print("Compiling meshes...")
		self.compile_meshes()

		print("Compiling texts...")
		self.compile_texts()

		print("Compiling scripts...")
		self.compile_scripts()

		print("Compiling vertex shaders...")
		self.compile_vertex_shaders()

		print("Compiling pixel shaders...")
		self.compile_pixel_shaders()

	# Compile a single resource from the repository
	def compile(self, resources):
		if (len(resources) == 0):
			return

		# Compute perfect seed if necessary
		if (self.m_perfect_seed == -1):
			self.compute_perfect_seed()

		root_path = self.m_repository.root_path()
		resource_params = (' '.join(resources)).split();
		compiler_params = [COMPILER_NAME, "--root-path", root_path, "--dest-path", self.m_dest_path, "--seed", str(self.m_perfect_seed)]

		p = subprocess.call(compiler_params + resource_params)

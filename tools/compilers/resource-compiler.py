#!/bin/python

import os
import subprocess

resource_extensions = ('.txt', '.tga', '.dae', '.lua')
resources = []
resource_hashes = []
root_path = "/home/dani/test/resources"

#------------------------------------------------------------------------------
def read_resources(root_path):
	for dirname, dirnames, filenames in os.walk(root_path):

		# print path to all filenames.
		for filename in filenames:
	
			# get the resource name
			abs_path = os.path.join(dirname, filename)
			resource = os.path.relpath(abs_path, root_path)

			# filter resource names by type
			if resource.endswith(resource_extensions):
				resources.append(resource)

#------------------------------------------------------------------------------
def perfect_seed(resources):
	seed = 0

	while True:
		for res in resources:
			p = subprocess.check_output(["resource-hash", "--resource-in", res, "--seed", str(seed)])
			resource_hashes.append(str(p))

		if len(resource_hashes) == len(set(resource_hashes)):
			return seed
		
		resource_hashes.clear()
		seed = seed + 1;

#------------------------------------------------------------------------------
def compile(dest_path, resource, perfect_seed):
	if resource.endswith('.txt'):
		p = subprocess.call(["txt-compiler", "--root-path", root_path, "--dest-path", dest_path, "--resource-in", resource, "--seed", str(perfect_seed)]);
	if resource.endswith('.tga'):
		p = subprocess.call(["tga-compiler", "--root-path", root_path, "--dest-path", dest_path, "--resource-in", resource, "--seed", str(perfect_seed)]);	

#------------------------------------------------------------------------------
def compile_all(dest_path, resources, perfect_seed):
	for res in resources:
		compile(dest_path, res, perfect_seed)
		
#------------------------------------------------------------------------------
def create_output_folder(root_path):
	output_path = os.path.dirname(root_path) + "/" + os.path.basename(root_path) + "_compiled"
	output_path = os.path.normpath(output_path)

	if not os.path.exists(output_path):
		os.makedirs(output_path)
		
	return output_path

#------------------------------------------------------------------------------
def write_perfect_seed(dest_path, perfect_seed):
	output_path = dest_path + "/" + "seed.ini"
	output_path = os.path.normpath(output_path)

	file = open(output_path, "w");
	file.write(str(perfect_seed))
	file.close()

#------------------------------------------------------------------------------
print("Reading resources...\n")
read_resources(root_path)

perfect_seed = perfect_seed(resources)

print("Perfect seed is: ", perfect_seed)

print("Creating output folder...\n")
dest_path = create_output_folder(root_path)

print("Writing perfect seed...\n")
write_perfect_seed(dest_path, perfect_seed)

print("Compiling resources...\n")
compile_all(dest_path, resources, perfect_seed)

print("\n")

print("Done.")



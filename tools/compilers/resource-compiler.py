#!/bin/python

import os

from crown.resources import Repository
from crown.resources import Compiler

root_path = "/home/daniele/resources"

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

repository = Repository.Repository(root_path)

#------------------------------------------------------------------------------
print("Reading resources...")
repository.scan()

print("Found", len(repository.all_resources()), "resources.")

print("Creating output folder...")
dest_path = create_output_folder(root_path)

compiler = Compiler.Compiler(repository, dest_path)
compiler.compute_perfect_seed()

print("Compiling resources...")

print("Perfect seed is: ", compiler.perfect_seed())

print("Writing resources...")
compiler.compile_all()

print("Writing perfect seed...")
write_perfect_seed(dest_path, compiler.perfect_seed())

print("\n")

print("Done.")



# ##### BEGIN GPL LICENSE BLOCK #####
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software Foundation,
#  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
# ##### END GPL LICENSE BLOCK #####

# <pep8 compliant>

import os
import time

import bpy
import mathutils
import bpy_extras.io_utils


def name_compat(name):
	if name is None:
		return 'None'
	else:
		return name.replace(' ', '_')


def mesh_triangulate(me):
	import bmesh
	bm = bmesh.new()
	bm.from_mesh(me)
	bmesh.ops.triangulate(bm, faces=bm.faces)
	bm.to_mesh(me)
	bm.free()

def write_file(filepath, objects, scene,
			   EXPORT_TRI=True,
			   EXPORT_NORMALS=True,
			   EXPORT_UV=True,
			   EXPORT_APPLY_MODIFIERS=True,
			   EXPORT_KEEP_VERT_ORDER=False,
			   EXPORT_GLOBAL_MATRIX=None,
			   ):
	"""
	Basic write function. The context and options must be already set
	This can be accessed externaly
	eg.
	write( '/home/user/project/foobar.mesh', Blender.Object.GetSelected() ) # Using default options.
	"""

	if EXPORT_GLOBAL_MATRIX is None:
		EXPORT_GLOBAL_MATRIX = mathutils.Matrix()

	def veckey3d(v):
		return round(v.x, 6), round(v.y, 6), round(v.z, 6)

	def veckey2d(v):
		return round(v[0], 6), round(v[1], 6)

	print('Crown Engine .mesh export path: %r' % filepath)

	time1 = time.time()

	file = open(filepath, "wb")
	def fw(string):
		file.write(bytes(string, 'UTF-8'))

	# Write Header
	# FIXME FIXME FIXME

	# Initialize totals, these are updated each object
	totverts = totuvco = totno = 0

	face_vert_index = 1

	globalNormals = {}

	copy_set = set()

	# Get all meshes
	for ob_main in objects:

		# ignore dupli children
		if ob_main.parent and ob_main.parent.dupli_type in {'VERTS', 'FACES'}:
			# XXX
			print(ob_main.name, 'is a dupli child - ignoring')
			continue

		obs = []
		if ob_main.dupli_type != 'NONE':
			# XXX
			print('creating dupli_list on', ob_main.name)
			ob_main.dupli_list_create(scene)

			obs = [(dob.object, dob.matrix) for dob in ob_main.dupli_list]

			# XXX debug print
			print(ob_main.name, 'has', len(obs), 'dupli children')
		else:
			obs = [(ob_main, ob_main.matrix_world)]

		for ob, ob_mat in obs:

			try:
				me = ob.to_mesh(scene, EXPORT_APPLY_MODIFIERS, 'PREVIEW', calc_tessface=False)
			except RuntimeError:
				me = None

			if me is None:
				continue

			me.transform(EXPORT_GLOBAL_MATRIX * ob_mat)

			if EXPORT_TRI:
				# _must_ do this first since it re-allocs arrays
				mesh_triangulate(me)

			if EXPORT_UV:
				faceuv = len(me.uv_textures) > 0
				if faceuv:
					uv_texture = me.uv_textures.active.data[:]
					uv_layer = me.uv_layers.active.data[:]
			else:
				faceuv = False

			me_verts = me.vertices[:]

			# Make our own list so it can be sorted to reduce context switching
			face_index_pairs = [(face, index) for index, face in enumerate(me.polygons)]
			# faces = [ f for f in me.tessfaces ]

			# Edges not supported by Crown Engine .mesh
			edges = []

			if not (len(face_index_pairs) + len(edges) + len(me.vertices)):  # Make sure there is somthing to write

				# clean up
				bpy.data.meshes.remove(me)

				continue  # dont bother with this mesh.

			if EXPORT_NORMALS and face_index_pairs:
				me.calc_normals()

			materials = me.materials[:]
			material_names = [m.name if m else None for m in materials]

			# avoid bad index errors
			if not materials:
				materials = [None]
				material_names = [name_compat(None)]

			# Sort by Material, then images
			# so we dont over context switch in the obj file.
			if EXPORT_KEEP_VERT_ORDER:
				pass
			else:
				if faceuv:
					sort_func = lambda a: (a[0].material_index,
											   hash(uv_texture[a[1]].image),
											   a[0].use_smooth)
				elif len(materials) > 1:
					sort_func = lambda a: (a[0].material_index,
											   a[0].use_smooth)
				else:
					# no materials
					sort_func = lambda a: a[0].use_smooth

				face_index_pairs.sort(key=sort_func)

				del sort_func

			# Set the default mat to no material and no image.
			contextMat = 0, 0  # Can never be this, so we will label a new material the first chance we get.
			contextSmooth = None  # Will either be true or false,  set bad to force initialization switch.

			fw("{\n")
			# Positions
			fw("    \"position\" : [")
			for v in me_verts:
				fw('\n        %.6f, %.6f, %.6f,' % v.co[:])
			file.seek(-1, 1)
			fw("]")

			# NORMAL, Smooth/Non smoothed.
			if EXPORT_NORMALS:
				fw(",\n    \"normal\" : [")
				for f, f_index in face_index_pairs:
					if f.use_smooth:
						for v_idx in f.vertices:
							v = me_verts[v_idx]
							noKey = veckey3d(v.normal)
							if noKey not in globalNormals:
								globalNormals[noKey] = totno
								totno += 1
								fw('\n        %.6f, %.6f, %.6f,' % noKey)
					else:
						# Hard, 1 normal from the face.
						noKey = veckey3d(f.normal)
						if noKey not in globalNormals:
							globalNormals[noKey] = totno
							totno += 1
							fw('\n        %.6f, %.6f, %.6f,' % noKey)
				file.seek(-1, 1)
				fw("]")

			# UV
			if faceuv:
				# in case removing some of these dont get defined.
				uv = uvkey = uv_dict = f_index = uv_index = uv_ls = uv_k = None

				uv_face_mapping = [None] * len(face_index_pairs)

				uv_dict = {}  # could use a set() here

				fw(",\n    \"texcoord\" : [")
				for f, f_index in face_index_pairs:
					uv_ls = uv_face_mapping[f_index] = []
					for uv_index, l_index in enumerate(f.loop_indices):
						uv = uv_layer[l_index].uv

						uvkey = veckey2d(uv)
						try:
							uv_k = uv_dict[uvkey]
						except:
							uv_k = uv_dict[uvkey] = len(uv_dict)
							fw('\n        %.6f, %.6f,' % uv[:])
						uv_ls.append(uv_k)
				file.seek(-1, 1)
				fw("]")

				uv_unique_count = len(uv_dict)

				del uv, uvkey, uv_dict, f_index, uv_index, uv_ls, uv_k
				# Only need uv_unique_count and uv_face_mapping

			# Index array
			fw(',\n    \"index\" : [')

			# Position indices
			fw('\n        [ ')
			for f, f_index in face_index_pairs:
				f_smooth = f.use_smooth
				f_v = [(vi, me_verts[v_idx]) for vi, v_idx in enumerate(f.vertices)]

				for vi, v in f_v:
					fw('%d,' % (v.index + totverts))
			file.seek(-1, 1)
			fw(' ]')

			# Normal indices
			if EXPORT_NORMALS:
				fw(',\n        [ ')
				for f, f_index in face_index_pairs:
					f_smooth = f.use_smooth
					f_v = [(vi, me_verts[v_idx]) for vi, v_idx in enumerate(f.vertices)]

					if f_smooth: # Use smoothed normals
						for vi, v in f_v:
							fw('%d, ' % (globalNormals[veckey3d(v.normal)]))
					else: # No smoothing, face normals
						no = globalNormals[veckey3d(f.normal)]
						for vi, v in f_v:
							fw('%d,' % no)
				file.seek(-1, 1)
				fw(' ]')

			# Texcoords
			if faceuv:
				fw(',\n        [ ')
				for f, f_index in face_index_pairs:
					f_smooth = f.use_smooth
					f_v = [(vi, me_verts[v_idx]) for vi, v_idx in enumerate(f.vertices)]

					for vi, v in f_v:
						fw('%d,' % (totuvco + uv_face_mapping[f_index][vi]))
				file.seek(-1, 1)
				fw(' ]')
			fw(']\n')

			# Make the indices global rather then per mesh
			totverts += len(me_verts)
			if faceuv:
				totuvco += uv_unique_count

			# clean up
			bpy.data.meshes.remove(me)

		if ob_main.dupli_type != 'NONE':
			ob_main.dupli_list_clear()

	fw('}\n')		
	file.close()

	# copy all collected files.
	bpy_extras.io_utils.path_reference_copy(copy_set)

	print("OBJ Export time: %.2f" % (time.time() - time1))


def _write(context, filepath,
			  EXPORT_TRI,  # ok
			  EXPORT_NORMALS,  # not yet
			  EXPORT_UV,  # ok
			  EXPORT_APPLY_MODIFIERS,  # ok
			  EXPORT_KEEP_VERT_ORDER,
			  EXPORT_SEL_ONLY,  # ok
			  EXPORT_ANIMATION,
			  EXPORT_GLOBAL_MATRIX,
			  ):  # Not used

	base_name, ext = os.path.splitext(filepath)
	context_name = [base_name, '', '', ext]  # Base name, scene name, frame number, extension

	scene = context.scene

	# Exit edit mode before exporting, so current object states are exported properly.
	if bpy.ops.object.mode_set.poll():
		bpy.ops.object.mode_set(mode='OBJECT')

	orig_frame = scene.frame_current

	# Export an animation?
	if EXPORT_ANIMATION:
		scene_frames = range(scene.frame_start, scene.frame_end + 1)  # Up to and including the end frame.
	else:
		scene_frames = [orig_frame]  # Dont export an animation.

	# Loop through all frames in the scene and export.
	for frame in scene_frames:
		if EXPORT_ANIMATION:  # Add frame to the filepath.
			context_name[2] = '_%.6d' % frame

		scene.frame_set(frame, 0.0)
		if EXPORT_SEL_ONLY:
			objects = context.selected_objects
		else:
			objects = scene.objects

		full_path = ''.join(context_name)

		# erm... bit of a problem here, this can overwrite files when exporting frames. not too bad.
		# EXPORT THE FILE.
		write_file(full_path, objects, scene,
				   EXPORT_TRI,
				   EXPORT_NORMALS,
				   EXPORT_UV,
				   EXPORT_APPLY_MODIFIERS,
				   EXPORT_KEEP_VERT_ORDER,
				   EXPORT_GLOBAL_MATRIX,
				   )

	scene.frame_set(orig_frame, 0.0)

	# Restore old active scene.
#   orig_scene.makeCurrent()
#   Window.WaitCursor(0)


"""
Currently the exporter lacks these features:
* multiple scene export (only active scene is written)
* particles
"""


def save(operator, context, filepath="",
		 use_triangles=False,
		 use_normals=False,
		 use_uvs=True,
		 use_mesh_modifiers=True,
		 keep_vertex_order=False,
		 use_selection=True,
		 use_animation=False,
		 global_matrix=None,
		 ):

	_write(context, filepath,
		   EXPORT_TRI=use_triangles,
		   EXPORT_NORMALS=use_normals,
		   EXPORT_UV=use_uvs,
		   EXPORT_APPLY_MODIFIERS=use_mesh_modifiers,
		   EXPORT_KEEP_VERT_ORDER=keep_vertex_order,
		   EXPORT_SEL_ONLY=use_selection,
		   EXPORT_ANIMATION=use_animation,
		   EXPORT_GLOBAL_MATRIX=global_matrix,
		   )

	return {'FINISHED'}

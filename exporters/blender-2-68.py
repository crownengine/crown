#
# Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
# License: https://github.com/taylor001/crown/blob/master/LICENSE
#

bl_info = {
	"name"        : "Crown Engine .mesh format",
	"author"      : "Daniele Bartolini",
	"blender"     : (2, 58, 0),
	"location"    : "File > Import-Export",
	"description" : "Export Crown Engine .mesh models",
	"warning"     : "",
	"tracker_url" : "https://github.com/taylor001/crown",
	"support"     : 'OFFICIAL',
	"category"    : "Import-Export"
}

import bpy
import bpy_extras.io_utils
import mathutils
import os

# Triangulates the mesh me
def mesh_triangulate(me):
	import bmesh
	bm = bmesh.new()
	bm.from_mesh(me)
	bmesh.ops.triangulate(bm, faces=bm.faces)
	bm.to_mesh(me)
	bm.free()

def write_file(file
	, objects
	, scene
	, EXPORT_TRI=True
	, EXPORT_NORMALS=True
	, EXPORT_UV=True
	, EXPORT_APPLY_MODIFIERS=True
	):

	def veckey3d(v):
		return round(v.x, 6), round(v.y, 6), round(v.z, 6)

	def veckey2d(v):
		return round(v[0], 6), round(v[1], 6)

	def fw(string):
		file.write(bytes(string, 'UTF-8'))

	def write_mesh(ob, name):
		try:
			me = ob.to_mesh(scene, EXPORT_APPLY_MODIFIERS, 'PREVIEW', calc_tessface=False)
		except RuntimeError:
			me = None

		if me is None:
			return

		if EXPORT_TRI:
			mesh_triangulate(me)

		if EXPORT_UV:
			faceuv = len(me.uv_textures) > 0
			if faceuv:
				uv_texture = me.uv_textures.active.data[:]
				uv_layer = me.uv_layers.active.data[:]
		else:
			faceuv = False

		if not (len(me.polygons) + len(me.vertices)):
			bpy.data.meshes.remove(me)
			return

		if EXPORT_NORMALS and me.polygons:
			me.calc_normals()
			if faceuv:
				me.calc_tangents(me.uv_layers.active.name)

		normals = {}
		tangents = {}
		bitangents = {}
		texcoords = {}

		def write_positions(me):
			for v in me.vertices:
				x, y, z = veckey3d(v.co)
				fw(" %.6f %.6f %.6f" % (x, z, y))

		def write_normals(me):
			for f in me.polygons:
				if f.use_smooth:
					for v_idx in f.vertices:
						v = me.vertices[v_idx]
						noKey = veckey3d(v.normal)
						if noKey not in normals:
							normals[noKey] = len(normals)
							x, y, z = noKey
							fw(" %.6f %.6f %.6f" % (x, z, y))
				else:
					noKey = veckey3d(f.normal)
					if noKey not in normals:
						normals[noKey] = len(normals)
						x, y, z = noKey
						fw(" %.6f %.6f %.6f" % (x, z, y))

		def write_tangents(me):
			for l in me.loops:
				noKey = veckey3d(l.tangent)
				if noKey not in tangents:
					tangents[noKey] = len(tangents)
					fw(" %.6f %.6f %.6f" % noKey)

		def write_bitangents(me):
			for l in me.loops:
				n = l.normal
				t = l.tangent
				sgn = l.bitangent_sign
				nt = n.cross(t)
				noKey = veckey3d(sgn * nt)
				if noKey not in bitangents:
					bitangents[noKey] = len(bitangents)
					fw(" %.6f %.6f %.6f" % noKey)

		def write_texcoords(me):
			for f in me.polygons:
				for loop_index in f.loop_indices:
					uv = uv_layer[loop_index].uv
					uvkey = veckey2d(uv)
					if uvkey not in texcoords:
						texcoords[uvkey] = len(texcoords)
						u, v = uvkey
						fw(" %.6f %.6f" % (u, 1.0 - v))

		def write_position_indices(me):
			for f in me.polygons:
				for v_idx in f.vertices:
					v = me.vertices[v_idx]
					fw(' %d' % v.index)

		def write_normal_indices(me, normals):
			for f in me.polygons:
				if f.use_smooth:
					for v_idx in f.vertices:
						v = me.vertices[v_idx]
						fw(' %d' % (normals[veckey3d(v.normal)]))
				else:
					no = normals[veckey3d(f.normal)]
					for v_idx in f.vertices:
						fw(' %d' % no)

		def write_tangent_indices(me, tangents):
			for l in me.loops:
				fw(' %d' % (tangents[veckey3d(l.tangent)]))

		def write_bitangent_indices(me, bitangents):
			for l in me.loops:
				n = l.normal
				t = l.tangent
				sgn = l.bitangent_sign
				nt = n.cross(t)
				fw(' %d' % bitangents[veckey3d(sgn * nt)])

		def write_texcoord_indices(me, texcoords):
			for f in me.polygons:
				for loop_index in f.loop_indices:
					uv = uv_layer[loop_index].uv
					fw(' %d' % texcoords[veckey2d(uv)])

		def write_indices(me, normals, texcoords):
			fw("        indices = {\n");
			fw("            size = %d\n" % (3*len(me.polygons))) # Assume triagles
			fw("            data = [\n")
			fw("                ["); write_position_indices(me); fw(" ]\n")
			if EXPORT_NORMALS:
				fw("                ["); write_normal_indices(me, normals); fw(" ]\n")
			if faceuv:
				fw("                ["); write_texcoord_indices(me, texcoords); fw(" ]\n")
				fw("                ["); write_tangent_indices(me, tangents); fw(" ]\n")
				fw("                ["); write_bitangent_indices(me, bitangents); fw(" ]\n")
			fw("            ]\n")
			fw("        }\n")

		fw("    \"%s\" = {\n" % name)
		fw("        position = ["); write_positions(me); fw(" ]\n")
		if EXPORT_NORMALS:
			fw("        normal = ["); write_normals(me); fw(" ]\n")
		if faceuv:
			fw("        texcoord = ["); write_texcoords(me); fw(" ]\n")
			fw("        tangent = ["); write_tangents(me); fw(" ]\n")
			fw("        bitangent = ["); write_bitangents(me); fw(" ]\n")
		write_indices(me, normals, texcoords);
		fw("    }\n")

		bpy.data.meshes.remove(me)

	def write_geometries(objects):
		fw("geometries = {\n")
		for o in objects:
			if o.type == 'MESH': write_mesh(o, o.name)
		fw("}\n")

	def write_nodes(objects):
		fw("nodes = {\n")
		for o in objects:
			if o.type == 'MESH':
				ml = o.matrix_local.copy()
				ml.transpose()

				ml[0][1], ml[0][2] = ml[0][2], ml[0][1]
				ml[1][1], ml[1][2] = ml[1][2], ml[1][1]
				ml[2][1], ml[2][2] = ml[2][2], ml[2][1]
				ml[3][1], ml[3][2] = ml[3][2], ml[3][1]

				fw('    \"%s\" = {\n' % o.name)
				fw ('        matrix_local = [ ')
				fw ('%f %f %f %f ' % ml[0][:])
				fw ('%f %f %f %f ' % ml[2][:])
				fw ('%f %f %f %f ' % ml[1][:])
				fw ('%f %f %f %f ' % ml[3][:])
				fw (']\n')
				fw('    }\n')
		fw("}\n")

	write_geometries(objects)
	write_nodes(objects)

def _write(context
	, filepath
	, EXPORT_TRI
	, EXPORT_NORMALS
	, EXPORT_UV
	, EXPORT_APPLY_MODIFIERS
	, EXPORT_SEL_ONLY
	):

	scene = context.scene

	# Exit edit mode before exporting, so current object states are exported properly.
	if bpy.ops.object.mode_set.poll():
		bpy.ops.object.mode_set(mode='OBJECT')

	if EXPORT_SEL_ONLY:
		objects = context.selected_objects
	else:
		objects = scene.objects

	f = open(filepath, "wb")
	write_file(f
		, objects
		, scene
		, EXPORT_TRI
		, EXPORT_NORMALS
		, EXPORT_UV
		, EXPORT_APPLY_MODIFIERS
	)
	f.close()

def save(operator
	, context
	, filepath=""
	, use_triangles=False
	, use_normals=False
	, use_uvs=True
	, use_mesh_modifiers=True
	, use_selection=True
	, use_animation=False
	, global_matrix=None
	):

	_write(context
		, filepath
		, EXPORT_TRI=use_triangles
		, EXPORT_NORMALS=use_normals
		, EXPORT_UV=use_uvs
		, EXPORT_APPLY_MODIFIERS=use_mesh_modifiers
		, EXPORT_SEL_ONLY=use_selection
		)

	return {'FINISHED'}

from bpy.props import (BoolProperty, FloatProperty, StringProperty, EnumProperty)
from bpy_extras.io_utils import (ExportHelper, path_reference_mode, axis_conversion)

class MyExporter(bpy.types.Operator, ExportHelper):
	"""Save a Crown Engine .mesh model"""

	bl_idname = "model.mesh"
	bl_label = 'Export .mesh'
	bl_options = {'PRESET'}

	filename_ext = ".mesh"
	filter_glob = StringProperty(default="*.mesh;"
		, options={'HIDDEN'}
		)

	use_selection = BoolProperty(name="Selection Only"
		, description="Export selected objects only"
		, default=False
		)

	use_mesh_modifiers = BoolProperty(name="Apply Modifiers"
		, description="Apply modifiers (preview resolution)"
		, default=True
		)

	use_normals = BoolProperty(name="Include Normals"
		, description=""
		, default=True
		)

	use_uvs = BoolProperty(name="Include UVs"
		, description="Write out the active UV coordinates"
		, default=True
		)

	use_triangles = BoolProperty(name="Triangulate Faces"
		, description="Convert all faces to triangles"
		, default=True
		)

	global_scale = FloatProperty(name="Scale"
		, min=0.01
		, max=1000.0
		, default=1.0
		)

	def execute(self, context):
		from mathutils import Matrix
		keywords = self.as_keywords(ignore=("axis_forward"
			, "axis_up"
			, "global_scale"
			, "check_existing"
			, "filter_glob"
			))

		return save(self, context, **keywords)

def menu_func_export(self, context):
	self.layout.operator(MyExporter.bl_idname, text="Crown Engine (.mesh)")

def register():
	bpy.utils.register_class(MyExporter)
	bpy.types.INFO_MT_file_export.append(menu_func_export)

def unregister():
	bpy.utils.unregister_class(MyExporter)
	bpy.types.INFO_MT_file_export.remove(menu_func_export)

# This allows you to run the script directly from blenders text editor
# to test the addon without having to install it.
if __name__ == "__main__":
	register()

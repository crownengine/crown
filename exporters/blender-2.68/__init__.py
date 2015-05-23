#
# Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
# License: https://github.com/taylor001/crown/blob/master/LICENSE
#

bl_info = {
	"name": "Crown Engine .mesh format",
	"author": "Daniele Bartolini",
	"blender": (2, 58, 0),
	"location": "File > Import-Export",
	"description": "Export Crown Engine .mesh models",
	"warning": "",
	"tracker_url": "https://github.com/taylor001/crown",
	"support": 'OFFICIAL',
	"category": "Import-Export"}

if "bpy" in locals():
	import imp
	if "export_crown" in locals():
		imp.reload(export_crown)

import bpy
from bpy.props import (BoolProperty,
	FloatProperty,
	StringProperty,
	EnumProperty,
	)
from bpy_extras.io_utils import (ExportHelper,
	path_reference_mode,
	axis_conversion,
	)

class MyExporter(bpy.types.Operator, ExportHelper):
	"""Save a Crown Engine .mesh model"""

	bl_idname = "model.mesh"
	bl_label = 'Export .mesh'
	bl_options = {'PRESET'}

	filename_ext = ".mesh"
	filter_glob = StringProperty(
		default="*.mesh;",
		options={'HIDDEN'},
		)

	use_selection = BoolProperty(
		name="Selection Only",
		description="Export selected objects only",
		default=False,
		)

	use_mesh_modifiers = BoolProperty(
		name="Apply Modifiers",
		description="Apply modifiers (preview resolution)",
		default=True,
		)

	use_normals = BoolProperty(
		name="Include Normals",
		description="",
		default=True,
		)

	use_uvs = BoolProperty(
		name="Include UVs",
		description="Write out the active UV coordinates",
		default=True,
		)

	use_triangles = BoolProperty(
		name="Triangulate Faces",
		description="Convert all faces to triangles",
		default=True,
		)

	global_scale = FloatProperty(
		name="Scale",
		min=0.01, max=1000.0,
		default=1.0,
		)

	def execute(self, context):
		from . import export_crown

		from mathutils import Matrix
		keywords = self.as_keywords(ignore=("axis_forward",
					"axis_up",
					"global_scale",
					"check_existing",
					"filter_glob",
					))

		scale_mt = Matrix.Scale(self.global_scale, 4)
		global_matrix = (scale_mt * axis_conversion(to_forward='-Z', to_up='Y').to_4x4())

		keywords["global_matrix"] = global_matrix
		return export_crown.save(self, context, **keywords)

def menu_func_export(self, context):
	self.layout.operator(MyExporter.bl_idname, text="Crown Engine (.mesh)")

def register():
	bpy.utils.register_module(__name__)
	bpy.types.INFO_MT_file_export.append(menu_func_export)

def unregister():
	bpy.utils.unregister_module(__name__)
	bpy.types.INFO_MT_file_export.remove(menu_func_export)

if __name__ == "__main__":
	register()

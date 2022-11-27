project('level_editor', ['vala', 'c'])
deps = [
  dependency('gdk-3.0'),
  dependency('gee-0.8'),
  dependency('gio-2.0'),
  dependency('glib-2.0'),
  dependency('gtk+-3.0')
]
src = files([
  './tools/api/engine_api.vala',
  './tools/config.vala',
  './tools/core/console_client.vala',
  './tools/core/database.vala',
  './tools/core/guid.vala',
  './tools/core/json/json.vala',
  './tools/core/json/sjson.vala',
  './tools/core/math/math_utils.vala',
  './tools/core/math/matrix4x4.vala',
  './tools/core/math/quaternion.vala',
  './tools/core/math/vector2.vala',
  './tools/core/math/vector3.vala',
  './tools/core/math/vector4.vala',
  './tools/level_editor/action_type.vala',
  './tools/level_editor/data_compiler.vala',
  './tools/level_editor/editor_view.vala',
  './tools/level_editor/launcher.vala',
  './tools/level_editor/level.vala',
  './tools/level_editor/level_editor.vala',
  './tools/level_editor/level_layers_tree_view.vala',
  './tools/level_editor/level_tree_view.vala',
  './tools/level_editor/panel_new_project.vala',
  './tools/level_editor/panel_projects_list.vala',
  './tools/level_editor/panel_welcome.vala',
  './tools/level_editor/preferences_dialog.vala',
  './tools/level_editor/project.vala',
  './tools/level_editor/project_browser.vala',
  './tools/level_editor/project_store.vala',
  './tools/level_editor/properties_view.vala',
  './tools/level_editor/resource_chooser.vala',
  './tools/level_editor/sprite_import_dialog.vala',
  './tools/level_editor/statusbar.vala',
  './tools/level_editor/unit.vala',
  './tools/level_editor/user.vala',
  './tools/resource/mesh_resource.vala',
  './tools/resource/resource_id.vala',
  './tools/resource/sound_resource.vala',
  './tools/resource/sprite_resource.vala',
  './tools/resource/texture_resource.vala',
  './tools/resource/types.vala',
  './tools/widgets/app_chooser_button.vala',
  './tools/widgets/check_box.vala',
  './tools/widgets/clamp.vala',
  './tools/widgets/color_button_vector3.vala',
  './tools/widgets/combo_box_map.vala',
  './tools/widgets/console_view.vala',
  './tools/widgets/entry_double.vala',
  './tools/widgets/entry_position.vala',
  './tools/widgets/entry_rotation.vala',
  './tools/widgets/entry_scale.vala',
  './tools/widgets/entry_search.vala',
  './tools/widgets/entry_text.vala',
  './tools/widgets/entry_vector2.vala',
  './tools/widgets/entry_vector3.vala',
  './tools/widgets/entry_vector4.vala',
  './tools/widgets/resource_chooser_button.vala',
])

gnome = import('gnome')

resources_xml = files('tools\\level_editor\\resources\\resources.gresource.xml')
resources = gnome.compile_resources('resources', resources_xml, source_dir: '.\\tools\\level_editor\\resources\\')

vala_args = [ 
  '--target-glib=2.38', # minimum glib version required for composite templates
  '--define=CROWN_PLATFORM_WINDOWS',
  '--gresources', resources_xml,
]

executable('app',
           [src, resources],
           dependencies: deps,
           vala_args: [vala_args])

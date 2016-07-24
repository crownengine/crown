OBJS = \
	level_editor_create_project.ui     \
	level_editor_menu.xml              \
	level_editor_preferences_dialog.ui \
\
	icons/128x128/pepper.png           \
	icons/16x16/pepper.png             \
	icons/24x24/pepper.png             \
	icons/256x256/pepper.png           \
	icons/32x32/pepper.png             \
	icons/64x64/pepper.png             \
\
	icons/theme/axis-local.png         \
	icons/theme/axis-world.png         \
	icons/theme/layer-locked.png       \
	icons/theme/layer-visible.png      \
	icons/theme/level-layers.png       \
	icons/theme/level-tree.png         \
	icons/theme/reference-local.png    \
	icons/theme/reference-world.png    \
	icons/theme/run.png                \
	icons/theme/snap-to-grid.png       \
	icons/theme/tool-move.png          \
	icons/theme/tool-place.png         \
	icons/theme/tool-rotate.png        \
	icons/theme/tool-scale.png         \

resources.c: resources.xml $(OBJS)
	glib-compile-resources resources.xml --target=resources.c --generate

clean:
	-@rm resources.c

OBJS = \
	level_editor/level_editor.xml      \
	level_editor/preferences_dialog.ui \
	level_editor/project_dialog.ui     \
\
	ui/icons/128x128/pepper.png \
	ui/icons/16x16/pepper.png   \
	ui/icons/24x24/pepper.png   \
	ui/icons/256x256/pepper.png \
	ui/icons/32x32/pepper.png   \
	ui/icons/64x64/pepper.png   \
\
	ui/icons/theme/16x16/actions/axis-local.png      \
	ui/icons/theme/16x16/actions/axis-world.png      \
	ui/icons/theme/16x16/actions/layer-locked.png    \
	ui/icons/theme/16x16/actions/layer-visible.png   \
	ui/icons/theme/16x16/actions/level-layers.png    \
	ui/icons/theme/16x16/actions/level-tree.png      \
	ui/icons/theme/16x16/actions/reference-local.png \
	ui/icons/theme/16x16/actions/reference-world.png \
	ui/icons/theme/16x16/actions/run.png             \
	ui/icons/theme/16x16/actions/snap-to-grid.png    \
	ui/icons/theme/16x16/actions/tool-move.png       \
	ui/icons/theme/16x16/actions/tool-place.png      \
	ui/icons/theme/16x16/actions/tool-rotate.png     \
	ui/icons/theme/16x16/actions/tool-scale.png      \

.PHONY: all
all: gresources.xml $(OBJS)
	glib-compile-resources gresources.xml --target=gresources.c --generate

.PHONY: clean
clean:
	-@rm gresources.c

.PHONY: rebuild
rebuild: clean all

OBJS = \
	level_editor/level_editor.xml      \
	level_editor/preferences_dialog.ui \
	level_editor/project_dialog.ui     \
\
	ui/icons/128x128/pepper.png        \
	ui/icons/16x16/pepper.png          \
	ui/icons/24x24/pepper.png          \
	ui/icons/256x256/pepper.png        \
	ui/icons/32x32/pepper.png          \
	ui/icons/64x64/pepper.png          \
\
	ui/icons/theme/axis-local.png      \
	ui/icons/theme/axis-world.png      \
	ui/icons/theme/layer-locked.png    \
	ui/icons/theme/layer-visible.png   \
	ui/icons/theme/level-layers.png    \
	ui/icons/theme/level-tree.png      \
	ui/icons/theme/reference-local.png \
	ui/icons/theme/reference-world.png \
	ui/icons/theme/run.png             \
	ui/icons/theme/snap-to-grid.png    \
	ui/icons/theme/tool-move.png       \
	ui/icons/theme/tool-place.png      \
	ui/icons/theme/tool-rotate.png     \
	ui/icons/theme/tool-scale.png      \

.PHONY: all
all: gresources.xml $(OBJS)
	glib-compile-resources gresources.xml --target=gresources.c --generate

.PHONY: clean
clean:
	-@rm gresources.c

.PHONY: rebuild
rebuild: clean all

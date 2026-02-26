Changelog
=========

.. _v0.62.0:

:ref:`0.62.0 --- DD MMM YYYY <v0.62.0>`
---------------------------------------

**Fixes**

Tools: fix Delete key being disabled in entries.
Runtime: fixed a crash in the Graph subsystem.

.. _v0.61.0:

:ref:`0.61.0 --- 23 Feb 2026 <v0.61.0>`
---------------------------------------

**New Features and Improvements**

* Tools: added a new HSV color picker.
* Tools: added a new "flythrough" camera mode for navigating viewports.
* Tools: added textual tooltips to main object types and prominent interface elements.
* Tools: crown-launcher can now be executed from any directory.
* Tools: added the ability to show all files (w/ or w/o extension) as well as mapped directories in the Project Browser.
* Tools: added the ability to choose which external tool to use for opening models, sounds and fonts.
* Tools: added the ability to reset properties to default by right-clicking on them.
* Tools: the Level Editor will now automatically open a new temporary project when the Projects List is empty.
* Tools: the game will now be killed if it takes to long to exit when testing it from the Level Editor.
* Tools: added the ability to unselect all objects with ``Shift+Ctrl+A`` keyboard shortcut.
* Runtime: Linux: switched to Vulkan for rendering.
* Runtime: added support to stencil testing in shaders.
* Runtime: changed the semantic of collision filters. See :ref:`Global Physics Config` for details.
* Runtime: added a new ``collides_with_all_except`` field to the ``collision_filters`` object in the :ref:`Global Physics Config`.
* Data Compiler: resource packages are now LZ4-compressed.
* Lua: ``Window.set_cursor_mode()`` will now return a boolean value to indicate if the operation succeeded.

0.60.0 --- 30 Dec 2025
----------------------

**New Features and Improvements**

* Tools: added a new Unit Editor to inspect and modify Unit prefabs.
* Tools: added a new State Machine Editor for editing 3D skeletal mesh and sprite animations.
* Tools: added a new Object Editor to inspect and modify via a generic UI most resource types that were previously opened as text files.
* Tools: added the ability to search any kind of resources in the Project Browser. As a consequence, the old Resource Chooser dialog has been removed.
* Tools: double-clicking an object in the Level Tree will now frame it in the Editor Viewport.
* Tools: units created via the FBX importer will now be named according to their corresponding node's name.
* Tools: added a generic editor to inspect and modify all registered resource types. Previously resources with no dedicated editor were opened in a plain-text editor.
* Tools: Windows: improved UI fonts rendering.
* Runtime: omni lights will now render shadow maps.
* Runtime: added the ability to reload animation state machines.
* Runtime: added an end-of-options marker to stop the runtime from parsing game-specific command line options.
* Runtime: the ``mesh`` shader will now expect normal maps to be encoded in the BC5 texture format.
* Data Compiler: added ``linear`` and ``premultiply_alpha`` options to the texture resource compiler.
* Lua: added ``Device.build()`` to retrieve the runtime's build configuration.
* Lua: added ``PhysicsWorld.actor_debug_draw()`` and ``PhysicsWorld.mover_debug_draw()``.
* Lua: added ``PhysicsWorld.mover_set_height()``, ``PhysicsWorld.mover_set_radius()``, ``PhysicsWorld.mover_center()`` and ``PhysicsWorld.mover_set_center()``.
* Lua: added ``AnimationStateMachine.create()``, ``AnimationStateMachine.destroy()`` and ``AnimationStateMachine.set_state_machine()``.
* Lua: ``AnimationStateMachine.variable_id()`` will now return ``nil`` if the variable does not exist.
* Lua: the ``update()`` script component callback is now optional.

**Fixes**

* Tools: fixed creation of animation state machine resources.
* Tools: the Project Browser will now reject dropped files while showing Favorites.
* Tools: fixed new levels containing GUIDs from their template resource.
* Tools: the Level Tree is now correctly restored after a search.
* Tools: fixed expanders not reflecting their state in some cases.
* Tools: fixed some properties in the Inspector not being set correctly when read the first time.
* Tools: fixed meshes nodes being reset unexpectedly when selecting the corresponding unit in the Inspector.
* Tools: fixed some properties unnecessarily requiring multiple undo steps to be set to their previous value.
* Tools: fixed importing non-animated FBX files in some circumnstances.
* Tools: duplicating objects in the Level Editor will now correctly select the duplicated objects instead of the ones being duplicated.
* Runtime: fixed rendering of local lights with shadows disabled.
* Runtime: fixed shading of meshes with non-uniform scaling.
* Runtime: Windows: fixed a number of misdetected keyboard keys.
* Runtime: Windows: fixed windows firewall popping up at startup.
* Runtime: Windows: fixed rendering of spot lights' shadow maps.
* Runtime: Linux: fixed a crash at startup due to some X11 libraries not being found.
* Runtime: fixed a regression that caused levels with many objects to not load.
* Runtime: fixed a crash when reloading materials.
* Runtime: fixed linking transforms in a SceneGraph not working in some circumnstances.
* Runtime: fixed a crash when reloading non-trivial units.
* Runtime: fixed imperfect spot lights' cone coverage.
* Runtime: fixed interrupted sun shadow map when too close to the viewer.
* Runtime: fixed destroying units with children.
* Runtime: fixed some UnitIDs not being released when destroying units with state machines.
* Runtime: fixed a crash when playing empty mesh animations.
* Runtime: fixed a leak when destroying worlds with animation state machines.
* Data Compiler: fixed a crash when compiling some mesh skeletons.
* Data Compiler: fixed compiling funny FBX animation files.
* Lua: fixed a memory leak when executing some script component callbacks.

0.59.0 --- 02 Oct 2025
----------------------

**New Features and Improvements**

* Samples: improved 01-physics sample with a first-person character mover, kinematic platforms, triggers and more.
* Manual: added a new 'Writing Gameplay' section to the manual.
* Tools: saving a Unit as a prefab now automatically replaces that Unit in the level with the saved prefab.
* Runtime: added a new Mover component for controlling characters that implements a standard collide-and-slide algorithm.
* Runtime: added a new .render_config resource for configuring the renderer, along with a `render_config` property in the boot files to allow selecting the desired config.
* Runtime: added a shadow mapping implementation to spot lights.
* Runtime: added a new Global Lighting component for controlling the skydome's appearance and other global lighting parameters.
* Runtime: added a new Bloom component.
* Runtime: added a new Tonemap component.
* Runtime: added a new dedicated skydome renderer.
* Runtime: added support for emission maps in the `mesh` shader.
* Runtime: added support for UV scale/offset in the `mesh` shader.
* Runtime: added support for MSAA.
* Runtime: added a new `spinning_friction` property to physics materials that you can specify in `global.physics_config`.
* Runtime: added configurable physics `step_frequency` and `max_substeps` via the `physics` property in the boot files.
* Runtime: GUIs are now automatically destroyed when the world that contains them is destroyed.
* Runtime: the sound subsystem now uses a more realistic attenuation model based on inverse distance of sounds from the listener.
* Runtime: added the ability to play mono sounds as 2d sounds (i.e. without attenuation nor spatialization).
* Lua: changed the order of arguments of ``World.play_sound()`` to support 2d sounds playback with a single API.
* Lua: removed 'pose' parameter from the following component creation functions:

	- ``World.camera_create()``
	- ``RenderWorld.mesh_create()``
	- ``RenderWorld.sprite_create()``
	- ``RenderWorld.light_create()``

	The affected components will now read the initial pose from the unit's transform.
* Lua: added ``World.destroy_level``.
* Lua: added ``collision_end()`` script component callback.
* Lua: renamed ``collision()`` script component callback to ``collision_stay()``.
* Lua: added 'world' and 'other_actor' parameters to ``collision_begin()`` and ``collision_stay()``. The first 'unit' argument now refers to the unit receiving the event, other parameters have been changed accordingly.
* Lua: added ``trigger_enter()`` and ``trigger_leave()`` script component callbacks.

**Fixes**

* Tools: fixed setting texture parameters in the texture settings editor.
* Tools: fixed 'Cast Shadows' property displaying the wrong value on old units.
* Runtime: fixed a crash when too many lights were spawned in a level.
* Runtime: fixed cascaded shadows artifacts in some circumnstances.
* Runtime: fixed creating material uniforms of the wrong type in some circumnstances.
* Runtime: fixed camera components not getting destroyed.
* Runtime: fixed range-based culling of sound sources.
* Runtime: fixed collision begin/stay generation.
* Runtime: fixed setting 'disabled' cursor mode on HTML5.
* Compiler: fixed a memory leak when compiling units or levels with deleted components.

0.58.0 --- 01 Aug 2025
----------------------

**New Features and Improvements**

* Tools: added Fog component.
* Tools: the sprite importer will now generate a default sprite_animation and state_machine for all imported sprites.
* Tools: added an option to automatically compute the collider shape when importing sprites.
* Tools: added the ability to set the Sprite Renderer's `flip_x` and `flip_y` properties in the Inspector.
* Tools: the FBX importer will now create a state machine resource and component when 'Unit' and 'New Skeleton' options are selected.
* Tools: resources can now be imported directly by dragging files into the Project Browser.
* Tools: upgraded FBX loader library (ufbx) to v0.20.0.
* Compiler: optimized mesh data compilation times.

**Fixes**

* Tools: fixed hot-reloading of Unit scripts.
* Tools: fixed Editor Viewport lag when using pointing devices with high polling rate.
* Tools: fixed missing sounds in the Level Tree.
* Tools: fixed a number of issues in the Level Tree when adding/removing components to units.
* Tools: fixed inability to select state machines from the "Animation State Machine" component in the Inspector.
* Tools: fixed missing SKINNING define in materials when importing animated FBX meshes.
* Tools: fixed shadows when rendering animated meshes.
* Tools: fixed Level Tree showing previous objects when creating new levels.
* Tools: fixed a crash when sending malformed scripts via Console.
* Runtime: Linux: fixed ``Window.set_fullscreen()`` to put the window fullscreen without delays.
* Compiler: the compiler will now return with an error if any source dir does not exist.
* Compiler: fixed an issue that caused a crash in the runtime when trying to reload a resource after deleting it.
* Compiler: fixed a memory leak when deleting components in a Unit in some cases.

0.57.0 --- 13 Jun 2025
----------------------

**New Features and Improvements**

* Tools: improved the way numerical inputs show decimals.

**Fixes**

* Runtime: Windows: fixed reloading of resources.
* Compiler: fixed lights not generating shadows.
* Compiler: fixed excessive amount of vertex data being generated when compiling meshes.
* Tools: fixed sorting of items in the Level Tree.
* Tools: fixed sprite depth property not being saved when modified in the Inspector.
* Tools: fixed a variety of issues with units-with-children.
* Tools: fixed inability of selecting FBX nodes in the Inspector.
* Tools: fixed misdetected clicks in the Project Browser in some circumnstances.
* Tools: Windows: fixed rendering of selected objects' outlines.

0.56.0 --- 04 Jun 2025
----------------------

**New Features and Improvements**

* Runtime: added cascaded shadow mapping.
* Runtime: added sounds streaming and OGG/Vorbis audio files support.
* Runtime: HTML5: added Touch input device support.
* Tools: added the ability to save level units as prefabs.
* Tools: added mouse-only shortcuts (middle-button/wheel) to tumble/dolly the camera.
* Tools: sped up deployers by enabling incremental data compilation.
* Tools: added support to JPEG image format for textures.
* Lua: added ``Gui.text_extents()``.
* Lua: added ``Light.set_cast_shadows()``.
* Lua: added ``Mesh.set_cast_shadows()``.
* Lua: added ``SoundWorld.set_group_volume()`` and a corresponding group parameter to ``World.play_sound()``.
* Lua: added an optional exit_code parameter to ``Device.quit()``.
* Lua: added an optional rays parameter to ``DebugLine.add_cone()``.
* Lua: the color parameter in all ``Gui`` functions is now optional and defaults to ``Color4.white()``.
* Lua: the material_resource parameter in ``Gui.text()`` and ``Gui.text_3d()`` is now optional.

**Fixes**

* Runtime: fixed spurious activations of physics bodies.
* Runtime: HTML5: fixed GL_INVALID_OPERATION (or missing geometry) when rendering untextured meshes.
* Compiler: fixed 'static_compile' being visible to external shader resources.
* Compiler: fixed shaders not including code in some circumnstances.
* Tools: fixed collider generation when importing sprites.
* Tools: fixed missing outlines on invisible objects.
* Tools: fixed importing sounds.
* Tools: fixed drag & drop of sound resources into the Editor View.
* Lua: fixed several issues when calling ``Gui.text*()`` with utf8-encoded strings.
* Lua: fixed start-of-line character placement and some other rendering issues in ``Gui.text*()``.

0.55.0 --- 03 Apr 2025
----------------------

**Tools**

* Added the ability to import skeletons and animation clips from FBX files.
* Added the ability to skip creating units when importing FBX files.
* Added the ability to create new materials from the Project Browser.
* Fixed a crash when editing many objects simultaneusly.
* Fixed a crash when trying to load levels with many objects.
* Fixed an issue that prevented undo/redo operations to be executed in bulk.
* Fixed an issue that caused large levels to load slowly.
* Fixed an issue that prevented importing DCC data with funny extensions.
* Fixed revealing resources while the Project Browser was hidden.
* Fixed importing FBX files with references to non-existing textures.
* Fixed an issue that caused level's editor camera to be saved incorrectly in some cases.
* The HTML5 deployer will now use EMSCRIPTEN environment variable to locate emsdk.
* Windows: fixed the Android deployer generating bad native library folder.

**Runtime**

* Added physically-based rendering (PBR) pipeline.
* Added support to shaders and materials hot-reloading.
* Increased the maximum number of lines that can be drawn with DebugLine.
* Increased the maximum number of texture samplers per shader to 16.
* Added tangent/bitangent vertex attributes support in .mesh resources.
* Fixed a crash when moving many objects simultaneusly.
* Fixed a crash when reloading unloaded or unsupported resources.
* Fixed setting kinematic actor's position and rotation.
* Fixed an issue that caused all samplers in a material to be set to the same texture.
* Fixed GL_INVALID_OPERATION on HTML5.
* Data Compiler: 'includes' key in shaders can now be specified as an array to include code from multiple locations.
* Data Compiler: shader #defines are now correctly exposed in vs_input_output and fs_input_output.
* Data Compiler: fixed shader compiler ignoring errors when parsing included files.
* Data Compiler: fixed and issue that caused OOM when compiling levels with many units.
* Lua: fixed max temporaries check failing to trigger in some circumnstances.
* Lua API: fixed ``Math.ray_obb_intersection()`` with scaled OBBs.
* Lua API: added ``Math.obb_merge()``.
* Lua API: added ``SceneGraph.parent()``.
* Lua API: implemented ``PhysicsWorld.actor_{enable,disable}_collision()`` and ``PhysicsWorld.actor_set_collision_filter()``.

0.54.0 --- 13 Jan 2025
----------------------

**General**

* Switched to right-handed Z-up coordinate system.

**Tools**

* Items in the Project Browser can now be sorted by name, type, size or last modification time.
* Added a list-view mode to the Project Browser.
* Added a tooltip to the Folder Icon View to show details about the hovered item.
* Added the ability to import scenes from FBX files (meshes, lights, cameras, textures and materials).
* Fixed an issue that caused erratic resources being shown in the Project Brower's folder view.
* Fixed an issue that caused warnings to be printed on the console when a Sound Source was deleted from a level.
* Reduced clutter in the Project Browser by hiding all files with importable extensions.
* Fixed missing/wrong previews and thumbnails in some circumnstances.
* Improved unit previews and thumbnails with better lighting/dimensionality.
* Fixed Project Browser showing stale files in some circumnstances.
* Items in the Level Tree can now be sorted by name or type.
* Fixed an issue in the Project Browser that caused a new Unit to be always created even if the user cancelled the "New Unit..." action.
* Fixed an issue that caused the Level Editor to launch a game with outdated state in some circumnstances.
* Fixed an issue with some core objects not being hidden when spawned in-game.
* Fixed an issue that caused the Project Browser to show deleted content sometimes.
* Fixed several crashes when parsing or writing invalid JSON data.
* The New Project panel will now offer to create the project folder automatically.

**Runtime**

* Lua API: 3D Gui will now place objects on the new XY plane (on the "floor") by default.
* Lua API: added ``SceneGraph.owner()``, ``SceneGraph.first_child()`` and ``SceneGraph.next_sibling()``.
* Data Compiler: .mesh resource can now have shared geometries between nodes.
* Data Compiler: .unit resources have now the ability to add/remove inherited children or to override them by adding, removing or modifying their components.
* Data Compiler: the data compiler will now print an error message instead of crashing when parsing malformed SJSON files.
* Data Compiler: fixed and issue that caused some resources to be always marked as outdated in some circumnstances.
* Fixed destroying units with a script component.
* Added customizable gravity vector in global.physics_config resource.

0.53.0 --- 30 Nov 2024
----------------------

**Tools**

* The Sprite Importer now has dedicated "preview" and "slices" tabs, which can now be zoomed. Plus, it received improvements in settings validation and placement and clearer collider outline rendering.
* The Font Importer can now zoom the generated atlas.
* Snap-to-grid when placing objects in the level is now disabled by default.
* Fixed some dialogs retaining old state when switching between projects.
* Fixed the Project Browser not updating when adding/removing files.
* Fixed the Camera Compass and Gizmos not updating immediately in some circumnstances.
* Fixed opening core folders from Favorites.
* Resource importers will now generate textures with appropriate default settings.
* Added thumbnails to the Project Browser's tree view.

**Runtime**

* Fixed an issue that caused the Runtime to stop sending console messages after a while.
* Fixed texture reloading crashing in some circumnstances.
* Lua API: Added ``World.create_world_gui()`` and 3D variants for all Gui functions.
* Data Compiler: render states in .shader files can now have properties set based on conditional expressions evaluated at compile-time.
* Data Compiler: fixed erratic texture compilation when both legacy properties and the new "output" object were specified in the .texture resource.

0.52.0 --- 11 Nov 2024
----------------------

**Tools**

* Added the ability to add/remove components from units in the Inspector panel.
* Fixed switching between orthographic and perspective camera modes.
* The Editor will now save and restore the camera state per Level.
* Improved compatibility with Hi-DPI screens.
* The Level Tree will now scroll to the last selected object.
* Added a simple camera compass to the Editor View.

**Runtime**

* Animation state machines can now contain empty animation sets.
* Fixed loading resources from bundles.
* Added texture hot-reloading.
* Data Compiler: fixed bundle generation.
* Lua API: Gui drawing primitives can now specify an optional depth value for sorting.
* Lua API: ``World.camera_screen_to_world()`` and ``World.camera_world_to_screen()`` now use a bottom-left screen-space origin to match the coordinate space used by the Gui subsystem. Additionally, ``World.camera_world_to_screen()`` will now return the point's distance from the camera in world-space.

0.51.0 --- 28 Oct 2024
----------------------

**Tools**

* The Editor's main window state (size, maximized, fullscreen etc.) will now be restored across sessions.
* The Inspector is now able to fully edit all the properties of all the components in a Unit.
* Added the ability to add resources to a Favorites list in the Project Browser.

**Runtime**

* Added the ability to set a new (smoothed) timestep policy.
* Improved ``graph`` command with the ability to add multiple fields, customize colors and limit the number of samples shown.
* Lua API: Added ``Device.set_timestep_policy()`` and ``Device.set_timestep_smoothing()``.
* Lua API: Added ``RenderWorld.mesh_set_geometry()``.
* Windows: fixed xinput.dll not found on some systems.
* Windows: fixed console output and absolute paths when launched under MinGW.

0.50.0 --- 10 Oct 2024
----------------------

**Tools**

* Added an option to use the debug keystore when deploying APKs for Android.
* Added the ability to copy the path of files in the Project Browser.
* Fixed unit preview in the Resource Chooser.
* The Console will now show a single line with a counter instead of spamming the view with duplicated entries.
* Fixed Console's text color in dark/light mode.
* Fixed mouse click in the Editor View not selecting the correct sprite in some circumnstances.
* Added the ability to drag & drop units from the ProjectBrowser to the EditorView.
* Added thumbnails for .unit, .material, .texture and .sound resources in the Project Browser.
* Fixed importing resources in the source directory's root asking for destination directory.
* Improved revealing a resource in the Project Browser.
* Fixed some dialogs not getting focus when opened after the first time.

**Runtime**

* Fixed intra-frame button press/release detection.
* Added ``--hidden`` CLI option.
* Fixed HashMap and HashSet.
* Packages will now bring resources online in the correct order. This enables runtime optimizations and features previously impossible to have.
* Windows: fixed resolution property from boot.config not being honored.
* Data Compiler: Added per-platform texture output settings.
* Data Compiler: Fixed existence/redefinition checks for samplers.
* Data Compiler: Added the ability to inherit render states via the ``inherit`` property.
* Data Compiler: Windows: Fixed shader compilation.
* Data Compiler: Improved data writing robustness.
* Lua API: Added ``Device.screenshot()`` and ``screenshot()`` callback, see :doc:`lua_api` for details.
* Lua API: Added ``Material.set_texture()``.
* Lua API: Added ``Gui.material()``.

0.49.0 --- 27 Nov 2023
----------------------

**Data Compiler**

* Linux: fixed detection of deleted directories in some cases.

**Runtime**

* Fixed a crash when rendering text with a font missing some of the glyphs.
* Fixed pixelation when rendering scaled text.
* Fixed .sprite_animation's compiler and resources.

**Tools**

* Added a TrueType and OpenType Font Importer.
* Added the ability to rename a sprite in the Sprite Importer.
* Added a popup menu to quickly access some resource-related functionalities directly from the Console.
* Fixed initial 'sensitivity' state in some widgets.
* Fixed erratic messages when importing assets and improved import procedure robustness.
* Fixed creating new project from templates.
* The most recent project in the Projects List can now be opened by pressing the 'Enter' key.

0.48.0 --- *31 Jul 2023*
------------------------

**Data Compiler**

* Data directories can now be deleted at run-time to force a full data compilation.
* Some dependencies for Lua scripts are now automatically determined by parsing require() calls in the source.
* Add ability to pack compiled data together into "bundles".
* Windows: fixed an issue that prevented the data-compiler to be launched in stand-alone mode when the data-compiler server was running.

**Runtime**

* Added experimental HTML5 target platform.
* Added ``--window-rect``, ``--bundle`` and ``--bundle-dir`` CLI options.
* Fixed a double-free error during shutdown.
* Bumped minimum OpenGL|ES version for Android platform to 3.0.

**Tools**

* Added Deploy dialog to generate packages for all supported platforms.
* Added camera framing of selected objects or whole Level.
* Numeric input fields will now accept simple math expressions.
* Fixed the Editor View struggling to grab keyboard focus sometimes.
* Fixed an issue that caused a project folder to be deleted when the ESC key was pressed in the confirmation dialog.
* Fixed the Editor View's size when Hi-DPI is enabled.
* Fixed minor aesthetic issues.

**Lua API**

* Added ``Matrix4x4.scale()`` and ``Matrix4x4.set_scale()``.
* Added ``Math.obb_vertices()``.
* Fixed ``Touch.axis()`` value not being updated at the start or end of a touch action.

0.47.0 --- *06 Feb 2023*
------------------------

**Data Compiler**

* Fixed file changes not detected sometimes.
* Fixed Ctrl+C/SIGTERM/SIGINT not being honored when launched with --server.
* Fixed handling of filenames containing some special characters.
* Fixed a crash when a directory was created and deleted immediately after in a project folder.

**Runtime**

* Windows: reduced CPU usage by polling joypads' status in a background thread.
* Added ``graph`` command to plot profiler data at runtime.
* The ConsoleServer will now report an error when a command is not found.
* Fixed a crash when reloading materials.
* Hot-reloading has been enabled for all resource types.
* Fixed a crash when malformed lua scripts were passed to boot.config or require()-ed from other lua scripts.

**Tools**

* The tools are now licensed under the GNU GPL v3.0 or later.
* Bumped minimum GTK+ version to 3.22 (Ubuntu 18.04+).
* Added the ability to set a limit to the memory used by the undo/redo system.
* The editor now uses an external service to launch subprocesses and clean them up effectively after crashes.
* Fixed crashes in the undo/redo system.
* Fixed minor issues when toggling the Console.
* Fixed the Project Browser not being able to obtain keyboard focus.
* Fixed camera view not being restored.
* Fixed minor aesthetic issues.
* Fixed Console's text not scrolling to bottom.

**Lua API**

* Added ``Input.events()``, see :doc:`lua_api` for details.

**Samples**

* Added 03-joypad sample.

0.46.0 --- *19 Nov 2021*
------------------------

**General**

* Updated various external web links to docs, website etc.
* Windows: fixed handling of spaces in filenames when spawning external processes.

**Tools**

* The Console will now use local time for log output.

0.45.0 --- *30 Jul 2021*
------------------------

**Data Compiler**

* Fixed standalone compiler never exiting when another compiler instance was running in server mode.

**Tools**

* Fixed opening projects from the menubar when in the welcome panel.
* Fixed projects opened from the menubar not being added to the recent projects list.
* Fixed deploy getting stuck.
* Linux: fixed launching editor under Wayland.

0.44.0 --- *13 May 2021*
------------------------

**Data Compiler**

* Fixed undetected file changes in some circumnstances.

**Tools**

* Fixed Engine View not redrawing when a command was sent from the Console.
* Various fixes and improvements to the Console.
* The Editor will now check whether the file being edited is deleted from the Project Browser to ask user for confirmation.
* Added the ability to set in the Preferences the external editors to use when opening Lua and image files. (Currently only available on Linux.)
* Custom theme improvements.
* Fixed duplicated entries in the Resource Chooser.
* Changing the sprite in the Sprite Renderer component is now reflected to the Runtime.
* Added noop resources in ``core/components/noop.*``.
* Changed the fallback shader to output Color4(255, 0, 255, 255).
* Added the ability to spawn empty units.
* Added the ability to choose between "Light" and "Dark" theme for the editor UI.

**Lua API**

* Added ``World.camera_destroy()``.

0.43.0 --- *17 Apr 2021*
------------------------

**Data Compiler**

* Windows: fixed garbage data written past EOF in some circumnstances.
* Fixed uniform data compilation in materials.

**Runtime**

* Added --pumped mode to skip rendering of frames unless explicitly requested.
* Fixed the creation of uniforms with ``matrix4x4`` type.
* Fixed crashes when loading shaders in some circumnstances.

**Tools**

* Windows: fixed wrong Editor View window size.
* Added a setting to limit the number of lines displayed by the Console.
* Added hyperlinks to resource paths in the Console.
* Selection is now correctly restored after Editor View restarts.
* Fix Editor window title showing incorrect level-changed state.
* Reduced CPU & GPU usage by launching Editor Views with --pumped runtime.
* Added multi-selection support.
* Improved the rendering of the outlines of selected objects.

**Lua API**

* Added ``Math.obb_intersects_frustum()``
* Removed ``DebugLine.add_unit()``
* Fixed ``World.camera_screen_to_world()`` returning incorrect z-axis values on Windows/D3D.
* ``print()`` will now try to detect the type of the lightuserdata and print it accordingly.

0.42.0 --- *05 Mar 2021*
------------------------

**Editor**

* Improved the visibility and picking of the handles of every gizmo.
* Added camera-plane translation to the Move tool.
* Added camera-plane rotation to the Rotate tool.
* Added axis-, plane- and uniform-scaling to the Scale tool.
* Fixed an issue that prevented the Editor View to be restarted in some circumnstances.
* Fixed the translation of multiple unaligned objects when snapping was enabled.
* Added new Crown logo.
* Added icons to the Level Tree View.
* Updated the meshes for Sound, Camera and Light units.
* Added a button to the Resource Chooser to "reveal" the selected resource in the Project Browser.
* Fixed placement of objects when snap-to-grid was enabled.

**Runtime**

* Upgraded to LuaJIT 2.1.
* Added support to 64-bits Android (ARMv8-a).
* Fixed changing Mesh and Sprite visibility.

0.41.0 --- *16 Jan 2021*
------------------------

**Manual**

* Improved the Introduction and added Features section
* Added Glossary
* Added license statement about "Your Game or Application"

**Data Compiler**

* Fixed compilation of collider volumes

**Runtime**

* Fixed loading of collider volumes

**Tools**

* The Editor View now will use the actual unit being placed as a preview instead of its wireframe
* Fixed an issue that caused textures with supported extension types to be skipped by the importer
* Added the ability select distinct resource types in the Import dialog
* Fixed Lua error when setting camera parameters
* The Project Browser will now show every file type except those used only internally by the Editor
* Removed the default "FPSCamera" camera from the core game framework

0.40.0 --- *06 Jan 2021*
------------------------

**Data Compiler**

* Fixed an issue that caused resources to not be compiled with the proper version in rare circumnstances
* Fixed an issue that caused the compiler to crash when reading empty source files
* Fixed an issue that prevented the output from external data compilers to not be read under Windows

**Runtime**

* Fixed child nodes in the SceneGraph not being marked as changed when their parent was changed
* Removed support for multiple components per Unit.
* Added ability to express unit's parent-child relationship from within .unit and .level files

**Exporters**

* Added the ability to export full scene hierarchy to the Blender exporter
* Removed support for Blender < 2.80

**Tools**

* Added logs expiration option to Preferences dialog
* Added the ability to select from a number of templates (samples) when creating new projects
* Changed the default accelerator for deleting objects from Ctrl+K to Delete
* Fixed an issue that caused the Level Editor to ask multiple times whether save the level in some circumnstances
* Level Editor now restores the Console's history from previous sessions
* Improved the title of the Level Editor window to include the name of the current opened level and an indication of whether it has been modified since the last save to disk

**Lua API**

* All component managers accessors have been uniformed to accept a component instance ID (instead of a UnitId or both):
	- All ``AnimationStateMachine.*``, except ``AnimationStateMachine.create()``
	- All ``RenderWorld.light_*``, except ``RenderWorld.light_create()``
	- All ``RenderWorld.sprite_*``, except ``RenderWorld.sprite_create()``
	- All ``SceneGraph.*``, except ``SceneGraph.create()``
	- All ``World.camera_*``, except ``World.camera_create()``
	- ``PhysicsWorld.actor_instances()``
* Added AnimationStateMachine.instance()
* ``RenderWorld.set_mesh_visible()`` will now work as expected
* Changed ``SceneGraph.link()`` behavior and added parameters to explicitly set the position, rotation and scale of the child transform after linking is done
* Fixed ``Matrix4x4.rotation()`` to return the correct Quaternion even when the matrix has scaling applied
* Fixed ``SceneGraph.*_rotation()`` to return the correct Quaternion even when the node has scaling applied
* Fixed ``SceneGraph.unlink()`` to correctly set the local pose of the unlinked transform to its previous world pose
* Fixed ``SceneGraph.destroy()`` to correctly update any linked transform before deleting the node

0.39.0 --- *24 Oct 2020*
------------------------

**Data Compiler**

* Fixed detection on new sub-directories and sub-directory renames
* Fixed handling of paths without type extension
* Improved file deletion detection and data directory coherence
* Improved file modification detection and source index caching
* Improved the unit compiler by fixing a number of bugs and adding support to "deleted_components"

**Runtime**

* Removed support for 32-bit x86 architectures

**Tools**

* Added ability to create new scripts from Project Browser
* Added ability to create new units from Project Browser
* Added Debug > Build Data
* Added QWER accelerators to place, move, rotate and scale object actions
* Added support for Windows
* Added the ability to duplicate objects from Level Tree
* Added Welcome panel with a list of recent projects and the ability to create new projects or import existing ones
* Bumped minimum GTK+ version to 3.16 (Ubuntu 16.04.2+)
* Fix an issue that caused the Editor View camera not returning to idle in some circumnstances
* Fixed a number of dialog boxes that were not centered to the Level Editor's main window
* Fixed an issue that allowed the user to enter blank names when creating new folders in the Project Browser
* Fixed an issue that caused Project Browser to show inconsistent folder structured in some circumnstances
* Fixed an issue that caused the camera view accelerators to interfere with text input
* Fixed an issue that prevented some components from being removed when reimporting sprites
* Fixed and issue that caused generation of spurious "set-actions" when editing properties in the Properties panel
* Fixed modifier keys getting stuck in the wrong state in some circumnstances
* Fixed multiple selection in Level Tree
* Fixed undo/redo when setting properties of some unit components
* Improved Project Browser to not show irrelevant items
* Improved Test Level/Start Game button behavior when game failed to launch
* Level Editor connection to the Data Compiler, Editor View(s) and Game is now faster and more robust
* Level Editor now saves aggregate logs to disk. User can browse logs folder from Help > Browse Logs...
* New Project dialog no longer allows selecting non-empty folders for new projects
* Objects inside .level files are now ordered by their ID before serialization
* Save Level dialog now warns before overwriting a file that already exists
* Unified Engine and Run menubar items into a single Debug menubar item

**Lua API**

* Added Matrix4x4.equal()

0.38.0 --- *24 Aug 2020*
------------------------

**Runtime**

* Added "help" command
* Core primitives now include UV, tangent and bitangent data
* Fixed a crash when multiple clients were connected to the Console Server
* Fixed a crash when reloading lua scripts that haden't been loaded previously
* Fixed an issue that caused levels to be compiled successfully even when the units they depended on contained errors
* Fixed reloading of main.lua files from samples
* The Data Compiler now detects when files are deleted

**Tools**

* Added Gizmo size and Autosave timer options to Preferences dialog
* Added the ability to toggle visibility of the Inspector inside the Level Editor
* Added the Project Browser
* Added the Statusbar
* Fixed an issue that allowed the Level Editor to load or save levels outside the source directory
* Fixed an issue that allowed the user to enter blank names when renaming objects in the Level Tree
* Fixed an issue that caused level auto-saving in Level Editor not triggering at the intended interval
* Fixed an issue that caused the Level Editor to not include "core/units/camera" in the boot.package of a newly created project
* Fixed an issue that caused the Level Editor to start the Editor View before data compilation was done in some circumstances
* Fixed main.lua files generated by Level Editor for new projects
* Improved look of EntryVector3 widget
* Lua reloading has been extended to the running game when pressing F5 from the Level Editor
* Nodes in the Level Tree can now be expanded by clicking on the corresponding row
* Preferences are now saved to the user's config directory
* Renaming of objects in the Level Tree is now handled with a modal dialog
* The Editor View will now show a message explaining how to recover the session after a crash or unintended disconnection
* Unified the asset import dialogs

**Samples**

* Unified projects directory structure

0.37.0 --- *26 Jun 2020*
------------------------

**Runtime**

* Added Material.set_vector4() and Material.set_matrix4x4()
* Added PhysicsWorld.actor_destroy()
* Added RenderWorld.mesh_material(), RenderWorld.mesh_set_material() and RenderWorld.sprite_material()
* Added the ability to hot-reload Lua files
* Added the ability to scale the shape of colliders at Unit spawn time
* Added Window.set_cursor_mode()
* Added World.unit_by_name() to retrieve unit by its name in the Level Editor
* Bumped minimum Android version to 7.0+
* Bumped minimum OpenGL version to 3.2+ for Linux
* Fixed an issue that caused PhysicsWorld.set_gravity() to re-enable gravity to actors that previously disabled it with PhysicsWorld.actor_disable_gravity()
* Fixed an issue that prevented kinematic actors to be controlled via the SceneGraph
* Fixed an issue that prevented PhysicsWorld.actor_center_of_mass() to be called for static actors
* Fixed an issue that prevented PhysicsWorld.actor_world_{position,rotation,pose}() to be called for static actors
* Fixed an issue that reset the sprite animation to the beginning even when loop was set to false
* Fixed an issue where a regular Matrix4x4 was returned if Matrix4x4Box is called without arguments
* Removed "io" and "os" libraries from Lua API
* Small fixes and performance improvements
* Sprite's frame number now wraps if it is greater than the total number of frames in the sprite

**Tools**

* Added the ability to specify a circle collider in the Sprite Importer
* Added the ability to specify the actor class in the Sprite Importer
* Added the ability to specify the destination of the console commands between Game and Editor
* Fixed a crash when entering empty commands in the console
* Fixed an issue that caused the Level Editor to not correctly save a level specified from command line
* Fixed an issue that could cause the Level Editor to crash when large number of TCP/IP packets were sent to it
* Fixed an issue that could cause the Level Editor to crash when scrolling through the console history
* Fixed an issue that could cause the Level Editor to incorrectly parse identifiers in SJSON files
* Fixed an issue that generated wrong render states when blending is enabled while no blend function/equation is specified
* Fixed an issue that prevented some operations in the Level Editor from being (un/re)done
* Fixed an issue that prevented the data compiler from restoring and saving its state when launched by the Level Editor
* Improved the numeric entry widget which now takes less space and provides more convenient input workflows
* Resources autoload is disabled when testing levels from Level Editor
* The Data Compiler will now track data "requirements" and automatically include them in packages when it's needed
* The game will now be started or stopped according to its running state when launched from the Level Editor
* The Properties Panel now accepts more sensible numeric ranges
* The Properties Panel now allows the user to modify most Unit's component properties

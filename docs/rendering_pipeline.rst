Data-driven rendering pipeline
==============================

Target and scope
----------------

The initial implementation targeted olracrafter/crown at
``e567cd09a43e6c59fdaeb2d9641ccb6821a08437``. The layer-removal and configuration
selection corrections apply on top of ``3da8fce437fcb16d11017e5a6954f73b1fd021c4``. It replaces the hard-coded
resource allocation and view-configuration loops with compiled render_config
data, and re-expresses the existing post-processing chain as modifier arrays.
It does not introduce a new shading technique or change units into resources.

The architectural references are Tobias Persson's *Benefits of a data-driven
renderer* (GDC 2011, pages 31-42) and *Flexible Rendering for Multiple Platforms*
(GDC 2012, pages 18-22). Their terms are retained: a **layer** is a logical
ordering group; a bgfx **View** is the backend implementation, not a shader pass.
A resource generator is an ordered array of modifiers. Native callbacks are
valid modifiers, not special cases in the generic executor.

This is an adaptation to Crown's current, single application-owned Pipeline.
There is one active executor owning views starting at zero. The separate
viewport/local-resource-set model from the slides, multiple simultaneous
viewports, asynchronous batch gathering, a graph editor, compute/buffer resource
types, a deferred renderer, loop modifiers and automatic dependency scheduling
are not implemented by this patch. Do not create two active RenderPipeline
instances sharing the same bgfx context: their view ranges would overlap.

Files and boundaries
--------------------

``src/resource/render_config_resource.h`` and ``render_config_resource.inl``
    Immutable, offset-based binary descriptions and const accessors. No bgfx
    handles, pointers into temporary compiler memory or callback pointers are
    serialized. The render_config resource version changes from 9 to 10.

``src/resource/render_config_resource.cpp``
    PipelineCompiler is private to this translation unit. It parses, validates
    and links names to indices before writing the binary tables. Nontrivial
    runtime resource helpers also live here. The shared .inl contains only
    short const accessors and predicates, not the compiler implementation.

``src/device/render_pipeline.h/.cpp``
    Technique-independent resource owner and layer/modifier executor. It knows
    texture formats, targets, sorting, conditions and a fullscreen primitive.
    It does not know bloom, shadows, forward shading or deferred shading.

``src/device/pipeline.h/.cpp``
    Adapter for Crown's current scene producers and native scene parameters.
    Shadow math, light packing and cookie-atlas packing remain native. Bloom,
    vignette, tonemapping and outline callbacks supply existing uniform values
    and invoke the generic fullscreen primitive. Their shader, inputs, outputs
    and position in the frame come from data.

``samples/core/renderer/default.render_config``
    The current renderer expressed as 13 resource declarations, 21 logical
    layers and 20 modifiers. It reserves 141 backend views, including inactive
    conditional branches. Bloom and vignette each have mutually exclusive
    effect/bypass paths.

``src/world/render_world.cpp``, material, GUI, debug-line and device call sites
    Resolve logical names rather than relying on the removed View enum. View
    arguments are 16-bit. GUI and DebugLine keep a Pipeline pointer, not a
    cached numeric layer ID. Materials still receive their destination when
    bound; they do not own a view or mutate their compiled material resource.

Configuration
-------------

An inline layout declares ``layers``. ``global_resources`` and
``resource_generators`` are optional; all three arrays may be empty. A
settings-only resource can instead select another complete configuration as
its layout::

    pipeline = "core/renderer/default"
    render_settings = {
        // Normal existing render settings belong here.
    }

A settings-only configuration uses that default layout automatically. The
external file is read through CompileOptions, so it is tracked as a compiler
dependency; its shader libraries are requirements too. Only the selected
layout is inherited, not its render_settings. There is no recursive inheritance
or array merging. Partial inline layouts are errors. An external layout must
itself contain all three layout sections.

Resources
~~~~~~~~~

``global_resources`` is an array of named declarations. The default type is
``render_target``; ``texture`` creates an ordinary sampled texture. A declaration
requires ``name`` and ``format``. Supported formats are RGBA8, BGRA8, RGBA16F,
RGBA32F, R32U, D16, D24, D24S8 and D32F.

Specify ``width`` and ``height`` together, or a named ``size``. Omitted size
means ``backbuffer``. The Crown adapter supplies ``sun_shadow_map_size``,
``local_lights_shadow_map_size`` and ``lights_cookie_atlas_size`` from the
requested render settings. Optional positive ``w_scale`` and ``h_scale`` apply
to that source. Resizing does not compound scaling of these external sizes.
Dimensions are rounded down and clamped to at least one texel, then constrained
by the device's texture-size limit.

``count`` (1-16, default 1) creates separate textures, each half the previous
texture's dimensions, again clamped to one. This is not a hardware mipmapped
texture. A reference is either a name or ``{ resource = "bloom" index = 2 }``.
Changing the bloom count also requires updating the explicit modifier array;
there is no hidden C++ loop that reconstructs its data-defined chain.

Flags are ``clamp``, ``point``, ``anisotropic``, ``compare`` and ``msaa``.
Point and anisotropic filtering are mutually exclusive. Comparison sampling
requires a depth format; MSAA requires a render target. MSAA quality comes from
render settings. The existing Linux versus non-Linux depth/MSAA flags are
preserved by the backend allocation adapter; in particular, this patch does
not claim to repair the pre-existing Windows selection/MSAA depth policy.

A single 1x1 RGBA8 ordinary texture may have ``initial_value = "RRGGBBAA"``.
The default cookie fallback uses zero. Initial bytes are emitted in explicit
RGBA order rather than depending on host integer byte order.

Layers and targets
~~~~~~~~~~~~~~~~~~

Layers run in declaration order. A geometry layer has a ``name`` and may have
``render_targets`` (an array), ``depth_stencil_target``, ``sort``, ``transform``,
``clear``, ``touch`` and ``count``. Count reserves consecutive views for an
existing native producer, such as the eight sprite layers or shadow cascades.
The shader route or native producer chooses a view within that range.

An empty color list with no depth target means the swap chain. A depth-only
target is supported. All attachments must agree on dimensions and MSAA quality.
They must be render targets with the appropriate color/depth format; duplicate
attachments are rejected. The backend attachment limit is checked as well.

Sort values: ``default``, ``sequential``, ``front_to_back``, ``back_to_front``.
The mesh producer now supplies a positive camera-distance key, calculated from
the mesh origin, so depth sorting is meaningful rather than sorting every mesh
with key zero. Sprite and GUI depth keys keep their existing meaning. This is
object sorting, not triangle sorting for intersecting transparent geometry.

Transforms: ``none``, ``camera``, ``ortho_unit``, ``ortho_screen``,
``ortho_graph`` and ``ortho_unit_flipped``. ``manual_rect = true`` lets a native
producer supply its own rectangle, as for shadow and cookie atlas tiles.

Clear data can specify booleans ``color``, ``depth``, ``stencil`` and optional
``rgba`` (eight hex digits), ``depth_value`` (0-1), ``stencil_value`` (0-255).
A view clears when it receives work; ``touch = true`` also schedules an empty
view. Geometry defaults to no touch. Fullscreen modifiers default to touch.

``profiling_scope`` supplies the backend view's display/profiling name. It is
not a new hierarchical CPU profiler or the complete performance HUD in the
BitSquid slides.

Generators and modifiers
~~~~~~~~~~~~~~~~~~~~~~~~

A generator has a name and a non-empty ``modifiers`` array. A generator layer
references it using ``resource_generator``. Every modifier receives its own
view in array order. Put outputs and clears on the modifiers, not on the
containing generator layer. Independent sub-views avoid sampling an attachment
while simultaneously writing it in the same draw.

``fullscreen_pass`` requires a shader program name. It supports named texture
inputs with explicit sampler names/stages and optional sampler flags, constant
vec4 ``uniforms``, and ``pixel_size_uniform``. The latter receives
``(1 / output_width, 1 / output_height, 0, 0)``. Input stages are limited to 0-15
and additionally checked against device capabilities. Shader names identify
programs/permutations inside the libraries in ``shaders``; they are not resource
filenames. Shader programs are resolved at execution time to observe reloads.

For example, add a resource, generator and layer to a copy of the default
configuration to capture the SDR result at half resolution::

    // Additional global_resources entry:
    { name = "preview" format = "RGBA8" w_scale = 0.5 h_scale = 0.5 }

    // Additional resource_generators entry:
    {
        name = "preview_copy"
        modifiers = [
            {
                type = "fullscreen_pass"
                shader = "blit"
                render_targets = [ "preview" ]
                inputs = [
                    { texture = "color_sdr" sampler = "s_color_map" stage = 0 }
                ]
            }
        ]
    }

    // Additional layer, after the SDR producers and before presentation:
    { name = "preview_copy" resource_generator = "preview_copy" }

Native modifier functions have this signature::

    void modifier(RenderPipeline &runtime,
                  const RenderModifierData &description,
                  u16 view,
                  void *user_data);

Register a name/function pair in the RenderModifierType array supplied to
RenderPipeline::create. The callback can use description inputs, constants and
an optional named ``resource`` context. Existing callbacks use
``runtime.draw_fullscreen(description, view)`` after setting dynamic scene
uniforms. Adding another callback does not require a technique switch in
RenderPipeline. New low-level resource kinds or new kinds of serialized
parameters would, separately, require extending the schema and compiler.

Conditions and shader routing
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``conditions`` declares at most 32 names. Resources, layers and modifiers can
have ``if`` and ``unless`` arrays. Every positive condition must be true; every
negative condition must be false. A consumer must require the conditions of
each resource it uses. A modifier's effective conditions include its layer's
conditions. Contradictory conditions are rejected.

Resources are allocated at create/reset time. Conditions used by resource
allocation cannot be changed in place; recreate the executor instead. Execution
conditions can change each frame without changing any view ID. In the default
config ``bloom_allocated`` controls allocation, while ``bloom`` controls which
of the effect and bypass paths runs. ``vignette`` is another execution condition.
Selection, cookies and MSAA conditions come from the Crown adapter.

``shader_layers`` optionally routes exact material shader IDs::

    shader_layers = [
        { shader = "my_transparent_shader" layer = "transparent" index = 0 }
    ]

The destination must be a declared geometry layer. Unmapped shaders retain the
producer's default mesh or sprite destination. Shadow and editor-selection
submissions retain explicit native destinations. This does not implement
multi-pass materials or a new shader permutation system.

Native producer contracts
-------------------------

No native layer is mandatory. Removing a geometry layer (or setting its count
to zero) suppresses submissions to that destination. A missing or disabled
name returns UINT16_MAX, never view zero. Native producers do not submit into
resource-generator views. Exact shader_layers routes can still send material
batches to other declared geometry layers when the default mesh/sprite layer
is absent. Dangling explicit routes remain authoring errors.

The existing native algorithms still have named roles: mesh, sprite,
sm_cascade, sm_local, lights_cookie_atlas, lights, selection, world_gui,
screen_gui, debug and graph. GUI/debug destinations are looked up for each
submission. The scene updates execution conditions before gathering native
batches. Reserved sub-view ranges are respected: cookie packing stops when its
configured views are exhausted, spot shadows consume one view, and omni
shadows require four remaining views. The native sun shader still uses four
cascade matrices; fewer declared cascade views omit those tile draws, not a
new arbitrary-cascade shading algorithm.

Resource names are optional too, but a native operation needs compatible
inputs. A material declaring a native lighting sampler is skipped when that
resource is unavailable, rather than binding an invalid texture handle. An
unlit shader that does not declare those samplers can render without the
native lighting resources. The native packed-light layout remains 768x1
RGBA32F, and native shadow atlases must be square and at least 2x2. These are
algorithm requirements, not a requirement to retain the default topology.

An explicitly empty layers array is a valid layout, not a request to inherit
the default pipeline. No scene or modifier draw is submitted. The executor
uses a presentation-only black backbuffer clear so that the previous frame is
not left on screen. Zero-count layers and empty generators are also valid.
Resources still declared in global_resources are allocated as requested.

Clear operations are explicit data. Removing sm_cascade_clear removes that
clear; sm_cascade can still draw into the atlas. This is not equivalent to
turning off shadow rendering or clearing the final color buffer to black.
Reading an uncleared target is the author's responsibility; its contents after
creation/reset are not guaranteed. No replacement clear is inserted for a
removed layer. The black presentation clear above is only for zero-view layouts.

Cookie packing no longer implicitly touches lights_cookie_atlas_clear. In
copies made before this correction, add ``touch = true`` to that clear layer
and to sm_local_clear, as in the updated default.render_config. A clear-only
view needs a touch (or a draw) to execute its declared clear.

Selecting and reloading a project configuration
-----------------------------------------------

A copied file is not automatically the active configuration. For a file named
``default.render_config`` at the project root, set this in the project's
``boot.config`` (the resource name has no extension)::

    render_config = "default"

For ``renderer/my_pipeline.render_config``, use ``renderer/my_pipeline``.
Do not confuse boot.config's ``render_config`` (which resource to load) with a
render_config resource's optional ``pipeline`` (where its layout comes from).
Do not add pipeline="core/renderer/default" alongside an inline layers array.

The runtime also accepts ``--render-config <resource>``. It overrides only the
selected render_config name, before loading its package; it does not replace
the boot script, window configuration, or the boot render_settings overrides.
EditorViewport reads the project root boot.config and passes its selected
render_config through this option. The editor keeps its own tool boot script
and selection setting, but no longer silently previews a different default
layout. The thumbnail service retains its separate configuration.

After rebuilding both the engine and editor, compile the project data and
restart the editor viewport once. After that, editing the selected
render_config and using compile-and-reload/Reload All recreates the active
pipeline, resets old bgfx views, and requests another frame in pumped mode.
Changing the resource name in boot.config itself requires compiling and
restarting the game/editor viewport; boot settings are not hot-switched.
Opening another level does not select a render_config, and Build Data without
a client refresh does not apply an already-loaded resource to that client.

The runtime logs the active resource's 64-bit ID and its layer/resource counts
on startup and successful reload. A compiler error means the changed source
was compiled, not necessarily that the running client selected it. Failed
compilation cannot replace the currently loaded pipeline with that source.

Ownership, reload and validation
--------------------------------

RenderPipeline is the sole owner of all declared textures. Its framebuffers
borrow attachments (destroyTextures=false). Destruction resets old views,
destroys framebuffers before textures, balances uniform references and clears
runtime lookup tables. No compiled resource data is modified. A layout reload
can renumber views; long-lived GUI/debug consumers resolve names at submission.
An invalid/disabled numeric view is UINT16_MAX and must not reach bgfx::submit.

The compiler rejects unknown properties and enum values, duplicate named
objects, invalid counts/scales, dangling names, bad attachment types, feedback
loops, duplicate texture stages/samplers and incompatible conditional resource
use. bgfx's dynamic device limits and framebuffer compatibility are checked at
runtime with diagnostics that remain enabled in release builds. Unsupported
runtime configurations fail instead of silently falling back to a different
format. This is not an automatic graph scheduler: authors must order producers
before consumers and initialize resources before reading them.

Applying and testing
--------------------

Apply the corrective patch on top of commit 3da8fce437fcb16d11017e5a6954f73b1fd021c4
without reapplying the initial implementation patch. Rebuild the engine and
the editor, and compile the project data. This correction does not change the
binary schema (version 10); old version-9 resources remain incompatible.

The isolated test command is::

    python3 tests/render_pipeline/run.py --sanitize

It compiles the production pipeline compiler helper, generic executor and
native adapter against an explicitly substituted recording SDK. It checks 64
feature/scene combinations, 64 resize cycles including 1x1 and 3x5 targets,
conditional bypasses, layout reloads, view IDs above 255, ownership, immutable
compiled data, scaled external sizes, transient-buffer failure, capability
checks and 21 invalid configurations. Regression cases also cover empty
layouts, zero-count layers, empty generators, all 21 individual default-layer
removals, persistent clear-state reuse, changed clear values, reduced cookie
view budgets, optional native inputs and execution-condition toggles. The SDK replaces the SJSON frontend,
containers, hash function, shader manager, bgfx and rectangle packer. These tests
do not establish compatibility with the real Crown/bgfx headers or render
pixels. They do not execute the full RenderWorld, resource package loader,
Device refresh path or Vala editor launcher.

The patch was mechanically apply-checked against the locally reconstructed
source used for development, with complete original pipeline files checked
against their Git blob hashes. A complete pristine repository checkout and
full engine build were not available in the development environment. Treat the
following as required integration acceptance checks, not as already passed:

* ``git apply --check`` on your complete checkout at the exact target commit;
  full debug/development and release builds, followed by a clean data compile.
* Empty-layout acceptance: select the project resource, set layers=[], compile
  and reload, and confirm a black viewport and no scene submissions. Restore
  the layers, reload again, and confirm the scene returns without restarting.
* Baseline image comparison for meshes, skydome, sprites, world/screen GUI,
  shadows, cookies and editor selection, with bloom/vignette independently on
  and off. Reordering layers and adding the preview copy must change execution
  without rebuilding engine code.
* MSAA on/off on actual supported GPU backends, especially Windows selection
  depth behavior; resize/minimize/restore and config/shader reload with existing
  GUI, debug-line and graph objects.
* Confirm real shader-library dependencies and settings-only external-layout
  compilation/recompilation through the full data compiler. These paths are
  not exercised by the isolated compiler-helper tests.

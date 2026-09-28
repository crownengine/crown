Data-driven rendering pipeline
==============================

Target and scope
----------------

This implementation targets olracrafter/crown at
``e567cd09a43e6c59fdaeb2d9641ccb6821a08437``. It replaces the hard-coded
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

``src/resource/render_config_pipeline.inl``
    PipelineCompiler, included by the existing resource compiler. It parses,
    validates and links names to indices before writing the binary tables.

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

A configuration may contain an entire layout (all three of
``global_resources``, ``layers`` and ``resource_generators``), or select another
complete configuration as its layout::

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

The generic executor permits arbitrary names and layouts, but Crown's existing
scene producers have an explicit adapter contract. A configuration used by
Pipeline must retain the mesh, sprite, sm_cascade, sm_cascade_clear, sm_local,
sm_local_clear, lights_cookie_atlas, lights_cookie_atlas_clear, lights, selection,
world_gui, screen_gui, debug and graph geometry layers with the required view
counts. These native producer layers cannot use arbitrary dynamic conditions:
only the existing selection/cookie gates are supported for their producers.
Custom geometry layers used through shader_layers can be conditional.

The adapter also resolves existing resource names such as color0, color1,
depth, selection_color/depth, outline_color, sun_shadow_map,
local_lights_shadow_map, lights_data, lights_cookie_atlas and its fallback.
Packed light data remains 768x1 RGBA32F; native shadow atlases must be square.
The color0_clear, color1_clear, sprite, selection, outline and atlas-clear
layers expose existing framebuffer aliases to current engine code. These are
borrowed handles, not additional owners. Changing the native producer contract
is an adapter change; it is not a change to the generic executor.

Thus this patch does not claim that deleting all 3D layers from the default
file alone creates an entirely different renderer. It provides the generic
execution boundary and the current Crown adapter, not every possible producer.

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

Apply the single patch to a clean checkout of the target commit, regenerate
build projects so the new src/device/render_pipeline.cpp is included, and
rebuild the engine and compiled data. Existing cached version-9 render_config
binaries are not compatible. The existing scripts/crown.lua source glob
includes the new .cpp when projects are regenerated.

The isolated test command is::

    python3 tests/render_pipeline/run.py --sanitize

It compiles the production pipeline compiler helper, generic executor and
native adapter against an explicitly substituted recording SDK. It checks 64
feature/scene combinations, 64 resize cycles including 1x1 and 3x5 targets,
conditional bypasses, layout reloads, view IDs above 255, ownership, immutable
compiled data, scaled external sizes, transient-buffer failure, capability
checks and 21 invalid configurations. The SDK replaces the SJSON frontend,
containers, hash function, shader manager, bgfx and rectangle packer. These tests
do not establish compatibility with the real Crown/bgfx headers or render pixels.

The patch was mechanically apply-checked against the locally reconstructed
source used for development, with complete original pipeline files checked
against their Git blob hashes. A complete pristine repository checkout and
full engine build were not available in the development environment. Treat the
following as required integration acceptance checks, not as already passed:

* ``git apply --check`` on your complete checkout at the exact target commit;
  full debug/development and release builds, followed by a clean data compile.
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

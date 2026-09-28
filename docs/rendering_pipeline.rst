Data-driven rendering pipeline
==============================

Scope and migration
-------------------

The prepared-batch change applies to olracrafter/crown at
``8ca1ad52c06473527671c1a2f2a5523a9d4d2051``. It makes mesh, sprite,
selection, shadow-caster, cookie and stencil submissions explicit consumers of
prepared frame data. It retains the existing rendering algorithms, rather than
introducing another graphics backend or a different lighting technique.

Rebuild the runtime/data compiler and regenerate build projects so that
``src/device/render_frame.cpp`` is compiled. The render_config binary version
changes from 10 to 11; recompile source data. Material and shader binary formats
are unchanged. Restart running editor/game backends after rebuilding.

A project copy of the previous default.render_config must be migrated too:
copy the new ``draw`` declarations and geometry input bindings from
``samples/core/renderer/default.render_config``, retaining the project's chosen
layer names, targets and order. A familiar name such as ``mesh`` no longer
implicitly requests geometry. A layer without ``draw`` or a generator performs
only its configured view/clear operations. This is intentional; there is no
legacy name-based fallback.

Continue selecting the project configuration using ``render_config`` in
boot.config. The existing editor selection and refresh paths are unchanged.
A settings-only config still inherits the current default layout. An inline
``layers = []`` is an explicit empty layout, not an inheritance request.

Architecture
------------

The architectural references are Tobias Persson's *Benefits of a data-driven
renderer* (GDC 2011, layers/resource generators) and *Flexible Rendering for
Multiple Platforms* (GDC 2012, batch gathering and callback modifiers). This is
an adaptation to Crown/bgfx, not an implementation of BitSquid's complete
render-command stream, job system or viewport-local resource sets.

The call sequence is::

    Device::render(world, camera)
        World::render(...)
            RenderWorld::prepare(...)
                synchronize bounds, cull, choose LODs
                prepare native lighting/shadow/cookie data
                gather reusable geometry and per-view batch ranges
            existing immediate debug overlays
        Pipeline::render(..., prepared_frame)
            RenderPipeline walks configured layers
                draw_visible: consume a prepared source
                upload: copy a prepared payload into its configured texture
                resource_generator: execute existing modifiers
                external: destination for an immediate overlay stream

``RenderWorld::prepare`` does not submit scene geometry. It still uses bgfx's
frame-local transform/transient-buffer allocation APIs when gathering geometry;
preparation is not a graphics-API-free or multithreaded stage.

``RenderFrame`` owns batch ranges, copied shadow cameras/rectangles, copied
uniform values and upload bytes. Mesh batches snapshot geometry handles and
material-slot index ranges. Sprite batches share the geometry written once
during preparation. Skinning transforms are cached per mesh for the prepared
world/camera invocation, and LOD selection is not advanced when another layer
consumes the same source. Selection identity remains the owning LOD group's unit
ID when that differs from its selected mesh's unit ID.

Shadow sources are gathered from the native shadow culling results, not from
the main camera's visible set. Each shadow view copies its batch range before
the next cull overwrites the scratch result. Off-camera shadow casters are not
lost by deriving a shadow list from the camera list.

Material objects and GPU resources are borrowed, not cloned. Materials must
remain alive and unchanged between preparation and execution. Transform-cache
IDs and transient geometry are valid only within the same bgfx frame. Clear
and rebuild the prepared frame before reuse after a resource/layout reload.
The current Device path prepares and executes synchronously for each call.

Files
~~~~~

``src/resource/render_config_resource.h/.cpp``
    Immutable offset-based descriptions, compilation, reference/condition
    validation. The shared .inl contains only small accessors.

``src/device/render_pipeline.h/.cpp``
    Owns textures, borrowed-attachment framebuffers, view ranges and uniform
    handles. Applies ordering, targets, conditions and shared input bindings.
    Dispatches registered frame drawing operations and existing modifiers.

``src/device/render_frame.h/.cpp``
    Prepared batches and generic draw/upload consumers. Does not know Crown's
    light selection, caster-culling or atlas-packing algorithms.

``src/world/render_world.h/.cpp``
    Scene preparation, culling, LOD selection and geometry gathering. No
    specially named destination layer is chosen by a mesh/sprite emitter.

``src/world/material.h/.cpp``
    ``bind_parameters(shader)`` binds material values without submitting.
    The existing ``bind(view, depth)`` remains for immediate API clients.

``src/device/pipeline.h/.cpp``
    Native source preparation, parameter values and modifier registration.
    Atlas outputs are resolved through draw source descriptions, not resource
    names. Existing effect/overlay integration remains native.

Configuration selection
-----------------------

An inline layout contains ``layers`` and optionally ``global_resources``,
``resource_generators``, ``conditions`` and ``shader_layers``. An external layout
can instead be selected using::

    pipeline = "core/renderer/default"
    render_settings = { bloom = false }

Inline and external layouts cannot be combined. Only layout data is inherited,
not another config's render_settings. Source reads register compilation
dependencies, including the inherited shader-library requirements. There is no
recursive inheritance or array merging.

Resources and targets
---------------------

``global_resources`` contains named declarations. ``type`` is ``render_target``
(the default) or ``texture``. Supported formats are RGBA8, BGRA8, RGBA16F,
RGBA32F, R32U, D16, D24, D24S8 and D32F.

Specify ``width`` and ``height`` together, or a named ``size``. The default size
is ``backbuffer``. Crown supplies ``sun_shadow_map_size``,
``local_lights_shadow_map_size`` and ``lights_cookie_atlas_size`` from requested
render settings. Positive ``w_scale`` and ``h_scale`` scale that source.
Dimensions are floored and clamped to at least one pixel, subject to device
limits. Repeated resize does not compound scaling.

``count`` (1-16) creates separate successively halved 2D textures, not hardware
mip levels or a general equal-sized texture array. A reference is a resource
name or ``{ resource = "bloom" index = 2 }``. Optional flags are ``clamp``,
``point``, ``anisotropic``, ``compare`` and ``msaa``. ``initial_value`` supports
one 1x1 ordinary RGBA8 texture, using eight RGBA hex digits.

Layer ``render_targets`` and ``depth_stencil_target`` specify attachments.
Empty color attachments and no depth attachment mean the backbuffer. Attachment
dimensions and MSAA quality must agree. Framebuffers borrow their attachments;
RenderPipeline alone destroys textures, after destroying the framebuffers.

Layer execution
---------------

Layers execute in declaration order. Each geometry layer reserves ``count``
views (default 1, zero is allowed). Names identify declarations and routing
references; names do not select native behavior. bgfx view IDs are resolved at
runtime and may change after a layout reload.

``sort`` accepts ``default``, ``sequential``, ``front_to_back`` or
``back_to_front``. Mesh batches retain positive camera-distance keys; sprite
batches retain their authored depth keys. This is object/batch sorting, not
sorting intersecting triangles.

``transform`` accepts ``none``, ``camera``, ``ortho_unit``, ``ortho_screen``,
``ortho_graph`` or ``ortho_unit_flipped``. The ``camera`` transform uses the
Device render invocation's matrices. A prepared shadow source supplies its own
camera and viewport. ``manual_rect = true`` leaves the initial rectangle to
that source. A prepared rectangle must fit its configured output.

``clear`` accepts ``color``, ``depth`` and ``stencil`` booleans, ``rgba`` hex,
``depth_value`` and ``stencil_value``. A configured clear executes when the view
receives work; ``touch = true`` also executes an otherwise empty view. Removing
a clear does not silently insert a replacement or disable subsequent draws.
For a zero-view layout only, presentation clears the backbuffer black without
any scene or effect draws.

Explicit drawing
~~~~~~~~~~~~~~~~

A geometry layer can contain::

    draw = {
        type = "draw_visible"
        source = "main_camera"
        context = "color"
        types = [ "mesh" ]
        parameters = "forward_lighting"
        inputs = [
            { texture = "sun_shadow_map" sampler = "u_cascaded_shadow_map" stage = 10 }
            { texture = "local_lights_shadow_map" sampler = "u_local_lights_shadow_map" stage = 11 }
            { texture = "lights_data" sampler = "u_lights_data" stage = 12 }
            { texture = "lights_cookie_atlas" fallback = "lights_cookie_fallback"
              sampler = "u_lights_cookie_atlas" stage = 13 flags = [ "clamp" ] }
        ]
    }

Those texture names are references chosen by this example/default configuration;
they can be renamed along with their declarations. The shader sampler names and
stages are part of the selected shader's interface and must match it.

``source`` and ``type`` are required. ``context`` defaults to ``color`` and is a
routing key, not an implicit lighting or shader operation. ``types`` selects
``mesh``, ``sprite`` and/or ``procedural`` batches; omitted means all three,
and an empty array selects none. ``filter`` is ``all`` (default), ``selected``
or ``shadow_casters``. A caster is an object selection, not a geometry type.

A registered source with no visible views/batches draws nothing. An unknown
source is an execution error. A layer without a draw description never consumes
batches, regardless of its name.

``view_mode`` determines how the layer's reserved views consume data:

* ``single`` (default): one source view, chosen by ``source_index`` (default 0).
  The layer can reserve zero or one view.
* ``views``: backend sub-view j consumes prepared view ``source_index + j``.
  This is used for cascades, local-light faces and cookie tiles.
* ``groups``: all sub-views share one prepared source view. Each consumes batches
  whose ``group`` equals its sub-view index, unless an explicit shader route
  selects that batch's destination. The default sprite layer uses eight groups.

``shader`` optionally overrides the material program. Without it, the batch's
material program/state are used. Procedural batches require an override.
``skinned_shader`` supplies the skinned override; a skinned batch cannot silently
fall back to a rigid override. Neither field generates new shader permutations.
Load the required shader libraries as usual. Custom deformation or alpha-masked
shadow/depth programs must be supplied by their author; this change does not
invent them from a color material.

``object_id_uniform`` writes the owning unit's ID bits into a vec4 for selection
shaders. ``uniforms`` uses the existing array of named constant vec4 values.
Binding precedence is material values, the named frame ``parameters`` set,
per-batch textures, explicit draw inputs, constants, then object ID. Render
state comes from the resolved shader, with native procedural overrides where
needed. Target MSAA quality controls the MSAA draw-state bit.

Drawing the same source twice
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The same prepared camera geometry can be consumed in different contexts::

    layers = [
        {
            name = "depth_prepass"
            depth_stencil_target = "scene_depth"
            transform = "camera"
            clear = { depth = true }
            touch = true
            draw = {
                type = "draw_visible"
                source = "main_camera"
                context = "depth"
                types = [ "mesh" ]
                shader = "my_depth"
                skinned_shader = "my_depth_skinned"
            }
        }
        // Add a color layer consuming main_camera, with color bindings/state.
    ]

This is a layout fragment, not a complete stock configuration: declare its
resources and shader programs. A subsequent color program must use a depth test
compatible with the prepass. Reusing geometry does not automatically change a
color shader's depth comparison.

Unrouted batches that match a draw's source/types/filter are consumed by that
draw. Therefore two matching draw declarations intentionally redraw them.
Use different contexts, explicit routes and/or filters to partition work.

Context-aware shader routing
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``shader_layers`` selects a destination/program per exact material shader ID
and context::

    shader_layers = [
        { shader = "my_surface" context = "depth" layer = "depth_prepass"
          program = "my_depth" skinned_program = "my_depth_skinned" }
        { shader = "my_surface" context = "color" layer = "opaque_color" }
    ]

The destination must be a draw_visible layer with the same context. ``index``
selects its sub-view (default 0). Duplicate shader/context routes are errors.
An explicit route suppresses that batch in other layers of the same context;
a disabled destination does not cause fallback rendering elsewhere. Without a
program override the layer's program selection applies. Avoid routing a shadow
material to one sub-view when it should appear in every prepared shadow view;
the default shadow layers use layer-level overrides instead.

Inputs and conditions
---------------------

Geometry and fullscreen operations share ``inputs`` and ``uniforms`` handling.
Inputs declare texture references, sampler names, stages and optional sampler
flags. Omitted flags preserve resource sampling flags. A ``fallback`` reference
supplies an allocated texture when the primary allocation is disabled. Both
references must exist, and neither may alias the operation's output attachment.
A fallback is not an automatic replacement for an unwritten/stale texture.

``conditions`` declares at most 32 names. Resources, layers and modifiers can
require ``if`` and exclude ``unless`` conditions. Consumers must imply the
allocation conditions of their inputs, or provide a guaranteed available
fallback. Allocation conditions require recreation to change; execution-only
conditions can change without renumbering views. Native settings continue to
provide selection/cookies/MSAA/bloom allocation values and per-frame bloom and
vignette values.

Crown's prepared sources
------------------------

These are native capability/source names, not layer names:

``main_camera``
    Culled meshes (including selected LOD meshes) and sprites. Use the layer's
    camera transform. Color, selection and custom depth contexts can share it.

``sun_shadows``
    Independently culled cascade views. The existing CPU/shader interface needs
    four cascades; an incomplete destination budget disables native sun-shadow
    preparation instead of advertising unrendered cascades to the shader.

``local_shadows``
    Spot views and four-face omni views prepared by the existing native
    algorithm. The configured contiguous source-view budget limits submission.

``light_cookies``
    Packed cookie tiles with per-view rectangles and texture bindings. Each
    successful tile reserves a view and its transient triangle during
    preparation. Allocation failure leaves the light without that cookie.

``local_shadow_stencil``
    The existing omni-shadow stencil geometry, consumed by an explicit
    procedural draw. The default local-shadow clear layer performs that draw;
    removing it does not implicitly recreate its stencil work.

``lights`` (upload source)
    Owned packed-light bytes. Use
    ``draw = { type = "upload" source = "lights" resource = "my_light_data" }``.
    The native layout remains 768x1 RGBA32F: 32 lights, 24 vec4 values per light.
    Generic uploads accept tightly packed color data and validate format,
    dimensions and byte count. They do not accept render-target destinations.

Native atlas consumers must use an appropriate texture attachment; all active
destinations for the same native atlas source must share it. Their size comes
from that attachment, not a global literal resource name. Source-view budgets
are contiguous from zero. Disabling a draw removes that destination's budget.
The native preparation algorithms and their CPU/shader ABI limits remain C++.

The ``forward_lighting`` frame parameter set supplies current light counts,
cascade matrices, shadow sampling parameters, fog and global/local-light values.
It is applied for every relevant draw, not inherited accidentally from a
previous encoder submission. Lighting sampler reflection remains intact;
geometry's pipeline texture bindings are now explicit configuration data.

Uploads are not view-sorted draws
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bgfx texture updates do not acquire ordering between GPU draws by touching a
view. The upload declaration controls whether/where a prepared payload is
uploaded, not an arbitrary between-draw texture mutation. Do not upload two
versions into one texture and expect different layers to see different versions.
Use separate resources or a suitable GPU operation for that requirement.

External overlays
-----------------

The existing GUI, debug, graph and ImGui APIs still submit immediately. They
resolve their destination using a declaration such as::

    { name = "my_overlay" transform = "ortho_screen"
      draw = { type = "external" source = "screen_gui" } }

Current streams are ``screen_gui``, ``world_gui``, ``debug``, ``graph`` and
``imgui``. Each has at most one declared destination. They do not become
camera-culled or replayable prepared geometry in this patch. Removing the
stream's declaration suppresses its submissions. Existing native overlay shader
caches are selected by source capability, not by the destination layer's name.

Generators and extension boundary
---------------------------------

Resource generators remain ordered modifier arrays. Fullscreen modifiers use
``fullscreen_pass`` with shader, inputs, constants and optional
``pixel_size_uniform``. Existing bloom/tonemap/vignette/outline callbacks retain
their native math and state. A generator layer cannot also contain ``draw``.
The existing one-view-per-modifier convention is unchanged; multi-view scene
work uses draw layers' explicit view budgets.

``RenderFrameContext`` registers frame operations by name/function. Native
code can build additional prepared sources without changing the generic draw
consumer. ``render_frame::draw_visible`` is also callable by a native modifier
with a valid draw context and prepared frame. It does not itself allocate views.
This does not add a resource-generator view-expansion protocol, automatic
scheduling or a general shader-permutation language.

Remaining boundaries and validation
----------------------------------

There is still one application-owned pipeline/resource set. Independent
simultaneous viewports, resource aliasing, arbitrary texture kinds, compute,
a visual pipeline editor and a different lighting technique are not added.
The native effect/condition setup, geometry layout knowledge, light selection,
shadow projections, atlas packing and backend depth/MSAA policy remain native.
Some unused legacy non-owning aliases remain in Pipeline for API compatibility;
the new scene draw path does not use them to select resources or destinations.

Run::

    python3 tests/render_pipeline/run.py --sanitize

The suite compiles production compiler tables, pipeline/frame execution,
material binding and selected native gathering helpers against recording test
doubles. It covers renamed declarations/references, explicit/empty drawing,
context routes, repeated geometry, skinning-cache reuse, selection ownership,
per-shadow result isolation, transient-buffer reuse/failure, fallback bindings,
copied uploads, bounds, conditions, reloads and resource ownership. The separate
lighting suite compiles the production ShaderManager with simulated reflection.

These are not GPU tests. Containers, SJSON, hashing, supporting scene APIs and
bgfx are substituted. The complete RenderWorld preparation/culling function,
Device and GUI translation units are not compiled by this suite. A full engine
build, fresh data compile, real backend validation and image comparisons for
meshes/skinning/LOD/sprites/shadows/cookies/selection/GUI remain required.

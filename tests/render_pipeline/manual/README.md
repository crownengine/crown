# DDRP manual acceptance guide

Target: `c4911eb7d5871676d9d1ffad7ee8e8a0c8041b5d` plus the PR-review fixes.
These are tests to run, not a claim that a GPU has already passed them.

## What is being tested?

The pipeline configuration controls orchestration. It does not create a new
lighting model merely because a layer is named differently.

| Concept | Meaning | A useful experiment |
|---|---|---|
| Global resource | GPU image storage, format and dimensions | Make an intermediate image half resolution. |
| Layer | An ordered destination, clear, transform and sorting policy | Exchange two clear layers; the last clear wins. |
| Draw operation | A consumer of a prepared source | Remove `draw`, or change its `types`/`filter`. |
| Prepared source | Geometry/data gathered for a camera, light or other producer | Consume camera geometry in color and selection contexts. |
| Shader context | A key for selecting a material's program | Use a specialized shadow program in all light views. |
| Shader route | An optional exact layer/sub-view restriction | Send a material only to a deliberately selected destination. |
| Resource generator | An ordered array of modifiers | Generate an image, then copy it to the backbuffer. |
| Bindings | The textures and constants read by an operation | Change a color uniform, or select another input texture. |
| Conditions | Enable execution or resource allocation | Toggle bloom/cookies/selection without inventing alternate C++ paths. |
| External source | Destination for an immediate GUI/debug stream | Remove that destination and confirm only that overlay disappears. |

Mesh, sprite, selection, shadows, cookie copies and local-shadow stencil geometry
are prepared and consumed by explicit draws. Lighting uploads use `type =
"upload"`. Bloom, vignette, tonemapping and outlines still use their native
parameter callbacks around configured fullscreen draws. Culling, light selection,
shadow projection, packing and bone-atlas math remain native. This is not yet a
render graph, deferred renderer, compute framework or independent multi-viewport
resource system. Generator modifiers still reserve one view each; multi-view
scene drawing belongs to a layer's `draw` description.

## Prepare once

Apply the patch to the target commit, regenerate build projects to include
`src/device/render_config_shaders.cpp`, and rebuild the engine/data compiler.
Recompile project data: the render-config resource version changes **11 -> 12**.
Shader/material binary formats are unchanged by these fixes. Restart the editor
backend and game once to load the rebuilt executable.

Run the recording regressions from the checkout:

```sh
python3 tests/render_pipeline/run.py --sanitize
```

`--review-only` selects the new tests. Run the repository formatter before
submitting the PR, then inspect and commit its diff:

```sh
make MAKE_JOBS=4 format-sources
git diff --check
```

The test runner and formatter serve different purposes. Passing either is not
proof of correct rendered pixels.

Keep the project's existing `boot.config` render-config selection. Work on a
backup of that selected file; there is no reason to change the selection for
these tests. Put backups outside the project source tree. A source backup named
`something.render_config` inside the project may itself be compiled.

For the commands below, set the real paths once:

```sh
PROJECT=/absolute/path/to/your/project
CONFIG="$PROJECT/path/to/your-selected.render_config"
FIXTURES=/absolute/path/to/crown/tests/render_pipeline/manual
BACKUP=/absolute/path/outside/project/ddrp-original.render_config
cp "$CONFIG" "$BACKUP"
```

After every configuration edit, use the normal **compile and refresh** action
for the running editor/game backend, and check that compilation succeeded and
that the runtime logged the active configuration's reload. Saving text alone is
not a reload. A stopped standalone game must be launched with the freshly
compiled data. Do not restart between the steps specifically marked as a
hot-reload test.

Fixtures have `.txt` extensions intentionally. Copy one over `$CONFIG` at a time;
do not install all invalid examples as `.render_config` assets. The probe shader
is also staged as text until the shader-library test asks you to install it.

Prepare a reference level with a receiving floor, rigid objects, a sprite,
a directional light and a spot or omni light. For the later checks add two
animated objects with visibly different poses, an LOD group, a distinctive light
cookie, an emissive object, and your normal sky/GUI. Save a baseline screenshot
with your normal pipeline. First do the simple color tests; add the more complex
objects only when reaching their tests.

## 1. Empty layout and explicit clears

```sh
cp "$FIXTURES/00-empty.txt" "$CONFIG"
```

Compile and refresh. **Expected: black**, with no scene or post-effect draws.
The empty pipeline has a presentation-only black clear; it does not recreate
missing default layers. The editor's surrounding application UI is unaffected.

Now:

```sh
cp "$FIXTURES/01-red.txt" "$CONFIG"
```

Compile and refresh. **Expected: solid red.** The entire configuration is:

```sjson
shaders = []
layers = [
    { name = "red" clear = { color = true rgba = "ff0000ff" } touch = true }
]
```

Change `rgba` to `0000ffff`: expect solid blue. Return to `00-empty.txt`: expect
black, not the previous color. Run the same sequence in the editor and game.

This isolates clear execution and hot reload from lighting, materials and
shadows. `touch = true` makes a clear-only view execute even without geometry.
With no touch and no draw, a configured clear is not guaranteed to execute;
retaining the previous image is not a useful acceptance criterion in that case.

## 2. Declaration order

Install `02-order.txt`, compile and refresh. It declares red and then blue
clears. **Expected: blue.** Exchange the two complete entries: expect red.
Rename both layers without changing their order: expect the same result.

This is layer order, not material depth sorting. Do not infer the final GPU order
from the order in which native C++ producers gather their batches.

## 3. Load a genuinely new shader library without restarting

This is the direct manual regression for the reload finding.

Start the backend with `01-red.txt` active **before** installing the probe. The
unique `ddrp_tests/probe` library and its programs must not already be resident.
When repeating from a fresh process, remove the previously installed probe before
startup, or use a new unique library/program name.

While that process remains running:

```sh
mkdir -p "$PROJECT/ddrp_tests"
cp "$FIXTURES/probe.shader.txt" "$PROJECT/ddrp_tests/probe.shader"
cp "$FIXTURES/03-live-shader.txt" "$CONFIG"
```

Compile and refresh. **Expected: a green/dark-green checkerboard**, not red, black
or the fallback material's magenta. Its cells are eight target pixels wide.
Change `u_ddrp_test_color` in the configuration from `[ 0 1 0 1 ]` to
`[ 0 0 1 1 ]`: expect a blue/dark-blue checkerboard without restarting.

Restore `01-red.txt`; compile and refresh. Expect red again. Restore
`03-live-shader.txt`; compile and refresh. Expect the checkerboard again. Repeat
addition/removal several times, then close the backend normally. Shared libraries
used by the scene or other packages must continue to work.

This tests new requirements becoming available before the replacement pipeline
resolves its programs. The fixture's `static_compile` entries are important:
loading a library does not automatically create every imaginable permutation.

A correct image cannot prove reference counts or prove that an unused shader was
freed. The automated ownership test checks those properties. For a debugger
check, inspect the library owner's `_libraries` and resource-manager references:
the active layout's one reference should disappear when removed, while another
package's reference remains. Shader programs shared across libraries may remain
alive legitimately.

## 4. Intermediate resources, input bindings and resolution

With the probe shader installed, use `04-offscreen.txt`. It generates the same
checkerboard into `small_image`, then copies that image to the backbuffer with
point filtering.

At an even-sized viewport, **expected: approximately 16-screen-pixel cells**,
because each eight-pixel source cell is enlarged from half resolution. Change
both `w_scale` and `h_scale` from `0.5` to `0.25`: expect approximately 32-pixel
cells. Change the uniform color: only the image color should change.

Resize the viewport wider, taller and smaller, then return to its original size.
The image must continue covering the whole viewport. Its relative resolution
must not compound on repeated resizes. Odd-sized viewports involve rounding, so
one-pixel differences at boundaries are expected.

Rename `small_image` and update its declaration, output and input references.
The image should remain unchanged. Changing only one reference should give a
compilation error, not a magic resource-name fallback.

The two modifiers are ordered: generation precedes consumption. Their inputs
and outputs are explicit. Reversing them is deliberately asking to consume an
image before generating the current frame's contents; the runtime does not infer
or repair a dependency graph for you.

## 5. Configuration validation and inheritance

With the probe installed, test each invalid fixture separately:

| Fixture | Expected compilation result |
|---|---|
| `90-invalid-mixed.txt` | Reject inline shader routes combined with an external `pipeline`. |
| `91-invalid-dangling.txt` | Reject a target name with no resource declaration. |
| `92-invalid-feedback.txt` | Reject sampling the same image that is the modifier's output. |

The previous working runtime layout should remain active after a failed data
compilation. Restore a valid file before proceeding. Runtime/device errors such
as unsupported formats are different from compile errors; this patch does not
make every GPU initialization failure recoverable.

For a valid inheritance test, install `05-inherit.txt`:

```sjson
pipeline = "core/renderer/default"
render_settings = { bloom = false }
```

It uses the default layout and its required shader libraries, with the selecting
file's render settings. Boot/user/editor settings may override a render-setting
value; check the effective settings when testing a toggle. This is not array
merging. Adding `conditions = []`, `shader_layers = []` or `shader_contexts = []`
to this inherited configuration must now fail rather than silently disappear.
Use a complete inline layout to author those arrays.

## 6. Explicit visible-object drawing, filters and names

Restore your original configuration:

```sh
cp "$BACKUP" "$CONFIG"
```

Compile and refresh; compare with the baseline. For experiments using the stock
names below, start from the current `samples/core/renderer/default.render_config`
or locate the equivalent entries in your customized file.

In the camera mesh layer, retain the layer, targets and clear/touch policy, but
change only its draw's `types` to `[]`. **Expected: no mesh color submissions from
that layer.** A skydome is also mesh geometry and may disappear. Sprites, GUI,
shadows and selection have separate consumers and need not disappear with it.
Restore `types = [ "mesh" ]`: the meshes return.

Remove the entire `draw` block from that layer: it still must not implicitly draw
meshes just because its name is `mesh`. Restore the block, rename the layer and
update any explicit routes: geometry should render again without C++ changes.

Try `filter = "selected"` on the camera draw. In the editor only selected batches
from that source should be submitted by that layer. A game with no selected
units may consequently show none. Remove the filter to restore all eligible
batches. `shadow_casters` is another selection filter, not a new geometry type.

Do not remove just the lighting input bindings to test geometry removal. A lit
program needs its shader-interface textures; missing those is a binding error,
not evidence that the geometry producer was not executed.

## 7. Sprite groups and batch depth sorting

Use overlapping sprites with distinct colors/alpha, authored depths and at least
two sprite-layer indices. The default sprite draw uses `view_mode = "groups"`
and eight reserved sub-views. Changing a sprite's group changes the sub-view
that consumes it; changing its depth changes sorting within that group.

Compare `sort = "front_to_back"` and `"back_to_front"` on that layer using these
order-sensitive sprites. Restore the default afterwards. Ordinary opaque meshes
with correct depth testing may produce the same final image with either sorting
order, so they are a poor visual test of sort keys. Capture submitted depths and
view order when certainty is needed.

Set the sprite layer's count to zero: sprite color draws disappear. Restore eight:
they return. This is a configured view budget, not a change to sprite geometry or
an instruction to allocate a new kind of resource.

## 8. Program selection independent of layer routing

Use a complete inline layout, with no `shader_layers` route for the material/key
being tested. A stock rigid primitive material uses `shader = "mesh"`. Other
materials may have a compiled variant suffix, so use their exact resolved shader
identifier; this table is not keyed by material filename.

First verify that your chosen material actually matches. Add:

```sjson
shader_contexts = [
    {
        shader = "mesh"
        context = "color"
        program = "fallback"
        skinned_program = "mesh+SKINNING"
    }
]
```

With the default shader library loaded, matching rigid primitives should now use
the distinctive magenta fallback program. The skinned override is supplied so a
matching skinned batch is not sent through the rigid fallback program. This
check is only to establish the lookup key; remove it after observing the change.

Now replace that entry with:

```sjson
shader_contexts = [
    {
        shader = "mesh"
        context = "shadow"
        program = "shadow"
        skinned_program = "shadow+SKINNING"
    }
]
```

Matching objects should keep casting shadows in **all applicable cascades and
local-light views**, not only cascade zero. Use near/far objects and both a sun
and a local light. Move the camera across cascade boundaries and move the local
light. A graphics capture should show the same matching material in every
applicable source view using the configured shadow program. The image should
match the stock shadow result because this example deliberately selects the
stock program.

Repeat using a skinned material's exact compiled shader ID as another entry.
A specialized alpha-masked shadow program can replace `shadow`, but must itself
implement masking and be compiled by a loaded library. The pipeline does not
invent a masked/deformed shader from its context name.

`shader_layers` is different: it intentionally restricts a key to an exact layer
and sub-view. Adding such a route for `context = "shadow"` can intentionally
remove that material from the other shadow views. Do not add one merely to choose
a shadow program. A route without program fields can coexist with
`shader_contexts`; conflicting program definitions in both tables are rejected.

## 9. Reusing a prepared source for a depth prepass

This optional test requires an opaque-only scene with no skydome or custom
vertex deformation. Use the stock rigid/skinned geometry and shader libraries.
Insert a layer after the main depth clears and before the main color draw:

```sjson
{
    name = "depth_prepass"
    depth_stencil_target = "depth"
    transform = "camera"
    clear = { depth = true }
    touch = true
    draw = {
        type = "draw_visible"
        source = "main_camera"
        context = "depth"
        types = [ "mesh" ]
        shader = "shadow"
        skinned_shader = "shadow+SKINNING"
    }
}
```

The stock shadow programs supply depth-only geometry for this constrained test.
The stock opaque color state's `lequal` comparison accepts the same geometry in
the following color draw. **Expected: the opaque geometry/animation remains
aligned**, while a capture shows the additional depth submission. Removing the
prepass should not change poses or cause LOD transitions to advance differently.

For your own materials, use real depth-context variants with the same masking and
deformation as their color variants. A context named `depth` does not itself
alter shader state. Do not generalize this fixture to arbitrary alpha-tested,
transparent or camera-relative sky geometry without corresponding shader work.

## 10. Lighting, shadows, cookie copies and uploads

Restore the normal pipeline and confirm each light type illuminates the scene.
Then change one operation at a time:

| Change | Expected result |
|---|---|
| Set `sm_cascade.count = 0` | No native directional shadow rendering; other rendering continues. Restore four. |
| Set `sm_local.count = 0` | No local shadow submissions. Restore its view budget. |
| Set `lights_cookie_atlas.count = 0` | No native cookie copies; the native producer advertises no active cookies. Restore its budget. |
| Remove the `lights` layer's upload draw | No upload from that operation, and the native forward-light count is zero when no active upload destination exists. Ambient/emission may still be visible. Restore the draw. |

The sun algorithm still needs four cascades; a smaller budget does not redefine
its projection math. Omni shadows use several local views. A small local budget
can therefore admit a spot light while excluding an omni caster requiring more
views. Budget exhaustion must not write into another layer's view range.

Place a caster just outside the camera view with its shadow falling on a visible
surface. The shadow should remain: shadow sources are not built only from the
camera-visible mesh list. Test two local lights and moving casters to expose
accidental reuse of another light's prepared culling result.

Use an asymmetric cookie and rotate/move its light. Check atlas copies, orientation
and boundaries against your baseline. Switch cookies off/on, and test a cookie
that cannot fit the atlas: it must not claim a successful copy or sample another
cookie's rectangle.

Removing `sm_cascade_clear` means **stop clearing that atlas**, not clear the
screen or disable the following shadow draws. Old/unwritten contents can be a
legitimate consequence of that deliberately authored pipeline. Use the explicit
clear-only tests for deterministic image checks.

## 11. Animation, selection and native effect regression

Use two animated skeletons with different poses and material-slot layouts. Let
both move, turn the camera so one leaves/re-enters view, and select each in the
editor. Repeat with shadows enabled. **Expected:** no palette swaps, stale poses,
exploding meshes or shadow/selection poses belonging to another object.

Select an LOD-group owner while its chosen mesh changes with distance. Its
outline/ID should remain the group's, not the chosen child mesh's. Inspect ID
attachments in a capture for a definitive check. Color, selection and depth draws
must not independently advance that LOD transition.

Exercise bloom, vignette, color grading/tonemapping and editor outlines with your
usual controls. Keep their output/bypass chain intact when disabling an effect;
deleting a generator that produced the final color is not equivalent to an
implemented bypass. Check selection off/on, MSAA off/on at the supported quality
levels, and cookies off/on together with resize and reload.

Do not assume a `.render_config` render-setting value overrides an explicit
boot/user/editor setting. The clear, draw and resource experiments above directly
author the layout and avoid that ambiguity.

Rename or remove an external GUI/debug destination and restore it. Existing GUI,
debug and graph APIs remain immediate streams. They must not submit to an
unrelated numerical view after the layout is reordered or reloaded.

## 12. Persistent-view regression: what config-only edits cannot prove

Recompiling a layout recreates its views, so changing `manual_rect` through a
reload alone does **not** reproduce the original stale-state bug.

The automated review test keeps the same runtime alive and alternates a prepared
source between these states over six frame advances:

```text
A: camera=true, viewport=true, rectangle=(16,24,32,40), custom matrices
B: camera=false, viewport=false, configured 128x128 destination and transform
```

It checks full-size/default restoration for orthographic, identity and camera
layers, plus preservation of a manual external stream. A custom C++ source can
repeat the sequence for GPU inspection. Stock shadow/cookie producers normally
supply their overrides consistently, so they are useful regression coverage but
not a direct reproduction of the missing-override transition.

A `manual_rect` source must supply a rectangle each time it provides a view.
Omitting it is now an error, not permission to reuse a previous frame's rectangle.

## 13. Performance and final acceptance

Compare a fixed animated scene with a growing number of visible skinned objects,
then with more shadow views. Measure CPU preparation and submission separately
from GPU time, using the same camera, materials and build. Rendering additional
geometry and applying its uniforms still costs time; the eliminated work is
searching unrelated anonymous palette records for every forward-lit draw.

The recording regression proves that adding 7,000 unrelated uniform records does
not add draw-time array accesses for parameter lookup. It is a complexity check,
not an FPS benchmark or a claim that all remaining pipeline work is linear.

Before updating the PR, restore the real configuration and verify: normal startup,
repeated valid reloads, shader-library addition/removal, resize, empty-to-normal
layout transitions, animated shadows/selection, and orderly shutdown. Run the
same baseline on each backend you support. Also test a packaged/bundled cold
startup: it must retain its package backing storage. Live reload is intentionally
not supported in bundle mode.

Record the commit/patch, backend, build type, scene, exact configuration edit,
expected/actual image and relevant log for failures. A useful final checklist is:

| Test | Editor | Game | Notes/capture |
|---|---|---|---|
| Empty/red/order | | | |
| Previously unloaded shader library | | | |
| Offscreen scale/rename/resize | | | |
| Invalid configurations rejected | | | |
| Mesh/sprite/filter/external streams | | | |
| Context override in all shadow views | | | |
| Two skeletons, LOD selection, prepass | | | |
| Effects, MSAA, cookies | | | |
| Cold startup and shutdown | | | |
| Bundled cold startup | n/a | | |

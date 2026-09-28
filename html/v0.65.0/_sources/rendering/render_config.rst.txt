=============
Render Config
=============

The Render Config specifies global render-related settings and shader libraries
to load at startup and in the future will be expanded to support defining full
data-driven rendering pipelines.

Multiple .render_config resources can exist in a project, each optimized for a
particular use case. You can choose which render config the runtime will use by
specifying it in the :ref:`Boot Config <boot.config reference>`.

If you don't specify any, the default render config is used:
``core/renderer/default.render_config``.

The render_settings block
-------------------------

This block contains a number of global rendering settings. Each setting in this
block can be overridden via the :ref:`Boot Config <boot.config reference>` on a
general or per-platform basis.

The shaders block
-----------------

This block lists the shader resources to load at runtime when the render config
is used. Add your custom shader resources to this list to ensure they are
properly loaded.

==================
Importing Textures
==================

Textures are usually added to the project indirectly by higher-level
importers (for example when :ref:`Importing Scenes` or :ref:`Importing
Sprites`). You can also import images as standalone textures when you need it
for UI, materials, or other specialized uses.

When you import a texture, Crown copies the source image into
the :ref:`project <project>` and creates a ``.texture`` resource file with
the same name. The ``.texture`` resource stores processing settings for each
target platform (compression, mipmaps etc.). Crown uses the ``.texture`` file
at compile time to produce optimized GPU-ready assets.

Open the :ref:`Texture Settings` tool to review or modify processing options.

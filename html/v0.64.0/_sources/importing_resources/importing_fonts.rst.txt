===============
Importing Fonts
===============

Crown can import fonts from popular font file formats. Importing a font produces
a ``.font`` resource (a map of glyphs), an accompanying ``.texture`` resource
(an atlas containing all the glyphs) and a ``.material`` resource suitable for
drawing text.

.. figure:: images/import_font.png
   :align: center

   The Import Font dialog.

Dialog overview
===============

On the left, the importer shows a preview of the final texture atlas. On the
right you can set various parameters that control which characters are included
in the atlas and their size. This size does not have to match the size used when
drawing text, but it is good practice to keep them similar to avoid scaling
artifacts.

Renaming the font
=================

Font files are often named with variants and other details embedded in the
filename. If you prefer a simpler name, you can rename the font before
importing by entering it in the ``Name`` field.

Atlas generation
================

The atlas dimensions are shown in ``Atlas size``. They are determined
automatically from the font ``Size`` and the selected ``Charset``. Enter the
desired font size and press ``Enter``; Crown will regenerate the atlas and
update the preview. Glyphs are packed as tightly as possible to keep the
atlas small and avoid runtime performance issues.

Custom charset
==============

By default Crown provides several common character sets in the ``Charset``
dropdown. For a custom range, use ``Range min`` and ``Range max`` to set the
first and last Unicode code point to include in the font.

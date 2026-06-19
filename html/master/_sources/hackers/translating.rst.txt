===========
Translating
===========

Crown tools use GNU gettext for handling translations. Source strings are marked
in code with ``_()`` or ``N_()``. Use ``C_()`` or ``NC_()`` when a string needs
translation context. Translations live in ``tools/po``.

Files
=====

``tools/po/POTFILES``
    Source files scanned by ``xgettext``. Add a file here when it contains
    translatable strings.

``tools/po/crown-editor.pot``
    Template generated from the source files. Do not edit translations here.

``tools/po/LINGUAS``
    List of languages to build and ship.

``tools/po/<lang>.po``
    Translation for ``<lang>``.

Creating a New Translation
==========================

1. Create the ``.po`` file from the current template. For example, for Italian:

   .. code::

        msginit --input=tools/po/crown-editor.pot --locale=it --output-file=tools/po/it.po

2. Add the language to ``tools/po/LINGUAS``.

3. Translate ``msgstr`` values in ``tools/po/it.po``. Do not change ``msgid`` values.

Updating an Existing Translation
================================

1. Regenerate the template and merge it into the language file:

.. code::

	make -B -C tools/po pot
	msgmerge --update tools/po/it.po tools/po/crown-editor.pot

2. Translate new empty or fuzzy entries, then remove any remaining fuzzy markers.

Checking a Translation
======================

Validate a ``.po`` file with:

.. code::

	msgfmt -c --statistics -o build/linux64/bin/po/it/LC_MESSAGES/crown-editor.mo tools/po/it.po

The output should not report fuzzy or untranslated messages before committing a
completed translation.

Build and Test
==============

The tools build compiles every language listed in ``tools/po/LINGUAS`` into the
build directory:

.. code::

	make crown-editor-linux-debug64

Run the editor in a language with:

.. code::

	env -u LC_ALL LANGUAGE=it build/linux64/bin/crown-editor

``LC_ALL`` overrides other locale variables, so unset it while testing. Using
``LANG=it_IT.UTF-8`` also works when that locale is installed on the system
``LANGUAGE=it`` is usually easier for quick gettext tests.

Translation Rules
=================

Keep the source capitalization style. If the English text is title case, use
title case in the translation too, for example ``New Project...`` becomes
``Nuovo Progetto...``.

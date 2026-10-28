==================
Deploying to HTML5
==================

Configure HTML5 deployer
========================

Install emscripten
------------------

Install emscripten:

   * https://emscripten.org/docs/getting_started/downloads.html

Set environment variables
-------------------------

.. code::

	export EMSCRIPTEN=<emsdk>/upstream/emscripten

Running the generated output
----------------------------

Use `emrun` to run the generated `index.html`:

.. code::

	cd <output_dir>
	<emsdk>/upstream/emscripten/emrun index.html

Bundle URL
----------

Set **Bundle URL** in the deploy dialog, or pass ``--bundle-url <url>`` to
``crown-editor --deploy``, to load bundle data over HTTP instead of preloading
``data.bin`` into the Emscripten filesystem.

Deployment exports a ``data/`` directory alongside the runtime instead of
``data.bin`` and ``data.js``. Upload it beneath the specified bundle URL.

Runtime arguments
-----------------

Set runtime arguments in the ``Module`` object in ``index.html``:

.. code-block:: javascript

	arguments: ['--bundle-url', 'https://example.com/game/'],

The URL ``?bundle-url=https://example.com/game/`` passes the same arguments.
URL options take precedence over HTML options.

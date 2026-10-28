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

Runtime arguments
-----------------

Set runtime arguments in the ``Module`` object in ``index.html``:

.. code-block:: javascript

	arguments: ['--bundle-url', 'https://example.com/game/'],

The URL ``?bundle-url=https://example.com/game/`` passes the same arguments.
URL options take precedence over HTML options.

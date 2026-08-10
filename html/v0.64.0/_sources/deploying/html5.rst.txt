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

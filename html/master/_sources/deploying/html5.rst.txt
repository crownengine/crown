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

Streaming data
--------------

HTML5 deployments normally download all game data up front. This makes
deployment simple, but players have to wait for the download to finish before
the game can start.

Streaming downloads resources as the game needs them, reducing the amount of
data needed at startup. Music and other streamed resources are read in small
chunks as they are used, so the whole file does not have to be downloaded first
or kept entirely in memory.

Streaming requires configuration in both Crown and the web server. Set **Bundle
URL** in the HTML5 deploy dialog to the URL where you will host the game data.
The deployer exports a ``data/`` directory alongside the runtime instead of
``data.bin`` and ``data.js``. For example, with **Bundle URL** set to
``https://example.com/game/``, upload ``data/`` to
``https://example.com/game/data/``.

On the server hosting ``data/``, enable byte-range requests and disable HTTP
compression. Range requests must return ``206 Partial Content`` with a matching
``Content-Range``. For cross-origin hosting, set ``Access-Control-Allow-Origin``
to the game's origin and include ``Content-Range`` and ``Content-Encoding`` in
``Access-Control-Expose-Headers``.

If the server cannot serve uncompressed ranges, Crown downloads each resource in
full to temporary OPFS storage before opening it. This fallback requires OPFS
and HTTPS or localhost.

Runtime arguments
-----------------

Set runtime arguments in the ``Module`` object in ``index.html``:

.. code-block:: javascript

	arguments: ['--bundle-url', 'https://example.com/game/'],

The URL ``?bundle-url=https://example.com/game/`` passes the same arguments.
URL options take precedence over HTML options.

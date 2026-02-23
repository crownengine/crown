================
Importing Sounds
================

Crown supports importing both short sound effects and longer music tracks.
Importing any supported audio file produces a ``.sound`` resource in the
project alongside the original audio file.

Audio streaming
===============

During project compilation Crown inspects the audio file and chooses an
appropriate processing strategy:

* Short sound effects are typically preloaded into memory so playback can
  start with no delay.
* Long music tracks use the original file compression and are streamed in at
  runtime to reduce memory usage.

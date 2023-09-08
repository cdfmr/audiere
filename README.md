This is an unofficial update of Chad Austin's Audiere, a high-level audio library.

**Differences from the official release 1.9.4**

* Replace mp3 decoder with minimp3 to get much better mp3 compatibility.
* Make mod streams seekable.
* Remove Speex support.
* Add some widechar functions.
* Add a group of simple playback functions (audiere_simple.h).

**Notice**

* The code is only tested on Windows system with Visual C++ 6, but the compiled dll works with Windows 10/11 perfectly.

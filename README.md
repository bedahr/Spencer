Spencer
=======

Speech Based Recommender System (Recomment 2.0)
Voice controlled, conversational, natural language recommender system.
Uses BBSimoneShared to talk to Simond (from the simon-tools package), set the path to it in `spencer.pro`.

The paths to the Avatar images is set at compile time and needs to be changed to your local system: `./src/ui/avatar/avatar.cpp:17`.
You can download the default avatar here: https://oc.grasch.net/index.php/s/SqPyp0vstTLwA9i

To launch, start mongodb (see mongodb/ subfolder), start MARY TTS (after installing the German voice bits3-hsmm), and start Simond (which you configured earlier to use the SBM shipped in `asr/` - the easiest way to do it is to just replace the currently used `.sbm` of a well configured Simon user with the file).

Please note that product images are not part of this distributio.

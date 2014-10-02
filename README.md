# Analysis Project #

## Overview ##
Uses a C library, and a C++ application using the [SFML library](http://sfml-dev.org/) (2.1)

### Basic functionality ###
The C library providing the audio analysis is found in the source directory.

The C++ application is found in applications/player. 
It has a MyStream class which overrides the SoundStream class to request samples at a rate of 512 (instead of the default 22050). 
These streams are then converted into a 32-bit floating point non-interleaved buffer. 
These buffers for each channel are then sent to the library, which performs analysis calculations before returning a result. 
This result is then translated into an update in the GUI.

### How do I get set up? ###

* You will need the SFML library 2.1 http://sfml-dev.org/download/sfml/2.1/
* Select the relevant version for the compiler/IDE that you will be using.

* If your build environment is not listed then you will need to download the source code from the bottom of the page.
* Once you have downloaded the source code, then follow the CMake tutorial to build the files for your compiler: 
http://sfml-dev.org/tutorials/2.1/compile-with-cmake.php


Then you must copy over each dll you have referenced (sfml-audio-2.dll, graphics, system, window) from SFML/bin into the build folder.
You must also copy across libsndfile-1.dll and openal32.dll. Then the application will be able to run.
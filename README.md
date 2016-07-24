vamp-aubio-plugins
==================

A set of [Vamp plugins](http://vamp-plugins.org/) for audio feature extraction
using the [aubio library](http://aubio.org/).

This set includes the following plugins:

 - Spectral Descriptors

  - Aubio Beat Tracker
    - *Time → Tempo*
    - Estimate the musical tempo and track beat positions.
  - Aubio Mel-frequency Band Energy Detector
    - *Low Level Features*
    - Computes Energy in each Mel-Frequency Bands.
  - Aubio Mfcc Detector
    - *Low Level Features*
    - Computes Mel-Frequency Cepstrum Coefficients.
  - Aubio Note Tracker
    - *Notes*
    - Estimate note onset positions, pitches and durations.
  - Aubio Onset Detector
    - *Time → Onsets*
    - Estimate note onset times.
  - Aubio Pitch Detector
    - *Pitch*
    - Track estimated note pitches.
  - Aubio Silence Detector
    - *Low Level Features*
    - Detect levels below a certain threshold.
  - Aubio Spectral Descriptor
    - *Low Level Features*
    - Computes spectral descriptor.

Build Instructions
------------------

You will need to have Python, git, and a C++ compiler.

Please refer to the build script corresponding for your platform for brief
instructions on how to build this project:

## Available OS scripts

  - `./build_linux.sh` for Linux
  - `./build_osx.sh` for Mac OS X
  - `./build_win32.sh` for Windows (32-bit)
  - `./build_ming32.sh` to cross-compile using [Mingw](http://www.mingw.org/)

## Windows

The preferred compiler on windows is Microsoft Visual 2013. Also you will want
to use a shell environment, for instance Git Bash, and have Python installed
and found in the PATH.

### Clean up

Use the following command to start from scratch:

    $ rm -rf contrib/ build/

Old-school makefiles
--------------------

This method is now considered **deprecated**.

The current build system is waf. See above, and read `wscript` and `build*.sh`
to find out how to use it. Makefiles are kept for the record, but they might be
out of date and will eventually disappear.

   $ make -f Makefile.<os_name> clean all

where `os_name` should be replaced by one of `linux`, `mingw32`, or `osx`.

Installation Instructions
-------------------------

The Vamp plugin is defined by the following three files. Depending on your
platform, the extension of the binary file will vary.

    vamp-aubio.cat
    vamp-aubio.n3
    vamp-aubio.{so,dll,dylib}

Follow the [Vamp installation
instructions](http://vamp-plugins.org/download.html#install) to copy the
above three files to your preferred plugin directory.

Copyright and License Information
---------------------------------

    Copyright (C) 2006-2012 Chris Cannam and Queen Mary University of London
    Copyright (C) 2006-2015 Paul Brossier <piem@aubio.org>

vamp-aubio-plugins is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your option) any
later version.

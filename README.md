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

Please refer to the detailed instructions to build vamp-aubio-plugins for your
platform in the following files:

  - `INSTALL.osx` for Mac OS X
  - `INSTALL.linux64` for Linux amd64
  - `INSTALL.linux32` for Linux i686
  - `INSTALL.mingw32` to cross-compile using [Mingw](http://www.mingw.org/)

You can try running the corresponding file directly from the current directory.
For instance, on a `Linux x86_64` host:

    $ sh INSTALL.osx

### Linux

Use Makefile.linux to compile vamp-aubio-plugins:

    $ make -f Makefile.linux clean all

### Mac OS X

Use Makefile.osx to compile vamp-aubio-plugins:

    $ make -f Makefile.linux clean all

### Windows

Use Makefile.mingw32 to compile vamp-aubio-plugins:

    $ make -f Makefile.mingw32 clean all

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

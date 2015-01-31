#! /bin/sh

# cross compile vamp-aubio-plugins using mingw32 toolchain

pushd ..

# get Vamp SDK
curl -O https://code.soundsoftware.ac.uk/attachments/download/690/vamp-plugin-sdk-2.5.tar.gz
tar xf vamp-plugin-sdk-2.5.tar.gz

# get Vamp windows binaries
curl -O https://code.soundsoftware.ac.uk/attachments/download/694/vamp-plugin-sdk-2.5-binaries-win32-mingw.zip
unzip -ox vamp-plugin-sdk-2.5-binaries-win32-mingw.zip

# build aubio
git clone git://git.aubio.org/git/aubio/ aubio-mingw32
pushd aubio-mingw32
git co develop
git pull
CFLAGS="-Os" CC=i586-mingw32msvc-gcc ./waf distclean configure build install \
  --destdir=../aubio-dist-mingw32 --testcmd="echo %s" \
  --with-target-platform=win32 --disable-avcodec --disable-samplerate \
  --disable-jack --disable-sndfile
popd

popd

# now build vamp-aubio-plugins
make -f Makefile.mingw32 clean all

#! /bin/sh

# cross compile vamp-aubio-plugins using mingw32 toolchain

echo "ERROR: This script does not produce a binary loadable by sonic visualiser."
echo "Comment the following line to run it anyway."
exit 1

. $PWD/VERSION
VAMP_AUBIO_VERSION=$VAMP_AUBIO_MAJOR_VERSION.$VAMP_AUBIO_MINOR_VERSION.$VAMP_AUBIO_PATCH_VERSION$VAMP_AUBIO_VERSION_STATUS

set -e
set -x

if [ "$1" = 'dist' ]
then
  rm -rf contrib/
fi

export CFLAGS="-Os"
#export CC="i586-mingw32msvc-gcc"
#export CXX="i586-mingw32msvc-g++"
export CC="x86_64-w64-mingw32-gcc"
export CXX="x86_64-w64-mingw32-g++"
export STRIP="x86_64-w64-mingw32-strip"
export WAFOPTS="--with-target-platform=win64 --disable-sndfile --disable-samplerate --disable-jack --disable-avcodec --notests"

# get waf
./scripts/get_waf.sh

# fetch Vamp SDK
./scripts/get_deps_mingw64.sh

# fetch and build aubio
./scripts/get_aubio.sh

# configure and build plugin
./waf configure

./waf build -v

# system-wide installation
#./waf install --destdir=dist-win

if [ "$1" = 'dist' ]
then
  DESTDIR=vamp-aubio-plugins-$VAMP_AUBIO_VERSION-win64
  rm -rf $DESTDIR $DESTDIR.zip
  mkdir $DESTDIR
  cp -prv contrib/aubio*/README.md $DESTDIR/README.aubio.md
  cp -prv README.md $DESTDIR
  cp -prv build/vamp-aubio.dll $DESTDIR
  $STRIP $DESTDIR/vamp-aubio.dll
  cp -prv vamp-aubio.cat vamp-aubio.n3 $DESTDIR
  zip -r $DESTDIR.zip $DESTDIR
fi

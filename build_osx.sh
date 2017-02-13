#! /bin/sh

# instructions to build vamp-aubio-plugins for mac os x

. $PWD/VERSION
VAMP_AUBIO_VERSION=$VAMP_AUBIO_MAJOR_VERSION.$VAMP_AUBIO_MINOR_VERSION.$VAMP_AUBIO_PATCH_VERSION$VAMP_AUBIO_VERSION_STATUS

if [ "$1" = 'dist' ]
then
  rm -rf contrib/
fi

# get waf
./scripts/get_waf.sh

# fetch Vamp SDK
./scripts/get_deps_osx.sh

# fetch and build aubio
./scripts/get_aubio.sh

# configure and build plugin
./waf configure

./waf build -v

# install in user's home directory
# ./waf install --destdir=$HOME

# system-wide installation
# sudo ./waf install

if [ "$1" = 'dist' ]
then
  ARCH=$(lscpu  | head -1  | awk '{print $2}')
  DESTDIR=vamp-aubio-plugins-$VAMP_AUBIO_VERSION-osx
  rm -rf $DESTDIR $DESTDIR.zip
  mkdir $DESTDIR
  cp -prv contrib/aubio*/README.md $DESTDIR/README.aubio.md
  cp -prv README.md $DESTDIR
  cp -prv build/vamp-aubio.dylib $DESTDIR
  strip $DESTDIR/vamp-aubio.dylib
  cp -prv vamp-aubio.cat vamp-aubio.n3 $DESTDIR
  tar jcvf $DESTDIR.tar.bz2 $DESTDIR
fi

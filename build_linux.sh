#! /bin/sh

# script to build vamp-aubio-plugins for linux

. $PWD/VERSION
VAMP_AUBIO_VERSION=$VAMP_AUBIO_MAJOR_VERSION.$VAMP_AUBIO_MINOR_VERSION.$VAMP_AUBIO_PATCH_VERSION$VAMP_AUBIO_VERSION_STATUS

if [ "$1" = 'dist' ]
then
  rm -rf contrib/
fi

# get waf
./scripts/get_waf.sh

# fetch Vamp SDK
./scripts/get_deps_linux.sh

# fetch and build aubio
./scripts/get_aubio.sh

# configure and build plugin
./waf configure

./waf build -v

# system-wide installation
# sudo ./waf install

if [ "$1" = 'dist' ]
then
  ARCH=$(lscpu  | head -1  | awk '{print $2}')
  DESTDIR=vamp-aubio-plugins-$VAMP_AUBIO_VERSION-$ARCH
  rm -rf $DESTDIR $DESTDIR.zip
  mkdir $DESTDIR
  cp -prv contrib/aubio*/README.md $DESTDIR/README.aubio.md
  cp -prv README.md $DESTDIR
  cp -prv build/vamp-aubio.so $DESTDIR
  strip $DESTDIR/vamp-aubio.so
  cp -prv vamp-aubio.cat vamp-aubio.n3 $DESTDIR
  tar jcvf $DESTDIR.tar.bz2 $DESTDIR
fi

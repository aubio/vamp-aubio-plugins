#! /bin/bash

# instructions to build vamp-aubio-plugins for mac os x

set -x
set -e

. $PWD/VERSION
VAMP_AUBIO_VERSION=$VAMP_AUBIO_MAJOR_VERSION.$VAMP_AUBIO_MINOR_VERSION.$VAMP_AUBIO_PATCH_VERSION$VAMP_AUBIO_VERSION_STATUS

if [ "$1" = 'dist' ]
then
  rm -rf contrib/
fi

# get waf
sh scripts/get_waf.sh

# fetch Vamp SDK
sh scripts/get_deps_msvc.sh

# stupid patch to get workaround quote escaping in git bash
patch -p1 < scripts/aubio_waf_msvc.patch

# fetch and build aubio
sh scripts/get_aubio.sh

# revert patch
patch -R -p1 < scripts/aubio_waf_msvc.patch

# configure and build plugin
python waf configure

python waf build -v

# install in user's home directory
# ./waf install --destdir=$HOME

# system-wide installation
# sudo ./waf install

#python waf install --destdir=dist

if [ "$1" = 'dist' ]
then
  DESTDIR=vamp-aubio-plugins-$VAMP_AUBIO_VERSION-win32
  rm -rf $DESTDIR $DESTDIR.zip
  mkdir $DESTDIR
  cp -prv contrib/aubio*/README.md $DESTDIR/README.aubio.md
  cp -prv README.md $DESTDIR
  cp -prv build/vamp-aubio.dll $DESTDIR
  cp -prv vamp-aubio.cat vamp-aubio.n3 $DESTDIR
  zip -r $DESTDIR.zip $DESTDIR
fi

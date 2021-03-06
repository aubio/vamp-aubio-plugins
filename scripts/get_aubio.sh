#! /bin/bash

set -e
set -x

source $PWD/VERSION

# external dependencies are not required for vamp
WAFOPTS="--disable-samplerate --disable-avcodec --disable-sndfile --disable-jack"

mkdir -p contrib
pushd contrib
if [ -n "$VAMP_AUBIO_VERSION_STATUS" ]
then
  git clone https://github.com/aubio/aubio aubio || ( pushd aubio; git pull; popd )
  pushd aubio
else
  echo "using aubio $AUBIO_VERSION"
  tarball=aubio-$AUBIO_VERSION.tar.bz2
  [ -f $tarball ] || curl -O https://aubio.org/pub/$tarball
  [ -f $tarball.asc ] || curl -O https://aubio.org/pub/$tarball.asc
  gpg --verify $tarball.asc $tarball
  rm -rf ${tarball%%.tar.bz2} && tar xf $tarball
  pushd ${tarball%%.tar.bz2}
fi

#rm -rf aubio
./scripts/get_waf.sh
#./waf distclean
./waf configure --prefix=$PWD/../aubio-dist $WAFOPTS
./waf build -v $WAFOPTS
./waf install -v $WAFOPTS
popd
popd

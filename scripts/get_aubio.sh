#! /bin/bash

set -e
set -x

mkdir -p contrib
pushd contrib
git clone https://github.com/aubio/aubio aubio || ( pushd aubio; git pull; popd )

#rm -rf aubio
pushd aubio
make getwaf
#./waf distclean
./waf configure --prefix=$PWD/../aubio-dist --disable-atlas
./waf build -v
./waf install -v
popd
popd

#! /bin/bash

set -e
set -x

which cl.exe && WAFOPTS='--msvc_version="msvc 12.0" --msvc_target="x86"'

mkdir -p contrib
pushd contrib
git clone https://github.com/aubio/aubio aubio || ( pushd aubio; git pull; popd )

#rm -rf aubio
pushd aubio
./scripts/get_waf.sh
#./waf distclean
./waf configure --prefix=$PWD/../aubio-dist --disable-atlas $WAFOPTS
./waf build -v
./waf install -v
popd
popd

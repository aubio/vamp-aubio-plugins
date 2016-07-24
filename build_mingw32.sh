#! /bin/sh

# cross compile vamp-aubio-plugins using mingw32 toolchain

set -e
set -x

export CFLAGS="-Os"
#export CC="i586-mingw32msvc-gcc"
#export CXX="i586-mingw32msvc-g++"
export CC="i686-w64-mingw32-gcc"
export CXX="i686-w64-mingw32-g++"
export WAFOPTS="--with-target-platform=win32 --disable-sndfile --disable-samplerate --disable-jack --disable-avcodec --notests"

# get waf
./scripts/get_waf.sh

# fetch Vamp SDK
./scripts/get_deps_mingw32.sh

# fetch and build aubio
./scripts/get_aubio.sh

# configure and build plugin
./waf configure

./waf build -v

# system-wide installation
# sudo ./waf install

#! /bin/sh

# instructions to build vamp-aubio-plugins for mac os x

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

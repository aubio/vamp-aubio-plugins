#! /bin/sh

# instructions to build vamp-aubio-plugins for linux

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

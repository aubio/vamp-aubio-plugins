#! /bin/sh

# instructions to build vamp-aubio-plugins for mac os x

# get waf
sh scripts/get_waf.sh

# fetch Vamp SDK
sh scripts/get_deps_msvc.sh

# fetch and build aubio
sh scripts/get_aubio.sh

# configure and build plugin
python waf configure

python waf build -v

# install in user's home directory
# ./waf install --destdir=$HOME

# system-wide installation
# sudo ./waf install

python waf install --destdir=dist

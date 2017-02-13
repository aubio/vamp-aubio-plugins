#! /bin/bash


source scripts/resources

#rm -rf contrib/
mkdir -p contrib/

pushd contrib

for p in $VAMP_PLUGIN_SDK #$VAMP_PLUGIN_SDK_MINGW
do
  fetch $p
done

pushd vamp-plugin-sdk-2.6
make -f build/Makefile.mingw64
popd

popd

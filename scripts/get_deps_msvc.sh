#! /bin/bash


source scripts/resources

#rm -rf contrib/
mkdir -p contrib/

pushd contrib

for p in $VAMP_PLUGIN_SDK $VAMP_PLUGIN_SDK_MSVC
do
  fetch $p
done

popd

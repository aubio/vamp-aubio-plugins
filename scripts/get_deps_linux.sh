#! /bin/bash


source scripts/resources

#rm -rf contrib/
mkdir -p contrib/

pushd contrib

for p in $VAMP_PLUGIN_SDK $VAMP_PLUGIN_SDK_LINUX32 $VAMP_PLUGIN_SDK_LINUX64
do
  fetch $p
done

popd

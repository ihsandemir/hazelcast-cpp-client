#!/usr/bin/env bash

function removeBuildFolders {
    rm -rf buildSTATIC64Release
    rm -rf buildSHARED32Release
    rm -rf buildSTATIC32Release
    rm -rf buildSHARED64Release
    rm -rf buildSTATIC64Release_SSL
    rm -rf buildSHARED32Release_SSL
    rm -rf buildSTATIC32Release_SSL
    rm -rf buildSHARED64Release_SSL
}

function cleanup {
    removeBuildFolders
}

trap cleanup EXIT

rm -rf ./cpp

mkdir -p ./cpp/Linux_32/hazelcast/include/hazelcast/
mkdir -p ./cpp/Linux_32/hazelcast/lib/tls
mkdir -p ./cpp/Linux_64/hazelcast/include/hazelcast/
mkdir -p ./cpp/Linux_64/hazelcast/lib/tls

cp -R hazelcast/include/hazelcast/* cpp/Linux_32/hazelcast/include/hazelcast/
cp -R hazelcast/generated-sources/include/hazelcast/* cpp/Linux_32/hazelcast/include/hazelcast/
cp -R hazelcast/include/hazelcast/* cpp/Linux_64/hazelcast/include/hazelcast/
cp -R hazelcast/generated-sources/include/hazelcast/* cpp/Linux_64/hazelcast/include/hazelcast/

scripts/build-linux.sh 64 STATIC Release COMPILE_WITHOUT_SSL &> STATIC_64_linux.txt &
STATIC_64_pid=$!

FAIL=0
wait ${STATIC_64_pid} || let "FAIL+=1"

if [ $FAIL -ne 0 ]; then
    echo "$FAIL builds FAILED!!!"
    exit $FAIL
fi

cp buildSTATIC64Release/libHazelcastClient* cpp/Linux_64/hazelcast/lib/

cp -R external/release_include cpp/external/include
mkdir -p cpp/examples
cp -r examples cpp/examples/src

echo "Linking to external libraries and examples for 32-bit release"
cd cpp/Linux_32
ln -s ../examples .
ln -s ../external .
cd -

echo "Linking to external libraries and examples for 64-bit release"
cd cpp/Linux_64
ln -s ../examples .
ln -s ../external .
cd -

# Verify release
#scripts/verifyReleaseLinux.sh


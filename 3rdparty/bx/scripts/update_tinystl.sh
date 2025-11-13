#!/bin/bash -eux

if [ $# != 1 ]; then
	echo "Usage: $0 <tinystl-upstream-folder>"
	exit 1
fi

SRC_DIR=$1
DST_DIR="include/tinystl"

pushd $(dirname $0)/..

cp $SRC_DIR/include/TINYSTL/*.h $DST_DIR/
find $DST_DIR -iname "*.h" -exec sed --in-place 's/<TINYSTL\//<tinystl\//g' {} \;

popd

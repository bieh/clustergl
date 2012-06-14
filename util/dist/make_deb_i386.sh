#!/bin/bash
CGL_RUNTIME="../../../runtime"
REVISION=`git shortlog | grep -E '^[ ]+\w+' | wc -l`

echo "Making i386 .deb package for r$REVISION"

cd ../../../

make clean
ARCH=-m32 make
cd util/dist

rm -rf files
mkdir files
cd files

cp -v $CGL_RUNTIME/cgl-render .
cp -v $CGL_RUNTIME/cgl-capture .
cp -v $CGL_RUNTIME/libcgl-capture.so .
cp -v $CGL_RUNTIME/cgl.conf .

#Build the package control file. TODO, this should be a template file
CONTROL_FILE="clustergl-r$REVISION"

echo "Package: clustergl" > $CONTROL_FILE
echo "Version: 0.$REVISION" >> $CONTROL_FILE
echo "Maintainer: Paul Hunkin <paul@bieh.net>" >> $CONTROL_FILE
echo "Architecture: i386" >> $CONTROL_FILE
echo "Files: cgl-render /usr/bin/" >> $CONTROL_FILE
echo " cgl-capture /usr/bin/" >> $CONTROL_FILE
echo " libcgl-capture.so /usr/lib/" >> $CONTROL_FILE
echo " cgl.conf /etc/" >> $CONTROL_FILE




equivs-build ./clustergl-r$REVISION > /dev/null

mv *.deb ../

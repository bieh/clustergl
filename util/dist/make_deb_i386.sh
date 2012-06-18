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
echo "Version: $REVISION-git" >> $CONTROL_FILE
echo "Maintainer: Paul Hunkin <paul@bieh.net>" >> $CONTROL_FILE
echo "Architecture: amd64" >> $CONTROL_FILE
echo "Depends: libsdl1.2 libsdl-net1.2 libglew liblzo2 libconfuse libzip"
echo "Files: cgl-render /usr/bin/cgl-render" >> $CONTROL_FILE
echo " cgl-capture /usr/bin/cgl-capture" >> $CONTROL_FILE
echo " libcgl-capture.so /usr/lib/libcgl-capture.so" >> $CONTROL_FILE
echo " cgl.conf /etc/cgl.conf" >> $CONTROL_FILE

equivs-build ./clustergl-r$REVISION > /dev/null

mv *.deb ../

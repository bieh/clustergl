#!/bin/bash
CGL_RUNTIME="../../../runtime"
REVISION=`git shortlog | grep -E '^[ ]+\w+' | wc -l`

echo "Making amd64 .deb package for r$REVISION"

cd ../../
#make clean
#make

#Debian package
sudo checkinstall -D -y\
	--pkgname=clustergl\
	--pkgversion=$REVISION-git\
	--pkgsource=src\
	--pakdir=util/dist\
	--maintainer=paul\@bieh.net\
	--requires="libsdl1.2debian, libsdl-net1.2, libglew1.6, liblzo2-2, libconfuse0, libzip1"\
	--nodoc\
	--install=no


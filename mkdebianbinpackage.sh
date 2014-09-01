#!/bin/bash

ORIG_DIR=`pwd`

DISTRO=ubuntu

function build_package {
	local PACKAGE_NAME=$1
	local DEB_DIR=$2

	local VER=`perl -e 'open FH, "'${DEB_DIR}'/changelog"; my $ver = <FH>; close FH; $ver =~ s/.*\((\d+\.\d+(?:\.\d+)?)[^)]*\).*/$1/; print $ver;'`
	local DIRNAME=${PACKAGE_NAME}_${VER}

	if [ ! -e .debian-package/${DIRNAME} ] ; then
		echo "build directory .debian-build/${DIRNAME} does not exist, please run mkdebiansrcpackage.sh first"
		exit 0
	fi
	
	cd .debian-package/${DIRNAME}
	
	echo building binary ${DIRNAME} package
	debuild -rfakeroot -k1753CC07
	
	cd $ORIG_DIR
}

if [ ! -e .debian-package ] ; then
	echo ".debian-package directory does not exist, please run mkdebiansrcpackage.sh first"
	exit 0
fi

build_package crystalpicnic ${DISTRO}

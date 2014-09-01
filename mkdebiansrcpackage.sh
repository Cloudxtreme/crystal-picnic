#!/bin/bash

#set -x

PACKAGE_NAME=crystalpicnic

function copy_dirs {

	for i in $*
	do
		echo copying ${i#${BASEDIR}/} to .debian-package/${DIRNAME}
		for j in `find ${i} -type f -a ! \( -iname "*.o" -o -iname "*.a" -o -path "*.svn*" -o -path "*build*" -prune -o -path "*xcode*" -prune \)`
		do
			DEST=${j#${BASEDIR}/}
			PERMS=0644
			if [ "$j" == "CrystalPicnic" ] ; then
				PERMS=0755
			fi
			if [ "$j" == "CrystalPicnicLauncher" ] ; then
				PERMS=0755
			fi
			install -D ${j} $DEST -m $PERMS
		done
	done

}

function do_package {
	local PACKAGE_NAME=$1
	local DEB_DIR=$2
	local DO_EXTRA=$3
	shift 3
	local DIRS="$@"

	local VER=`perl -e 'open FH, "../'${DEB_DIR}'/changelog"; my $ver = <FH>; close FH; $ver =~ s/.*\((\d+\.\d+(?:\.\d+)?)[^)]*\).*/$1/; print $ver;'`
	local DIRNAME=${PACKAGE_NAME}_${VER}
	local TARBALL_NAME="${DIRNAME}.orig.tar.bz2"

	mkdir ${DIRNAME}
	cd ${DIRNAME}

	copy_dirs $DIRS

	if [ "$DO_EXTRA" == "1" ] ; then
		echo Resizing crystalpicnic.png
		convert ${BASEDIR}/icon256.png -filter Point -resize 32x32 crystalpicnic.png

		echo Copying crystalpicnic.desktop
		cp -a ${BASEDIR}/crystalpicnic.desktop crystalpicnic.desktop

		echo Creating directories
		mkdir -p i386
		mkdir -p amd64

		echo Copying programs and shared libraries
		cp -a ${BASEDIR}/linux/i386/* i386
		cp -a ${BASEDIR}/linux/amd64/* amd64

		echo Copying data
		cp ${BASEDIR}/linux/data.cpa .
		cp ${BASEDIR}/linux/FamilyLicense.txt .
		cp ${BASEDIR}/linux/ReadMe.txt .
	fi
	
	if [ ! -f ../${TARBALL_NAME} ] ; then
		cd ..
		echo Building .debian-package/${TARBALL_NAME}
		tar cjf ${TARBALL_NAME} ${DIRNAME}
		cd ${DIRNAME}
	else
		echo Not building .debian-package/${TARBALL_NAME}, it exists
	fi

	copy_dirs ${BASEDIR}/${DEB_DIR}
	if [ "$DEB_DIR" != "debian" ] ; then
		echo renaming ${DEB_DIR} to debian
		mv ${DEB_DIR} debian
	fi
	
	echo build source package
	debuild ${DEBUILD_ARGS} -k1753CC07
	cd ..
}

ORIG_DIR=`pwd`
KEEP_ORIG=0

DEBUILD_ARGS="-S"
DISTRO=ubuntu

while getopts “hkfd:” OPTION
do
	case "$OPTION" in
		d)	DISTRO=$OPTARG
				;;
		k)	KEEP_ORIG=1
			;;
		f)	DEBUILD_ARGS="$DEBUILD_ARGS -sa"
			;;
		:)	echo "option $OPTION requires an argument.";
			exit
			;;
		h)	echo "Usage: $0 [-k] [-h] -d <distro-name>"
			echo "  -d  pick which distro you want to build for"
			echo "        options are debian and ubuntu, default of ubuntu"
			echo "  -k  force this script to /keep/ any existing orig tarball."
			echo "        this is useful to make package only updates"
			echo "  -f  force full upload of orig tarball"
			echo "        normally debuild will skip uploading the orig tarball if it doesn't have to"
			echo "  -h  show this information"
			exit 0
			;;
	esac
done

ORIG_APP_VER=`perl -e 'open FH, "'${DISTRO}'/changelog"; my $ver = <FH>; close FH; $ver =~ s/.*\((\d+\.\d+(?:\.\d+)?)[^)]*\).*/$1/; print $ver;'`
ORIG_TARBALL_NAME="${PACKAGE_NAME}_${ORIG_APP_VER}.orig.tar.bz2"

echo "ORIG_TARBALL_NAME: $ORIG_TARBALL_NAME"

if [ "$KEEP_ORIG" == "1" ] && [ -f .debian-package/${ORIG_TARBALL_NAME} ] ; then
	echo moving .debian-package/${ORIG_TARBALL_NAME} out of the way
	mv .debian-package/${ORIG_TARBALL_NAME} ${ORIG_TARBALL_NAME}
fi

if [ -e .debian-package ] ; then
	echo removing existing .debian-package directory
	rm -fr .debian-package
fi

mkdir .debian-package

if [ "$KEEP_ORIG" == "1" ] && [ -f ${ORIG_TARBALL_NAME} ] ; then
	echo moving ${ORIG_TARBALL_NAME} back to .debian-package
	mv ${ORIG_TARBALL_NAME} .debian-package/${ORIG_TARBALL_NAME}
fi

cd .debian-package

BASEDIR=../../../crystalpicnic



do_package ${PACKAGE_NAME} ${DISTRO} 1

cd $ORIG_DIR



set +x

#!/bin/bash -x
SUBDIRS="orbisdev-utils orbisdev-libkernelUtil orbisdev-libdebugnet orbisdev-libz orbisdev-liborbisPad orbisdev-liborbisAudio orbisdev-liborbisNfs orbisdev-liborbisKeyboard orbisdev-libmod orbisdev-libSQLite orbisdev-liborbisLink"
PROC_NR=$(getconf _NPROCESSORS_ONLN)


## Fetch the main headers
git clone https://github.com/orbisdev/orbisdev-headers --depth=1
cp -a orbisdev-headers/include/* $ORBISDEV/usr/include/

## Download the source code.
for dir in $SUBDIRS
do 
	echo $dir
	if test ! -d "$dir/.git"; then 
		git clone https://github.com/orbisdev/$dir && cd $dir || exit 1 
	else
		cd $dir && 
			git pull && git fetch origin &&
			git reset --hard origin/master || exit 1
	fi
	make --quiet -j $PROC_NR clean || { exit 1; }
	make --quiet -j $PROC_NR || { exit 1; }
	make --quiet -j $PROC_NR install || { exit 1; }
	make --quiet -j $PROC_NR clean || { exit 1; } 
	cd ..
done


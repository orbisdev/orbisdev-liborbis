#!/bin/bash -x
SUBDIRS="orbisdev-libkernelUtil orbisdev-libdebugnet orbisdev-libz orbisdev-liborbisPad orbisdev-liborbisAudio orbisdev-libmod orbisdev-liborbisKeyboard orbisdev-liborbisNfs orbisdev-libSQLite orbisdev-liborbisLink"

## Download the source code.
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
	cd ..
done
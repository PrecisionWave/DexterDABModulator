#!/bin/bash

for ip_dir in `dirname */ip/*.zip` ; do
	echo "Processing $ip_dir"
	pushd $ip_dir
	if [ -f *.zip ]; then
		rm -rf *
		git checkout .
		unzip *.zip
	fi
	popd
done


#!/bin/bash

function build_sketch()
{
	local sketch=$1

	# buld sketch with arudino ide
	echo -e "\n Build $sketch \n"
	arduino --verbose --verify $sketch

	# get build result from arduino
	local re=$?

	# check result
	if [ $re -ne 0 ]; then
		echo "Failed to build $sketch"
		return $re
	fi
}

# download and unpack arduino ide
function get_arduino()
{
	# arduino version
	local a_ver="1.6.6-linux64-gitd215780"
	local a_arch="arduino-"$a_ver".tar.xz"

	# download
	wget "http://gollordasilva.de/travis/"$a_arch
	tar xf $a_arch
}

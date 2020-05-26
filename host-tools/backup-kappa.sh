#!/bin/sh

echo "This tool is meant for backing up Layer 95 and the Kappa layer on an already consoleOS-formatted storage medium"

echo "What device do you want to backup?"
read device

if ls $device 2> /dev/null; then
	:
else
	echo "$device does not exist, exitting ..."
	exit 1
fi

echo "Unmounting Layer 95 and Kappa layer ..."
sudo umount "$device"1
sudo umount "$device"2

set -e

echo "Backing up Layer 95 to /tmp/layer95.img ..."
sudo dd if="$device"1 of=/tmp/layer95.img bs=4M && sync

echo "Backing up Kappa layer to /tmp/kappa.img ..."
sudo dd if="$device"2 of=/tmp/kappa.img bs=4M && sync

echo "Finished backing everything up successfully"
exit 0

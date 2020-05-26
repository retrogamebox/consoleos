#!/bin/sh

echo "This tool is meant for preparing a storage medium for consoleOS installation"
echo "It expects /tmp/layer95.img and /tmp/kappa.img to be provided"
echo "You can generate these files from a pre-existing consoleOS installation by running the backup-kappa.sh script"

echo "What device do you want to format?"
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

echo "Zeroing out disk ..."
sudo dd if=/dev/zero of=$device bs=4M && sync

echo "Creating Layer 95 at $device"1" ..."
sed -e 's/\s*\([\+0-9a-zA-Z]*\).*/\1/' << EOF | fdisk $device
	n # create new partition
	p # what type? (primary)
	1 # what partition?
	8192 # first sector? (4096 * 2 = 8192)
	270335 # last sector? (8192 + 128 * 1024 * 2 - 1 = 7217151)
	w # write changes to disk
EOF

echo "Copying Layer 95 image to $device"1" ..."
sudo dd if=/tmp/layer95.img of="$device"1 bs=4M && sync

echo "Setting Layer 95 label ..."
sudo fatlabel "$device"1 LAYER95

echo "Making Layer 95 bootable ..."
sed -e 's/\s*\([\+0-9a-zA-Z]*\).*/\1/' << EOF | fdisk $device
	t # set partition type
	1 # what partition?
	c # what code? (0x0C, W95 FAT32 (LBA))
	w # write changes to disk
EOF

echo "Creating Kappa layer at $device"2" ..."
sed -e 's/\s*\([\+0-9a-zA-Z]*\).*/\1/' << EOF | fdisk $device
	n # create new partition
	p # what type? (primary)
	2 # what partition?
	270336 # first sector? (270335 + 1 = 270336)
	6561791 # last sector? (270336 + 3 * 1024 * 1024 * 2 - 1 = 6561791)
	w # write changes to disk
EOF

echo "Copying Kappa layer image to $device"2" ..."
sudo dd if=/tmp/kappa.img of="$device"2 bs=4M && sync

echo "Setting Kappa layer label ..."
sudo e2label "$device"2 KAPPA

echo "Creating Layer 3's extended partition at $device"3" ..."
sed -e 's/\s*\([\+0-9a-zA-Z]*\).*/\1/' << EOF | fdisk $device
	n # create new partition
	e # what type? (extended)
	3 # what partition?
	6561792 # first sector? (6561791 + 1 = 6561792)
	7217151 # last sector? (6561792 + 320 * 1024 * 2 - 1 = 7217151)
	w # write changes to disk
EOF

echo "Creating Sigma layer ..."
sed -e 's/\s*\([\+0-9a-zA-Z]*\).*/\1/' << EOF | fdisk /dev/mmcblk0
	n # create new partition
	l # what type? (logical)
	6563840 # first sector? (6561792 + 1024 * 2 = 6563840)
	6629375 # last sector? (6563840 + 32 * 1024 * 2 - 1 = 6629375)
	w # write changes to disk
EOF

echo "Creating ext4 file system on Sigma layer ($device"5") ..."
sudo mkfs -t ext4 "$device"5

echo "Setting Sigma layer label ..."
sudo e2label "$device"5 SIGMA

echo "Creating Omega layer ..."
sed -e 's/\s*\([\+0-9a-zA-Z]*\).*/\1/' << EOF | fdisk /dev/mmcblk0
	n # create new partition
	l # what type? (logical)
	6631424 # first sector? (6629375 + 1 * 1024 * 2 + 1 = 6631424)
	6696959 # last sector? (6631424 + 32 * 1024 * 2 - 1 = 6696959)
	w # write changes to disk
EOF

echo "Creating ext4 file system on Omega layer ($device"6") ..."
sudo mkfs -t ext4 "$device"6

echo "Setting Omega layer label ..."
sudo e2label "$device"6 OMEGA

echo "Creating Alpha layer ..."
sed -e 's/\s*\([\+0-9a-zA-Z]*\).*/\1/' << EOF | fdisk /dev/mmcblk0
	n # create new partition
	l # what type? (logical)
	6699008 # first sector? (6696959 + 1 * 1024 * 2 + 1 = 6699008)
	7217151 # last sector? (6699008 + 253 * 1024 * 2 - 1 = 7217151)
	w # write changes to disk
EOF

echo "Creating ext4 file system on Alpha layer ($device"7") ..."
sudo mkfs -t ext4 "$device"7

echo "Setting Alpha layer label ..."
sudo e2label "$device"7 ALPHA

echo "Creating Delta layer ..."
sed -e 's/\s*\([\+0-9a-zA-Z]*\).*/\1/' << EOF | fdisk /dev/mmcblk0
	n # create new partition
	p # what type? (primary)
	7217152 # first sector? (7217151 + 1 = 7217152)
	# last sector? (default)
	w # write changes to disk
EOF

echo "Creating ext4 file system on Delta layer ($device"4") ..."
sudo mkfs -t ext4 "$device"4

echo "Setting Delta layer label ..."
sudo e2label "$device"4 DELTA

echo "Finished preparing everything successfully"
exit 0

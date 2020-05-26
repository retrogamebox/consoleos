#!/bin/sh

echo "This tool is meant for installing consoleOS on a storage medium"
echo "It expects said medium to have already been prepared correctly"
echo "You can prepare a medium for installation by running the prepare-medium.sh script from this folder"

echo "What device do you want to install consoleOS to?"
read device

if ls $device 2> /dev/null; then
	:
else
	echo "$device does not exist, exitting ..."
	exit 1
fi

set -e

echo "Mounting Layer 95 and Kappa layer ..."

mkdir -p /tmp/layer95
sudo mount "$device"1 /tmp/layer95

mkdir -p /tmp/kappa
sudo mount "$device"2 /tmp/kappa

echo "Getting Layer 95 partition UUID ..."
LAYER95_PARTUUID=`sudo blkid | grep "$device"1 | grep -o -P '(?<=PARTUUID\=").*(?=")'`
echo "Layer 95's partition UUID is $LAYER95_PARTUUID"

echo "Getting Kappa layer partition UUID ..."
KAPPA_PARTUUID=`sudo blkid | grep "$device"2 | grep -o -P '(?<=PARTUUID\=").*(?=")'`
echo "Kappa layer's partition UUID is $KAPPA_PARTUUID"

echo "Creating cmdline.txt file on Layer 95 ..."
sudo echo "dwc_otg.lpm_enable=0 console=serial0,115200 console=tty1 root=PARTUUID=$KAPPA_PARTUUID rootfstype=ext4 elevator=deadline fsck.repair=yes rootwait fastboot noswap quiet logo.nologo vt.global_cursor_default=0" | sudo tee /tmp/layer95/cmdline.txt

echo "Creating config.txt file on Layer 95 ..."
sudo cp ../installation-files/config.txt /tmp/layer95/config.txt

echo "Creating /etc/fstab file on Kappa layer ..."
sudo echo "proc /proc proc defaults 0 0" | sudo tee /tmp/kappa/etc/fstab
sudo echo "PARTUUID=$LAYER95_PARTUUID /boot vfat defaults 0 2" | sudo tee -a /tmp/kappa/etc/fstab
sudo echo "PARTUUID=$KAPPA_PARTUUID / ext4 defaults,noatime 0 1" | sudo tee -a /tmp/kappa/etc/fstab

echo "Copying the install.sh file to Kappa layer ..."
sudo cp ../installation-files/install.sh ~/tmp/kappa/home/pi/install.sh

echo "Unmounting Layer 95 and Kappa layer ..."

sudo umount /tmp/layer95
sudo umount /tmp/kappa

echo "Finished installing everything successfully"
exit 0

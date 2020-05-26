#!/bin/sh

echo "Installing consoleOS ..."

echo "Making sure in user's home directory ..."
cd

# install packages

echo "Updating packages ..."
sudo apt-get update -y

echo "Installing packages ..."
sudo apt-get -y install git

# setup services

echo "Compiling MiniGL library ..."
gcc -c consoleos/files/minigl/minigl.c -o consoleos/files/minigl/minigl.o -I/opt/vc/include

echo "Compiling services ..."

gcc -c consoleos/files/safe-unplug-splash/main.c -o consoleos/files/safe-unplug-splash/main.o -Iconsoleos/files/minigl
gcc consoleos/files/minigl/minigl.o consoleos/files/safe-unplug-splash/main.o -o consoleos/files/safe-unplug-splash/main -L/opt/vc/lib -lbrcmEGL -lbrcmGLESv2 -lbcm_host -lm

echo "Copying service files to systemd ..."
sudo cp consoleos/installation-files/systemd-services/* /lib/systemd/system

echo "Enabling services ..."
sudo systemctl enable boot-splash.service
sudo systemctl enable shutdown-splash.service
sudo systemctl enable safe-unplug-splash.service

# setup aqua

echo "Installing AQUA environment ..."
git clone https://github.com/inobulles/aqua-linux

echo "Downloading latest consoleOS ROM ..."
wget https://files.retrogamebox.be/packages/consoleos/stable/latest.zpk -O aqua-linux/package.zpk

echo "Compiling AQUA ..."
( cd aqua-linux && sh build.sh update kos broadcom )

# login and hostname stuff

echo "Setting hostname ..."
sudo sed -i "s/raspberrypi/retrogamebox/g" /etc/hostname /etc/hosts

echo "Enabling auto-login ..."
sudo raspi-config nonint do_boot_behaviour B2

# not sure this does what i expect
#~ echo "Disabling boot console ..."
#~ sudo raspi-config nonint do_serial 1

echo "Adding options for fastboot, disabling swap, and silencing the boot console to cmdline.txt ..."
echo "console=tty3 fastboot noswap quiet logo.no-logo vt.global_cursor_default=0" | sudo tee -a /boot/cmdline.txt

# setup layers and file system

echo "Creating Layer 3's extended partition at /dev/mmcblk0p3 ..."
sed -e 's/\s*\([\+0-9a-zA-Z]*\).*/\1/' << EOF | fdisk /dev/mmcblk0
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

echo "Creating ext4 file system on Sigma layer (/dev/mmcblk0p5) ..."
sudo mkfs -t ext4 /dev/mmcblk0p5

echo "Setting Sigma layer label ..."
sudo e2label /dev/mmcblk0p5 SIGMA

echo "Creating Omega layer ..."
sed -e 's/\s*\([\+0-9a-zA-Z]*\).*/\1/' << EOF | fdisk /dev/mmcblk0
	n # create new partition
	l # what type? (logical)
	6631424 # first sector? (6629375 + 1 * 1024 * 2 + 1 = 6631424)
	6696959 # last sector? (6631424 + 32 * 1024 * 2 - 1 = 6696959)
	w # write changes to disk
EOF

echo "Creating ext4 file system on Omega layer (/dev/mmcblk0p6) ..."
sudo mkfs -t ext4 /dev/mmcblk0p6

echo "Setting Omega layer label ..."
sudo e2label /dev/mmcblk0p6 OMEGA

echo "Creating Alpha layer ..."
sed -e 's/\s*\([\+0-9a-zA-Z]*\).*/\1/' << EOF | fdisk /dev/mmcblk0
	n # create new partition
	l # what type? (logical)
	6699008 # first sector? (6696959 + 1 * 1024 * 2 + 1 = 6699008)
	7217151 # last sector? (6699008 + 253 * 1024 * 2 - 1 = 7217151)
	w # write changes to disk
EOF

echo "Creating ext4 file system on Alpha layer (/dev/mmcblk0p7) ..."
sudo mkfs -t ext4 /dev/mmcblk0p7

echo "Setting Alpha layer label ..."
sudo e2label /dev/mmcblk0p7 ALPHA

echo "Creating Delta layer ..."
sed -e 's/\s*\([\+0-9a-zA-Z]*\).*/\1/' << EOF | fdisk /dev/mmcblk0
	n # create new partition
	l # what type? (logical)
	7217152 # first sector? (7217151 + 1 = 7217152)
	# last sector? (default)
	w # write changes to disk
EOF

echo "Creating ext4 file system on Delta layer (/dev/mmcblk0p4) ..."
sudo mkfs -t ext4 /dev/mmcblk0p4

echo "Setting Delta layer label ..."
sudo e2label /dev/mmcblk0p4 DELTA

echo "Creating wpa.ocon in Omega layer and symbolically linking it to /etc/wpa_supplicant.conf ..."
sudo mv /etc/wpa_supplicant.conf /omega/wpa.conf
sudo ln -s /omega/wpa.conf /etc/wpa_supplicant.conf

echo "Deleting installation file ..."
sudo rm -f ~/install.sh

echo "Rebooting ..."
sudo reboot

exit 0

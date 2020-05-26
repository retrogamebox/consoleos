# Overview
Folder containing all the files needed for consoleOS on Raspbian Buster Lite.

# Prerequisites
This tool assumes you have a fresh Raspbian Buster Lite installation on a (minimum) 6 GiB block storage medium (e.g. an SD card).
It also assumes a 128 MiB FAT32 boot partition starting 4 MiB from the start of the disk directly followed by a 3 GiB ext4-formatted partition containing your Linux installation (the Kappa layer) (documentation pertaining to all this can be found in the `documentation/` directory).

You can format your disk correctly by plugging your block device (assumed to be located at `/dev/sdb`) into an external computer and executing the `prepare-medium.sh` script from the `host-tools/` directory.

# Installing
To install consoleOS on the previously stated block device, simply run the `install.sh` script from the `host-tools/` directory, and next time you boot, the Delta Govenor service will kick in and install everything you need (libraries, backup ROMs, virtual machines, &c) to the Kappa layer and lock it.
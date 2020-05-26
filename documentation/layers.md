The consoleOS disk is divided into multiple 4MiB-aligned partitions (or "layers", in consoleOS' terminology).
Here is a list of how all these layers ought to be layed out, and what purposes they serve:

# Layer 95
(format: fat32 lba, size: 256 MiB, mount point: /boot, read only)
Contains binary blobs to be passed on to the VPU and general boot information for setting up the system clocks and such. Also contains details on the manufacturer of the console and what it's default configuration was like.

# Kappa layer
(format: ext4, size: 3 GiB, mount point: /, read only)
Contains the Linux kernel, core system components and drivers, and everything needed to mend other layers in case of disk corruption. This layer also contains the code for keeping the ZVM and other parts of the system up to date aswell as consoleOS backups.

# Layer 3
(format: extended partition, size: 320 MiB)
Contains the Sigma and Omega layers

## Sigma layer
(format: ext4, size: 32 MiB, mount point: /sigma, full read/write)
Contains journals and temporary system debug logs.

## Omega layer
(format: ext4, size: 32 MiB, mount point: /omega, restricted write)
Contains system preferences, specifically known WiFi networks and their encrypted keys.

## Alpha layer
(format: ext4, size: 253 MiB, mount point: /alpha, restricted write)
Contains AQUA ZVM, KOS, and consoleOS ROM's.

# Delta layer
(format: ext4, size: remaining disk space, mount point: /delta, restricted write)
Contains all installed applications and user save data.

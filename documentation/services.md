consoleOS has a few services to be handled by systemd.
Here is a list of all these services, and what they do:

# Safe unplug (safe-unplug.service)

Display a message to the user telling them the device is safe to be unplugged after all the disks have been made read-only, and the system has finished its shutdown process.

# Delta Govenor (delta-govenor.service)

Check all the layers for integrity, get into the Alpha layer, and boot consoleOS as fast as possible.
If any corruption is found in any of the layers, attempt to mend it, and if all else fails, boot into the backup ROMs of consoleOS and ask the user to reconfigure or inform them that some data may be missing.
Run as soon as the Kappa layer is entered and up and running and Layer 95 has initialized the VPU (and subsequently, the GPU).
Keep running in the background to handle things like update detection, the SSH server, requests from retrogamebox.be, crash reporting, &c.

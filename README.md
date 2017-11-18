# pocketchip-batt

A lighter replacement for the `pocketchip-batt` script on PocketCHIP.

The original one was running a relatively heavy shell script (`/usr/sbin/battery.sh`) outputting many 
battery-related parameters in order to grep only 2 (voltage and charging status) and put them into 
two files that were later used by other scripts and even the home screen program.

This would not be an issue on a desktop system but here it was running often enough to cause audio to stutter 
in some programs, for example, VisualBoyAdvance.

This little C program replaces the `pocketchip-batt` shell script, so the normal battery warnings 
functionality is still available with lesser penalty to the CPU resources. It also replaces 3 related systemd services
(`pocketchip-off00`, `pocketchip-war05` and `pocketchip-warn15`) that were checking the battery levels and calling 
those power notification scritps.

## Installation

Make sure you have git and gcc installed, then clone the repo and build/install it: 

	git clone https://github.com/aleh/pocketchip-batt.git
	cd pocketchip-batt
	sudo make install

Please reboot the device just in case. Enjoy!

## Removal

If you want to undo the changes:

	sudo make uninstall

---

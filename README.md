# pocketchip-batt

A lighter replacement for the `pocketchip-batt` script on PocketCHIP.

The original one was running a relatively heavy shell script (`/usr/sbin/battery.sh`) outputting many 
battery-related parameters in order to capture only 2 (voltage and charging status) and put them into 
two files that were later used by other scripts and even the home screen program.

This would not be an issue on a desktop system but here it was running often enough to cause audio to stutter 
in some programs, for example, VisualBoyAdvance.

This little C program replaces both `battery.sh` and `pocketchip-batt` shell scripts, so the normal battery warnings 
functionality is still available with lesser penalty to the CPU resources. It also replaces 3 related systemd services:
`pocketchip-off00`, `pocketchip-war05` and `pocketchip-warn15`.

To install:

	sudo make install

(The above assumes that you have gcc installed on your system, try `sudo apt-get install build-essential` if you don't).

If you want to undo the changes:

	sudo make uninstall

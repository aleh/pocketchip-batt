# pocketchip-batt

A lighter replacement for the `pocketchip-batt` script on PocketCHIP 4.3 (4.4 has additional services interfering with audio playback).

The original one was running a relatively heavy shell script (`/usr/sbin/battery.sh`) outputting many 
battery-related parameters in order to grep only 2 (voltage and charging status) and put them into 
two files that were later used by other scripts and even the home screen program.

This would not be an issue on a desktop system but here it was running often enough to cause audio to stutter 
in some programs, for example, VisualBoyAdvance.

This little C program replaces the `pocketchip-batt` shell script, so the normal battery warnings 
functionality is still available with lesser penalty to the CPU resources. It also replaces 3 related systemd services
(`pocketchip-off00`, `pocketchip-war05` and `pocketchip-warn15`) that were checking the battery levels and calling 
those power notification scritps.

In addition to that the newer version of the script replaces the functionality of the `pocketchip-load` service, which was responsible for proper key mapping on startup and display backlight deactivation when the system turns the display off.  

## Installation

Make sure you have git and gcc installed, then clone the repo and build/install it: 

	git clone https://github.com/aleh/pocketchip-batt.git
	cd pocketchip-batt
	sudo make install

Please reboot the device just in case. Enjoy!

## Removal

If you want to undo the changes:

	sudo make uninstall

## TODO

- `pocket-home` appears to be constantly consuming up to 1-4% of CPU, must be polling something fairly hard, would be great to patch it;
- `ubihealthd` seems to be causing issues on 4.4;
- we should patch `rsyslogd.conf` with our Makefile as described here: [https://www.raspberrypi.org/forums/viewtopic.php?f=63&t=134971#p898539];
- the emulator itself can be improved to sync audio a bit nicer, here is my attempt: [https://github.com/aleh/VisualBoyAdvance]. 
---

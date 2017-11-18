# pocket-batt

A lighter replacement for the `pocket-batt` script on PocketCHIP.

The original script was running a relatively heavy shell script (`/usr/sbin/battery.sh`) outputting many 
battery-related parameters in order to get only the voltage and charging status of the battery and put them into 
two files that were later used by other scripts and even the home screen program.

This would not be an issue if it did not run often enough to cause audio to stutter in some programs.

This little C program replaces both `battery.sh` and `pocket-batt` shell scripts, so the normal battery warnings 
functionality is still available with lesser penalty to the CPU resources.

To install:

	sudo make install

The above assumes that you have gcc installed on your system.

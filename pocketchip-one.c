/*
 * A lighter replacement for pocketchip-batt script in PocketCHIP. See the README.md.
 * Copyright (C) Aleh Dzenisiuk, 2017. http://github.com/aleh/pocketchip-batt
 */

#include <stdio.h>
#include <stdint.h>
#include <errno.h>

#include <linux/i2c-dev.h>
#include <fcntl.h>

#include <X11/Xlib.h>
#include <X11/extensions/dpms.h>

static int _i2c_device = 0;

static void _battery_close() {
	
	if (_i2c_device) {
		close(_i2c_device);
		_i2c_device = 0;
	}
}

static int _battery_open() {

	if ((_i2c_device = open("/dev/i2c-0", O_RDWR)) < 0) {
		perror("Could not open the I2C device");
		return 1;	
	}

	if (ioctl(_i2c_device, I2C_SLAVE_FORCE, 0x34) < 0) {
		perror("Could not set the slave address");
		_battery_close();
		return 2;
	}

	return 0;
}

static int _i2c_write(const uint8_t *buf, int len) {
	if (write(_i2c_device, buf, len) != len) {
		return 0;
	}
	return 1;
}

static int _i2c_read_reg(uint8_t reg, uint8_t *buf, int len) {
	if (write(_i2c_device, &reg, 1) != 1) {
		return 0;
	}
	if (read(_i2c_device, buf, len) != len) {
		return 0;
	}
	return 1;
}

static int battery_read(int *charging, int *voltage, int *gauge) {

	int result = -1;
	do {

		if (_battery_open() != 0)
			break;
		
		uint8_t buf[8];
		if (!_i2c_read_reg(0x01, buf, 1)) {
			perror("Could not read charging status");
			break;
		}
		*charging = (buf[0] >> 6) & 1; 
	
		if (!_i2c_read_reg(0xB9, buf + 2, 1)) {
			perror("Could not read fuel gauge");
			break;
		}
		*gauge = buf[2];
		
		if (0) {

			// There is little sense in reading the actual voltage, we are not doing it anymore, see below.
			//  The old code here is for history/experiments.

			// The comment in the original battery.sh for this was "force ADC enable for battery voltage and current".
			if (!_i2c_write((const uint8_t[]){ 0x82, 0xC3 }, 2)) {
				perror("Could not enable the ADC");
				break;
			}

			if (!_i2c_read_reg(0x78, buf, 1)) {
				perror("Could not read the voltage (MSB)");
				break;
			}
			
			if (!_i2c_read_reg(0x79, buf + 1, 1)) {
				perror("Could not read the voltage (LSB)");
				break;
			}
			*voltage = (((uint16_t)buf[0] << 4) | (buf[1] & 0x0F)) * 1.1;
		
		} else {

			// So, instead of reading the voltage and then letting pocket-home (incorrectly) convert it to battery level, 
			// let's set the voltage based on the fuel gauge we've just read above.
			// These are the values for 0% and 100% that pocket-home uses, let's map our fuel gauge into this range.
			const int max_voltage = 4250;
			const int min_voltage = 3275;
			*voltage = min_voltage + (max_voltage - min_voltage) * (*gauge) / 100;
		}

		result = 0;

	} while (0);

	_battery_close();	

	return result;
}

static int write_battery_file(const char *name, int value) {

	FILE *f = fopen(name, "w");
	if (f == NULL) {
		fprintf(stderr, "Could not write to %s: %s\n", name, strerror(errno));
		return errno;
	} else {
		fprintf(f, "%d\n", value);
		fclose(f);
		return 0;
	}
}

static int has_fs_flag(const char *name) {

	if (access(name, F_OK) == 0) {
		return 1;
	}
	int f = open(name, O_CREAT | O_WRONLY);
	if (f == 0) {
		perror("Could not create a flag file");
	} else {
		close(f);
	}
	return 0;
}

static void remove_fs_flag(const char *name) {
	unlink(name);
}

static int shell(const char *cmd) {
	int status = system(cmd);
	if (status != 0) {
		fprintf(stderr, "Shell command failed (%d): '%s'\n", status, cmd);
	}
	return status;
}

static void check_battery() {

	seteuid(0);

	int charging = 0, voltage = 0, gauge = 0;
	if (battery_read(&charging, &voltage, &gauge) != 0)
		return;

	// Not printing these params anymore as instead of replacing the battery.sh I've decided to replace 
	// pocketchip-batt and extra output would just cause more logging by systemd.
	/*~
	// Let's print to stdout just in case this script is used instead of battery.sh .
	printf("CHARG_IND=%d\n", charging);
	printf("Battery voltage = %.0fmV\n", voltage);
	*/

	//
 	// And this is what the pocket-batt actually does (except for averaging).
	//

	// We want the directory /usr/lib/pocketchip-batt to actually be on a temporary FS,
	// so making a directory in /run and there will be a symlink pointing there created via a makefile.
	mkdir("/run/pocketchip-batt", 0777);

	// Charging status.
	if (write_battery_file("/usr/lib/pocketchip-batt/charging", charging) != 0) {
		perror("Could not update charging file");
	}

	// Voltage.
	// The original script was averaging the voltage as well, but I don't want to bother.
	if (write_battery_file("/usr/lib/pocketchip-batt/voltage", voltage) != 0) {
		perror("Could not update the voltage file");
	}

	// Something new here, the fuel gauge. 
	// Calculating the battery level using voltage (as it is currently done) is not very good idea, 
	// especially the assumption of linear dependency of the capacity on voltage.
	// The battery controller already does the job for us, so let's use it.
	if (write_battery_file("/usr/lib/pocketchip-batt/gauge", gauge) != 0) {
		perror("Could not update the voltage file");
	}

	//
	// And it looks like it is not hard to do what the rest of the scripts were doing
	//

	// Below we were comparing voltages: 3300 for off00, 3375 for warn05 (3400 to reset it), 3500 for warn15 (3550 to reset it), but it's much better to use the fuel gauge, let's use it.
	
	// I am not sure who else is using these flag files, otherwise would put them to /run or just had them here as we are a daemon anyway.
	const char warn05_flag[] = "/home/chip/.pocketchip-batt/warn05";
	const char warn15_flag[] = "/home/chip/.pocketchip-batt/warn15";
	if (!charging) {
		mkdir("/home/chip/.pocketchip-batt", 0777);	
		if (gauge <= 1) {
			// pocketchip-off00
			shell("sudo -u chip DISPLAY=:0 pocket-off");
		} else if (gauge <= 5) {
			// pocketchip-warn05
			if (!has_fs_flag(warn05_flag)) {
				shell("sudo -u chip DISPLAY=:0 pocket-exit-5");
			}
		} else if (gauge <= 15) {
			// pocketchip-warn15
			if (!has_fs_flag(warn15_flag)) {
				shell("sudo -u chip DISPLAY=:0 pocket-exit-15");
			}
		}
	}
	// pocketchip-warn05
	if (charging || gauge > 7) {
		remove_fs_flag(warn05_flag);
	}
	// pocketchip-warn15
	if (charging || gauge > 20) {
		remove_fs_flag(warn15_flag);
	}
}

static void init_xmodmap_if_needed() {

	static int initialized = 0;

	// Well, let's do this 3 times just in case. Does not work once for some reason, perhaps too early.
	if (initialized >= 3)
		return;

	seteuid(1000);
	if (shell("DISPLAY=:0 XAUTHORITY=/home/chip/.Xauthority xmodmap /home/chip/.Xmodmap") == 0) {
		//~ printf("Successfully initialized .Xmodmap\n");
		initialized++;
	}
}

static int get_backlight_level() {

	int result = -1;	
	FILE *f = fopen("/sys/class/backlight/backlight/brightness", "r");
	if (f)	{
		fscanf(f, "%d", &result);
	}
	fclose(f);

	return result;
}

static void set_backlight_level(int level) {

	FILE *f = fopen("/sys/class/backlight/backlight/brightness", "w");
	if (f)	{
		fprintf(f, "%d\n", level);
	}
	fclose(f);
}

static void check_backlight() {

	seteuid(1000);

	Display *dpy = XOpenDisplay(":0");
	if (dpy == NULL) {
		fprintf(stderr, "Could not open the display\n");
		return;
	}

	int scr = DefaultScreen(dpy);

	BOOL onoff = 0;
	CARD16 state;
	BOOL got_info = DPMSInfo(dpy, &state, &onoff);

	XCloseDisplay(dpy);

	if (!got_info) {
		fprintf(stderr, "Could not get DPMS info\n");
		return;
	}

	int display_off = onoff && (state != DPMSModeOn);

	seteuid(0);
	static int disabled_backlight = 0;
	static int backlight_level;
	if (display_off) {
		if (!disabled_backlight) {
			backlight_level = get_backlight_level();
			if (backlight_level > 0) {
				set_backlight_level(0);
				disabled_backlight = 1;
			} 
		}
	} else {
		if (disabled_backlight) {
			set_backlight_level(backlight_level);
			disabled_backlight = 0;
		} else if (get_backlight_level() == 0) {
			// This is just in case the daemon was started later for some reason.
			set_backlight_level(backlight_level > 0 ? backlight_level : 4);
		}
	}
}

int main(int argc, char **argv) {

	int daemon = (argc >= 2 && strcmp(argv[1], "daemon") == 0);

	while (1) {

		init_xmodmap_if_needed();

		check_battery();

		if (!daemon)
			return 0;

		check_backlight();
		
		usleep(3 * 1000000);	
	}

	return 0;
}

// vim: ts=4:sw=4

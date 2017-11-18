/*
 * A lighter replacement for pocketchip-batt script in PocketCHIP. See the README.md.
 * Copyright (C) Aleh Dzenisiuk, 2017. http://github.com/aleh/pocketchip-batt
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>

#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int i2c_device;

static int _i2c_write(const uint8_t *buf, int len) {
	if (write(i2c_device, buf, len) != len) {
		return 0;
	}
	return 1;
}

static int _i2c_read_reg(uint8_t reg, uint8_t *buf, int len) {
	if (write(i2c_device, &reg, 1) != 1) {
		return 0;
	}
	if (read(i2c_device, buf, len) != len) {
		return 0;
	}
	return 1;
}

static int read_battery(int *charging, int *voltage) {

	if ((i2c_device = open("/dev/i2c-0", O_RDWR)) < 0) {
		perror("Could not open the adapter device");
		return 1;
	}

	if (ioctl(i2c_device, I2C_SLAVE_FORCE, 0x34) < 0) {
		perror("Could not set the slave address");
		return 2;
	}

	// The comment in the original battery.sh for this was "force ADC enable for battery voltage and current".
	if (!_i2c_write((const uint8_t[]){ 0x82, 0xC3 }, 2)) {
		perror("Could not enable the ADC");
		return 3;
	}

	uint8_t buf[8];
	if (!_i2c_read_reg(0x01, buf, 1)) {
		perror("Could not read charging status");
		return 5;
	}
	*charging = (buf[0] >> 6) & 1; 
	
	if (!_i2c_read_reg(0x78, buf, 1)) {
		perror("Could not read the voltage (MSB)");
		return 6;
	}
	
	if (!_i2c_read_reg(0x79, buf + 1, 1)) {
		perror("Could not read the voltage (LSB)");
		return 7;
	}
	*voltage = (((uint16_t)buf[0] << 4) | (buf[1] & 0x0F)) * 1.1;

	return 0;
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

int main() {

	int charging;
	int voltage;
	int result = read_battery(&charging, &voltage);
	if (result)
		return result;

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

	FILE *charging_file = fopen("/usr/lib/pocketchip-batt/charging", "w");
	if (charging_file == NULL) {
		perror("Could not update charging file");
	} else {
		fprintf(charging_file, "%d\n", charging);
		fclose(charging_file);
	}

	// The original script was averaging the voltage as well, but I don't want to bother.
	FILE *voltage_file = fopen("/usr/lib/pocketchip-batt/voltage", "w");
	if (voltage_file == NULL) {
		perror("Could not update the voltage file");
	} else {
		fprintf(voltage_file, "%d\n", voltage);
		fclose(voltage_file);
	}

	//
	// And it looks like it is not hard to do what the rest of the scripts were doing
	//

	// I am not sure who else is using these flag files, otherwise would put them to /run as well.
	const char warn05_flag[] = "/home/chip/.pocketchip-batt/warn05";
	const char warn15_flag[] = "/home/chip/.pocketchip-batt/warn15";
	if (!charging) {
		mkdir("/home/chip/.pocketchip-batt", 0777);	
		if (voltage < 3300) {
			// pocketchip-off00
			system("DISPLAY=:0 pocket-off");
		} else if (voltage < 3375) {
			// pocketchip-warn05
			if (!has_fs_flag(warn05_flag)) {
				system("DISPLAY=:0 pocket-exit-5");
			}
		} else if (voltage < 3500) {
			// pocketchip-warn15
			if (!has_fs_flag(warn15_flag)) {
				system("DISPLAY=:0 pocket-exit-15");
			}
		}
	}
	// pocketchip-warn05
	if (charging || voltage > 3400) {
		remove_fs_flag(warn05_flag);
	}
	// pocketchip-warn15
	if (charging || voltage > 3550) {
		remove_fs_flag(warn15_flag);
	}

	return 0;
}

// vim: ts=4:sw=4

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
		perror("I2C write failed");
		return 0;
	}
	return 1;
}

static int _i2c_read_reg(uint8_t reg, uint8_t *buf, int len) {
	if (write(i2c_device, &reg, 1) != 1) {
		perror("Could not write register address");
		return 0;
	}
	if (read(i2c_device, buf, len) != len) {
		perror("I2C read failed");
		return 0;
	}
	return 1;
}

int main() {

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
		return 3;
	}

	// We are interested only in printing of the info actually used by pocketchip-batt script.
	uint8_t charging;
	float voltage;

	uint8_t buf[8];
	if (!_i2c_read_reg(0x01, buf, 1)) {
		return 5;
	}
	charging = (buf[0] >> 6) & 1; 
	
	if (!_i2c_read_reg(0x78, buf, 1)) {
		return 6;
	}
	if (!_i2c_read_reg(0x79, buf + 1, 1)) {
		return 7;
	}
	voltage = (((uint16_t)buf[0] << 4) | (buf[1] & 0x0F)) * 1.1;

	// Let's print to stdout just in case this script is used instead of battery.sh .
	printf("CHARG_IND=%d\n", charging);
	printf("Battery voltage = %.0fmV\n", voltage);

	//
 	// And this is what the pocket-batt actually does (except for averaging).
	//

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
		fprintf(voltage_file, "%.0f\n", voltage);
		fclose(voltage_file);
	}

	close(i2c_device);

	return 0;
}


// vim: ts=4:sw=4



battery.sh: battery.c
	gcc battery.c -o battery.sh

install: battery.sh
	cp battery.sh /usr/bin

.PHONY: install

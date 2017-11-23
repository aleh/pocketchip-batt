

pocketchip-one: pocketchip-one.c
	gcc $< -lX11 -lXext -o $@

install: pocketchip-one pocketchip-one.service
	
	systemctl disable pocketchip-batt.timer
	systemctl disable pocketchip-off00.timer
	systemctl disable pocketchip-warn05.timer
	systemctl disable pocketchip-warn15.timer
	systemctl disable pocketchip-load.timer
	
	cp -f ./pocketchip-one /usr/sbin/pocketchip-one
	cp -f ./pocketchip-one.service /etc/systemd/system/
	
	# Need to make sure our temporary files with battery status will be on a temp FS.
	-rm -rf /usr/lib/pocketchip-batt
	ln -s /run/pocketchip-batt /usr/lib/pocketchip-batt
	
	# Enabling only our "one" service, the rest should be disabled.
	systemctl daemon-reload
	systemctl enable pocketchip-one.service
	systemctl reload-or-restart pocketchip-one.service

uninstall:
	systemctl disable pocketchip-batt.timer
	
	mv /etc/systemd/system/pocketchip-batt.timer~old /etc/systemd/system/pocketchip-batt.timer
	mv /etc/systemd/system/pocketchip-batt.service~old /etc/systemd/system/pocketchip-batt.service
	
	mv /usr/sbin/pocketchip-load~old /usr/sbin/pocketchip-load
	
	-mv /usr/sbin/pocketchip-batt~old /usr/sbin/pocketchip-batt
	-rm -rf /usr/lib/pocketchip-batt
	-mkdir /usr/lib/pocketchip-batt
	systemctl enable pocketchip-batt.timer
	systemctl enable pocketchip-off00.timer
	systemctl enable pocketchip-warn05.timer
	systemctl enable pocketchip-warn15.timer
	systemctl enable pocketchip-load.service

.PHONY: install uninstall


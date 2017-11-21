

pocketchip-batt: pocketchip-batt.c
	gcc $< -lX11 -lXext -o $@

install: pocketchip-batt pocketchip-batt.service pocketchip-batt.timer
	# Will reenable pocketchip-batt after replacing the script, but the rest should be disabled, out service does their job.
	systemctl disable pocketchip-batt.timer
	systemctl disable pocketchip-off00.timer
	systemctl disable pocketchip-warn05.timer
	systemctl disable pocketchip-warn15.timer
	systemctl disable pocketchip-load.timer

	if [[ -e /usr/sbin/pocketchip-batt ]] ; then mv -n /usr/sbin/pocketchip-batt /usr/sbin/pocketchip-batt~old ; fi
	cp ./pocketchip-batt /usr/sbin/pocketchip-batt

	mv -n /etc/systemd/system/pocketchip-batt.service /etc/systemd/system/pocketchip-batt.service~old
	mv -n /etc/systemd/system/pocketchip-batt.timer /etc/systemd/system/pocketchip-batt.timer~old
	cp ./pocketchip-batt.service /etc/systemd/system/
	cp ./pocketchip-batt.timer /etc/systemd/system/
	
	# Need to make sure our temporary files with battery status will be on a temp FS.
	-rm -rf /usr/lib/pocketchip-batt
	ln -s /run/pocketchip-batt /usr/lib/pocketchip-batt
	
	# OK, enabling our service back, the rest should be disabled.
	systemctl enable pocketchip-batt.timer

uninstall:
	systemctl disable pocketchip-batt.timer
	
	mv /etc/systemd/system/pocketchip-batt.timer~old /etc/systemd/system/pocketchip-batt.timer
	mv /etc/systemd/system/pocketchip-batt.service~old /etc/systemd/system/pocketchip-batt.service
	
	-mv /usr/sbin/pocketchip-batt~old /usr/sbin/pocketchip-batt
	-rm -rf /usr/lib/pocketchip-batt
	-mkdir /usr/lib/pocketchip-batt
	systemctl enable pocketchip-batt.timer
	systemctl enable pocketchip-off00.timer
	systemctl enable pocketchip-warn05.timer
	systemctl enable pocketchip-warn15.timer
	systemctl enable pocketchip-load.timer

.PHONY: install uninstall


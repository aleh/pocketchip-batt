

pocketchip-batt: pocketchip-batt.c
	gcc $< -o $@

install: pocketchip-batt
	# Will reenable pocketchip-batt after replacing the script, but the rest should be disabled, out service does their job.
	systemctl disable pocketchip-batt.timer
	systemctl disable pocketchip-off00.timer
	systemctl disable pocketchip-warn05.timer
	systemctl disable pocketchip-warn15.timer
	
	if [[ -e /usr/sbin/pocketchip-batt ]] ; then mv -n /usr/sbin/pocketchip-batt /usr/sbin/pocketchip-batt~old ; fi
	cp $< /usr/sbin/pocketchip-batt
	
	# Need to make sure our temporary files with battery status will be on a temp FS.
	-rm -rf /usr/lib/pocketchip-batt
	ln -s /run/pocketchip-batt /usr/lib/pocketchip-batt
	
	# OK, enabling our service back, the rest should be disabled.
	systemctl enable pocketchip-batt.timer

uninstall:
	systemctl disable pocketchip-batt.timer
	-mv /usr/sbin/pocketchip-batt~old /usr/sbin/pocketchip-batt
	-rm -rf /usr/lib/pocketchip-batt
	-mkdir /usr/lib/pocketchip-batt
	systemctl enable pocketchip-batt.timer
	systemctl enable pocketchip-off00.timer
	systemctl enable pocketchip-warn05.timer
	systemctl enable pocketchip-warn15.timer

.PHONY: install uninstall


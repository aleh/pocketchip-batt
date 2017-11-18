

pocketchip-batt: pocketchip-batt.c
	gcc $< -o $@

install: pocketchip-batt
	# Will reenable pocketchip-batt after replacing the script, but the rest should be disabled, out service does their job.
	systemctl disable pocketchip-batt.timer
	systemctl disable pocketchip-off00.timer
	systemctl disable pocketchip-warn05.timer
	systemctl disable pocketchip-warn15.timer
	
	cp --backup $< /usr/sbin/pocketchip-batt
	
	# Need to make sure our temporary files with battery status will be on a temp FS.
	rm -rf /usr/lib/pocketchip-batt
	ln -s /run/pocketchip-batt /usr/lib/pocketchip-batt
	
	# OK, enabling our service back, the rest should be disabled.
	systemctl enable pocketchip-batt.timer

.PHONY: install


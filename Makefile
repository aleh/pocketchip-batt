pocketchip-one: pocketchip-one.c
	gcc $< -lX11 -lXext -o $@

install: pocketchip-one pocketchip-one.service 
	
	# Let's disable the services that ours is replacing.
	for i in batt off00 warn05 warn15 load ; do echo $$i ; systemctl disable pocketchip-$$i.timer ; systemctl stop pocketchip-$$i.service ; done

	# Make sure battery status files are created on a tmpfs and not touching the flash.
	-rm -rf /usr/lib/pocketchip-batt
	ln -s /run/pocketchip-batt /usr/lib/pocketchip-batt
	
	# Copy and enable our service.	
	cp -f ./pocketchip-one /usr/sbin/pocketchip-one
	cp -f ./pocketchip-one.service /etc/systemd/system/
	
	systemctl daemon-reload
	systemctl enable pocketchip-one.service
	systemctl reload-or-restart pocketchip-one.service

uninstall:
	# Stop and disable our service.
	systemctl disable pocketchip-one.service
	systemctl stop pocketchip-one.service
	
	# Make a normal directory for the battery status files as it was before.
	-rm -rf /usr/lib/pocketchip-batt
	-mkdir /usr/lib/pocketchip-batt
	
	# Enable the pocketchip-* services back.
	for i in batt off00 warn05 warn15 load ; do echo $$i ; systemctl enable pocketchip-$$i.timer ; systemctl start pocketchip-$$i.service ; done

.PHONY: install uninstall


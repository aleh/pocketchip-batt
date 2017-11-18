

pocketchip-batt: pocketchip-batt.c
	gcc $< -o $@

install: pocketchip-batt
	cp $< /usr/sbin/pocketchip-batt
	rm -rf /usr/lib/pocketchip-batt
	ln -s /run/pocketchip-batt /usr/lib/pocketchip-batt

.PHONY: install




pocketchip-batt: pocketchip-batt.c
	gcc $< -o $@

install: pocketchip-batt
	cp $< /usr/sbin/pocketchip-batt

.PHONY: install


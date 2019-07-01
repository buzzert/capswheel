src = $(wildcard *.c)
obj = $(src:.c=.o)

PREFIX = /usr/local
LDFLAGS = -lpthread

capswheel: $(obj)
	$(CC) -o $@ $^ $(LDFLAGS)

install: capswheel
	cp $< $(DESTDIR)$(PREFIX)/bin/capswheel
	cp capswheel.service /etc/systemd/system
	systemctl daemon-reload

.PHONY: clean
clean:
	rm -f $(obj) capswheel

.PHONY: uninstall
	rm -f $(DESTDIR)$(PREFIX)/bin/capswheel


MENUS = gcin gcin-setup

include ../config.mak

all:

install:
	if [ $(prefix) = /usr/local ]; then \
           install gcin-setup.desktop $(GCIN_SETUP_DESKTOP); \
	   install gcin-setup.desktop /usr/share/applications; \
	   install -d /usr/lib/menu; \
	   install -m 644 $(MENUS) /usr/lib/menu; \
	   which update-menus > /dev/null 2>&1 && update-menus || exit 0; \
	else \
	   install -d $(libdir)/menu; \
	   install -m 644 $(MENUS) $(libdir)/menu; \
	   install -d $(GCIN_SETUP_DESKTOP_rpm); \
	   install -m 644 gcin-setup.desktop $(GCIN_SETUP_DESKTOP_rpm); \
	   install -d $(datadir)/applications; \
	   install -m 644 gcin-setup.desktop $(datadir)/applications; \
	fi
clean:
	rm -f $(MENUS) *~

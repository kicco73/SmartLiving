CONTIKI_PROJECT=border-router
all: resource_directory

chmod:
	sudo chmod 777 /dev/ttyUSB*

upload: chmod resource_directory.upload

tun:	chmod z1-reset connect-router

tuna: tun

CONTIKI=/home/user/contiki-2.7

WITH_UIP6=1
UIP_CONF_IPV6=1
CFLAGS += -DUIP_CONF_IPV6_RPL -DRPL_CONF_STATS=1
CFLAGS += -DUIP_CONF_IPV6=1 -DWITH_UIP6=1
#CFLAGS += -I../../wsn $(SENSORS)
#LDFLAGS += $(SENSORS)

CFLAGS += -DWITH_COAP=13
CFLAGS += -DREST=coap_rest_implementation
CFLAGS += -DUIP_CONF_TCP=1
APPS += er-coap-13
APPS += erbium

#linker optimizations
SMALL=1

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
PROJECT_SOURCEFILES += slip-bridge.c

#Simple built-in webserver is the default.
#Override with make WITH_WEBSERVER=0 for no webserver.
#WITH_WEBSERVER=webserver-name will use /apps/webserver-name if it can be
#found in the /apps, /platform/$(TARGET)/apps/, or current directory (in that order).
# WITH_WEBSERVER=webserver for /apps/webserver
# WITH_WEBSERVER=raven-webserver for /platform/avr-raven/apps/raven-webserver/
#make clean before changing webservers!

#Note /apps/webserver contains a 2500 byte style sheet which is a severe test
#of the slip connection. Large MSS together with low baud rates without flow
#control will overrun the transmit buffer when the style sheet is requested.

WITH_WEBSERVER=1
ifeq ($(WITH_WEBSERVER),1)
CFLAGS += -DWEBSERVER=1
PROJECT_SOURCEFILES += httpd-simple.c
else ifneq ($(WITH_WEBSERVER), 0)
APPS += $(WITH_WEBSERVER)
CFLAGS += -DWEBSERVER=2
endif

ifeq ($(PREFIX),)
 PREFIX = aaaa::1/64
endif

include $(CONTIKI)/Makefile.include

$(CONTIKI)/tools/tunslip6:	$(CONTIKI)/tools/tunslip6.c
	(cd $(CONTIKI)/tools && $(MAKE) tunslip6)

connect-router:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 $(PREFIX)

connect-router-cooja:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 -a 127.0.0.1 $(PREFIX)

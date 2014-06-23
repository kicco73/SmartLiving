CONTIKI = /home/user/contiki-2.7
CONTIKI_PROJECT = sensorigniter
LDFLAGS += $(SENSORS)
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
CFLAGS += -DUIP_CONF_IPV6_RPL -DRPL_CONF_STATS=1
CFLAGS += -DUIP_CONF_IPV6=1 -DWITH_UIP6=1
CFLAGS += -DUIP_CONF_TCP=0
CFLAGS += -DWITH_COAP=13 -DREST=coap_rest_implementation
CFLAGS += $(SENSORS)
APPS += er-coap-13 erbium
WITH_UIP6 = 1
UIP_CONF_IPV6 = 1
# code size optimization
SMALL = 1

all: server

power-sensor: drivers/power.o drivers/fan.o
	make SENSORS="$(SENSORS) -DWITH_POWER_SENSOR -Wl,drivers/power.o -DWITH_FAN_SENSOR -Wl,drivers/fan.o" all

upload: server.upload

chmod:
	sudo chmod 777 /dev/ttyUSB*

include $(CONTIKI)/Makefile.include

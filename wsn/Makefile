CONTIKI = /home/user/contiki-2.7
CONTIKI_PROJECT = sensorigniter
LDFLAGS += $(SENSORS)
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
CFLAGS += -DUIP_CONF_IPV6_RPL -DRPL_CONF_STATS=1
CFLAGS += -DUIP_CONF_IPV6=1 -DWITH_UIP6=1
CFLAGS += -DUIP_CONF_TCP=0 -DDEBUG=1
CFLAGS += -DWITH_COAP=13 -DREST=coap_rest_implementation
CFLAGS += $(SENSORS)
APPS += er-coap-13 erbium
WITH_UIP6 = 1
UIP_CONF_IPV6 = 1
# code size optimization
SMALL = 1

all: server

board1: drivers/motion.o
	make TARGET=z1 SENSORS="$(SENSORS) -DWITH_MOTION_SENSOR -Wl,../drivers/motion.o" -C rd

board2: drivers/sound.o drivers/fan.o drivers/light.o drivers/temp.o
	make TARGET=z1 SENSORS="$(SENSORS) -DWITH_SOUND_SENSOR -Wl,drivers/sound.o -DWITH_FAN_SENSOR -Wl,drivers/fan.o -DWITH_LIGHT_SENSOR -Wl,drivers/light.o -DWITH_TEMP_SENSOR -Wl,drivers/temp.o" all

board3: drivers/power.o drivers/dimmer.o
	make TARGET=z1 SENSORS="$(SENSORS) -DWITH_POWER_SENSOR -Wl,drivers/power.o -DWITH_DIMMER_SENSOR -Wl,drivers/dimmer.o" all

board4: drivers/co.o drivers/co2.o
	make TARGET=sky SENSORS="$(SENSORS) -DWITH_CO_SENSOR -Wl,drivers/co.o -DWITH_CO2_SENSOR -Wl,drivers/co2.o" all

upload: server.upload

chmod:
	sudo chmod 777 /dev/ttyUSB*

cleanall: clean
	make -C rd clean
	rm -f drivers/*.o

include $(CONTIKI)/Makefile.include

all: server_lux_temp_noise_fan

chmod: 
	sudo chmod 777 /dev/ttyUSB*

CONTIKI=/home/user/contiki-2.6

# configure CoAP implementation (3|7|12|13) (er-coap-07 also supports CoAP draft 08) (da Francesco)
WITH_COAP=13
REST=coap_rest_implementation


CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
CFLAGS += -DUIP_CONF_IPV6_RPL -DRPL_CONF_STATS=1

WITH_UIP6=1
UIP_CONF_IPV6=1
CFLAGS+= -DUIP_CONF_IPV6=1 -DWITH_UIP6=1
SMALL = 1

CFLAGS += -DWITH_COAP=13
CFLAGS += -DREST=coap_rest_implementation
CFLAGS += -DUIP_CONF_TCP=0
APPS += er-coap-13
APPS += erbium

# link librerie Math.h (da Francesco)
TARGET_LIBFILES += -lm


include $(CONTIKI)/Makefile.include

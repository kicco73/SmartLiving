/*
 */

#include <stdio.h>		/* For printf() */
#include <string.h>
#include "contiki.h"
#include "net/uip.h"
#include "net/uip-ds6.h"
#include "net/uip-debug.h"
#include "sys/etimer.h"
#include "dev/leds.h"
#include "dev/button-sensor.h"

#include "contiki-net.h"
#include "erbium.h"
#include "er-coap-13.h"
#include "er-coap-13-engine.h"

#include "drivers/power.h"
#include "drivers/light.h"
#include "drivers/sound.h"
#include "drivers/co.h"
#include "drivers/co2.h"
#include "drivers/fan.h"
#include "drivers/dimmer.h"
#include "drivers/temp.h"
#include "drivers/motion.h"

static driver_t driver[] = {
#ifdef WITH_POWER_SENSOR
	&POWER_DRIVER,
#endif
#ifdef WITH_LIGHT_SENSOR
	&LIGHT_DRIVER,
#endif
#ifdef WITH_SOUND_SENSOR
	&SOUND_DRIVER,
#endif
#ifdef WITH_CO_SENSOR
	&CO_DRIVER,
#endif
#ifdef WITH_CO2_SENSOR
	&CO2_DRIVER,
#endif
#ifdef WITH_FAN_SENSOR
	&FAN_DRIVER,
#endif
#ifdef WITH_DIMMER_SENSOR
	&DIMMER_DRIVER,
#endif
#ifdef WITH_TEMP_SENSOR
	&TEMP_DRIVER,
#endif
#ifdef WITH_MOTION_SENSOR
	&MOTION_DRIVER,
#endif
};

#define LED_TOGGLE_INTERVAL (CLOCK_SECOND >> 3)
#define REGISTER_INTERVAL (CLOCK_SECOND << 6)

/*---------------------------------------------------------------------------*/

PROCESS(registration_process, "registration handler for resource data delivery");
PROCESS(status_process, "led status feedback");
AUTOSTART_PROCESSES(&status_process, &registration_process);

static char registered = 0;

/*---------------------------------------------------------------------------*/

static uip_ipaddr_t ipaddr;

static void network_init() {
	int i, state;
	uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
	uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
	uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF); 
	puts("IPv6 addresses: ");
	for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
		state = uip_ds6_if.addr_list[i].state;
		if(uip_ds6_if.addr_list[i].isused &&
			(state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
			uip_debug_ipaddr_print(&uip_ds6_if.addr_list[i].ipaddr);
			puts("");
		}
	}
}

/*---------------------------------------------------------------------------*/

static void drivers_init() {
	int i;
	rest_init_engine();
	for(i = 0; i < sizeof(driver)/sizeof(driver_t); i++) {
		PRINTF("%s: initializing driver\n", driver[i]->name);
		driver[i]->init();
	}
}

/*---------------------------------------------------------------------------*/

/* This function is will be passed to COAP_BLOCKING_REQUEST() to handle responses. */
static void client_chunk_handler(void *response) {
	const uint8_t *chunk;
	int len = coap_get_payload(response, &chunk);
	PRINTF("RESOURCE DIRECTORY REPLY: %.*s", len, (char *)chunk);
	registered = 1;
}

/*---------------------------------------------------------------------------*/

static void sprint_ipaddr(uint8_t *buf, const uip_ipaddr_t *addr) {
  if(addr == NULL || addr->u8 == NULL) {
	sprintf(buf, "::");
    return;
  }
  uint16_t a;
  unsigned int i;
  int f;
  buf[0] = 0;
  for(i = 0, f = 0; i < sizeof(uip_ipaddr_t); i += 2) {
    a = (addr->u8[i] << 8) + addr->u8[i + 1];
    if(a == 0 && f >= 0) {
      if(f++ == 0) {
        strcat(buf, "::");
      }
    } else {
      if(f > 0) {
        f = -1;
      } else if(i > 0) {
        strcat(buf, ":");
      }
      sprintf(buf+strlen(buf), "%x", a);
    }
  }
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(registration_process, ev, data) {
	static int i;
	static uip_ipaddr_t server_ipaddr;
	static coap_packet_t request[1];
	static uint8_t buf[256];
	static struct etimer timer;

	PROCESS_BEGIN();
	PRINTF("Registration process started\n");
	network_init();
	drivers_init();
	SENSORS_ACTIVATE(button_sensor);
	uip_ip6addr(&server_ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 1);
	while(1) {
		PRINTF("*** REGISTERING ALL RESOURCES TO RESOURCE DIRECTORY\n");
		registered = 0;
		sprint_ipaddr(buf, &ipaddr);
		for(i = 0; i < sizeof(driver)/sizeof(driver_t); i++)
			sprintf(buf+strlen(buf), "\n%s\t%s\t%s", driver[i]->name, driver[i]->unit, driver[i]->type);
		strcat(buf, "\n");
		coap_init_message(request, COAP_TYPE_CON, COAP_PUT, 0);
		coap_set_header_uri_path(request, "register");
		coap_set_payload(request, buf, strlen(buf));
		COAP_BLOCKING_REQUEST(&server_ipaddr, REMOTE_PORT, request, client_chunk_handler);
		etimer_set(&timer, REGISTER_INTERVAL);
		PROCESS_WAIT_EVENT_UNTIL(!registered || (ev == sensors_event && data == &button_sensor) || etimer_expired(&timer));
	}
	PROCESS_END();
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(status_process, ev, data) {
	static struct etimer timer;
	static char led_period = 0;

	PROCESS_BEGIN();

  	PRINTF("CoAP observable event server started\n");
	etimer_set(&timer, LED_TOGGLE_INTERVAL);
	while(1) {
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
		switch(led_period) {
		case 0:
		case 2:
			leds_on(registered? LEDS_GREEN : LEDS_RED);
			break;
		case 1:
		case 4:
			if(registered) {
				leds_on(LEDS_GREEN);
				break;
			}
		default:
			leds_off(LEDS_GREEN | LEDS_RED);
		}
		etimer_reset(&timer);
		led_period = (led_period + 1) % 8;
	}
  	PROCESS_END();
}

/*---------------------------------------------------------------------------*/



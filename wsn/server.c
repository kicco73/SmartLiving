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

#include "drivers/driver.h"
#include "drivers/power.h"

static driver_t driver[] = {
#ifdef WITH_POWER_SENSOR
	POWER_DRIVER,
#endif
};

#if 1
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define LED_TOGGLE_INTERVAL (CLOCK_SECOND >> 1)

/*---------------------------------------------------------------------------*/

PROCESS(server_process, "CoAP server process with observable resources");
PROCESS(button_process, "Button handler for resource data delivery");
AUTOSTART_PROCESSES(&server_process, &button_process);

/*---------------------------------------------------------------------------*/

static void network_init() {
	static uip_ipaddr_t ipaddr;
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

PROCESS_THREAD(server_process, ev, data) {
	static struct etimer timer;

	PROCESS_BEGIN();

	network_init();
	rest_init_engine();

	for(i = 0; i < sizeof(driver)/sizeof(driver_t); i++) {
		PRINTF("%s: initializing driver\n", driver[i]->sensor->name);
		driver[i]->init();
	}

	etimer_set(&timer, LED_TOGGLE_INTERVAL);
  	PRINTF("CoAP observable event server started\n");
	while(1) {
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
		leds_toggle(LEDS_GREEN);
		etimer_reset(&timer);
	}
  }
  PROCESS_END();
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(button_process, ev, data) {
	static int i;
	static struct etimer timer;
	PROCESS_BEGIN();
	SENSORS_ACTIVATE(button_sensor);
	PRINTF("Button process started\n");
	while(1) {
		PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor);
		PRINTF("*** FORCING DELIVERY OF ALL RESOURCESY\n");
		leds_on(LEDS_RED);

		for(i = 0; i < sizeof(driver)/sizeof(driver_t); i++) {
			PRINTF("%s: forcing notification\n", driver[i]->sensor->name);
			driver[i]->notify();
		}

		etimer_set(&timer, CLOCK_SECOND >> 1);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
		leds_off(LEDS_RED);
	}
	PROCESS_END();
}

/*---------------------------------------------------------------------------*/


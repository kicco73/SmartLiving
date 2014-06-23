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
//#include "drivers/light.h"
//#include "drivers/sound.h"
//#include "drivers/co.h"
//#include "drivers/co2.h"
#include "drivers/fan.h"
//#include "drivers/dimmer.h"
//#include "drivers/temperature.h"

static driver_t driver[] = {
#ifdef WITH_POWER_SENSOR
	&POWER_DRIVER,
#endif
#ifdef WITH_LIGHT_SENSOR
	LIGHT_DRIVER,
#endif
#ifdef WITH_SOUND_SENSOR
	SOUND_DRIVER,
#endif
#ifdef WITH_CO_SENSOR
	CO_DRIVER,
#endif
#ifdef WITH_CO2_SENSOR
	CO2_DRIVER,
#endif
#ifdef WITH_FAN_SENSOR
	FAN_DRIVER,
#endif
#ifdef WITH_DIMMER_SENSOR
	DIMMER_DRIVER,
#endif
#ifdef WITH_TEMPERATURE_SENSOR
	TEMPERATURE_DRIVER,
#endif
};

#define LED_TOGGLE_INTERVAL (CLOCK_SECOND >> 1)

/*---------------------------------------------------------------------------*/

PROCESS(server_process, "CoAP server process with observable resources");
PROCESS(button_process, "Button handler for resource data delivery");
AUTOSTART_PROCESSES(&server_process, &button_process);

/*---------------------------------------------------------------------------*/

static void network_init() {
	int i, state;
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
	int i;
	static struct etimer timer;

	PROCESS_BEGIN();

	network_init();
	rest_init_engine();

	for(i = 0; i < sizeof(driver)/sizeof(driver_t); i++) {
		PRINTF("%s: initializing driver\n", driver[i]->name);
		driver[i]->init();
	}

	etimer_set(&timer, LED_TOGGLE_INTERVAL);
  	PRINTF("CoAP observable event server started\n");
	while(1) {
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
		leds_toggle(LEDS_GREEN);
		etimer_reset(&timer);
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
			PRINTF("%s: forcing notification\n", driver[i]->name);
			driver[i]->notify();
		}

		etimer_set(&timer, CLOCK_SECOND >> 1);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
		leds_off(LEDS_RED);
	}
	PROCESS_END();
}

/*---------------------------------------------------------------------------*/


/*
 */

#include "contiki.h"
#include <stdio.h>		/* For printf() */
#include "net/uip.h"
#include "net/uip-ds6.h"
#include "net/uip-debug.h"
#include "sys/etimer.h"
#include "dev/leds.h"
#include "dev/button-sensor.h"
#include "dev/sky-sensors.h"

#include "contiki-net.h"
#include "erbium.h"
#include "er-coap-13.h"
#include <string.h>
#include "dev/tmp102.h"
#include "dev/light-ziglet.h"
#include "dev/i2cmaster.h"
#include "dev/relay-phidget.h"
//#include "sound-sensor.h"

#include "sound-sensor.c"
//#include "irq.c"

#if 1
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif


#if 0
#define PRINTFDEBUG(...) printf(__VA_ARGS__)
#else
#define PRINTFDEBUG(...)
#endif


#define SENSOR_READ_INTERVAL (5*CLOCK_SECOND)

  static uint16_t light;

/*---------------------------------------------------------------------------*/

void resource1_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
	uint8_t length;
	uint8_t method = REST.get_method_type(request);
	if(method & METHOD_GET) {
		sprintf(buffer, "%ld", light);
		length = strlen(buffer)+1;
		REST.set_header_content_type(response, REST.type.APPLICATION_JSON); 
		REST.set_header_etag(response, (uint8_t *) &length, 1);
		REST.set_response_status(response, REST.status.OK);
		REST.set_response_payload(response, buffer, length);    // 
		leds_on(LEDS_BLUE);
	} else
		REST.set_response_status(response, REST.status.BAD_REQUEST);
}


/**************************************************/

void resource1_periodic_handler(resource_t *r) {
	static int event_counter;
  	leds_toggle(LEDS_BLUE);
	char *buffer[16];
	sprintf(buffer, "%ld", light);
	coap_packet_t notification[1];
	coap_init_message(notification, COAP_TYPE_CON, REST.status.OK, 0);
	coap_set_payload(notification, buffer, strlen(buffer)+1);
	REST.notify_subscribers(r, event_counter++, notification);
}
/******************************************************/


PERIODIC_RESOURCE(resource1, METHOD_GET, "lux", "title=\"lux resorce\";rt=\"Text\";obs", 5*CLOCK_SECOND);
/**************************/


PROCESS(coap_server_process, "CoAP server process with observable event resource");
AUTOSTART_PROCESSES(&coap_server_process);

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(coap_server_process, ev, data)
{

  PROCESS_BEGIN();
  static uip_ipaddr_t ipaddr;
  int i;
  uint8_t state;
  static uint8_t status;
  static struct etimer timer;
 
  int16_t tempint;
  uint16_t tempfrac;
  int16_t raw;
  uint16_t absraw;
  int16_t sign;
  char minus;

  static uint16_t val;

  light_ziglet_init();
  tmp102_init();
  relay_enable(7);
  SENSORS_ACTIVATE(sound_sensor);

  uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF); 

  rest_init_engine();
  rest_activate_periodic_resource(&periodic_resource_resource1);

  printf("IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE 
       || state == ADDR_PREFERRED)) {
            uip_debug_ipaddr_print(
               &uip_ds6_if.addr_list[i].ipaddr);
            printf("\n");
    }
  }

  PRINTFDEBUG("CoAP event observable server started\n");
  while(1) {
    etimer_set(&timer, SENSOR_READ_INTERVAL);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));

// misura di luminosità
    light = light_ziglet_read();
    printf("Light = %u\n", light);

// misura della temperatura
    sign = 1;
    raw = tmp102_read_temp_raw();
    absraw = raw;
    if(raw < 0) {		// Perform 2C's if sensor returned negative data
      absraw = (raw ^ 0xFFFF) + 1;
      sign = -1;
    }
    tempint = (absraw >> 8) * sign;
    tempfrac = ((absraw >> 4) % 16) * 625;	// Info in 1/10000 of degree
    minus = ((tempint == 0) & (sign == -1)) ? '-' : ' ';
    PRINTF("Temp = %c%d.%d\n", minus, tempint, tempfrac);

//attivazione fan
 status = relay_toggle();
 PRINTF("Relay [%d]\n", status);

//misura del rumore
 val = 200*value(0);
 printf("Actual val : %u\n", val);

    leds_toggle(LEDS_GREEN);
  }
  PROCESS_END();
}

/*---------------------------------------------------------------------------*/

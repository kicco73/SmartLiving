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

#include "power-sensor.c"
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


#define SENSOR_READ_INTERVAL (3*CLOCK_SECOND)
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


PERIODIC_RESOURCE(resource1, METHOD_GET, "lux", "title=\"lux resorce\";rt=\"Text\";obs", 1*CLOCK_SECOND);
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
  static OFFSET = 112;
  static SCALE_FACTOR=0.354;

  char minus;

 

  SENSORS_ACTIVATE(power_sensor);
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

//misura della potenza
  static int val=0; 
  static int val_int, val_dec;
 /* while(j<20) {
    etimer_set(&timer, SENSOR_READ_INTERVAL);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    j++;
    val += power_sensor.value(0);
    printf("Valori intermedi: %d\n", val); 
    leds_toggle(LEDS_GREEN);
  }
    printf("Actual val : %d\n", (int)(val/20));
*/
   /*---------------------------------*/
  while(1) {
     etimer_set(&timer,SENSOR_READ_INTERVAL);
     PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
     val=power_sensor.value(0);
     printf("Valore letto: %d\n", val);
     if(val<OFFSET) printf("Errore di lettura!!\n");
     else {
           printf("Il valore corretto: %d.%d\n", (int)((val-OFFSET)), (int)((val-OFFSET))*100);
	   val_int = (int)((val-OFFSET)*SCALE_FACTOR);
           val_dec = (int)((val-OFFSET)*SCALE_FACTOR)*100;
 	  }
   }
  PROCESS_END();
}

/*---------------------------------------------------------------------------*/

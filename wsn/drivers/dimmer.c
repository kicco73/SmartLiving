#include <string.h>
#include "contiki.h"
#include "dimmer.h"
#include "dev/sky-sensors.h"
#include "lib/sensors.h"
#include "erbium.h"
#include "er-coap-13.h"

static int low_value = 200;    // intervallo in cui l'onda quadra è bassa in battiti di CK
static int period0 = 400;       // periodo dell'onda quadra in battiti di CK

RESOURCE(dimmer_resource, METHOD_GET | METHOD_PUT, "dimmer", "title=\"dimmer status\";rt=\"Text\"");

/*---------------------------------------------------------------------------*/

static int sensor_configure(int period, int low) {

  if (!(period > 65535) || (period < 0) || (low < 0) || (low > period)){  
  
  
  TBCCR0 = (uint16_t) period;                // period è il periodo dell'onda quadra in battiti di Ck
  TBCCR2 = (uint16_t) low;                  // low è la durata in cui l'onda quadra è bassa in battiti di Ck
  period0 = period;
  low_value = low;
  return 1;
  }
  else return 0;
}

/*---------------------------------------------------------------------------*/

static void sensor_init() {
    rest_activate_resource(&resource_dimmer_resource);
	
    P4SEL |= 0x04;              // Configurazione delle porte presa dai Datasheet del Microcontrollore
    P4DIR |= 0x04;              // SEL = 1 indica l'utilizzo del T7/TB2, DIR = 1 indica che il segnale è in uscita 
    
    //START TIMER: si attiva automaticamente
    
    TBCTL = TBSSEL_1 | TBCLR; 	// setto il timer ACLK + Timer clean
    TBCCTL2 = OUTMOD_3;         // setto la modalità di uscita per la soglia 1 come: Set/Reset
    TBCTL |= MC_1;              // setto la Up Mode (TBR cresce fino al raggiungimento della soglia) per il Timer B
    sensor_configure(period0,low_value);  // setto il dimmer con un valore di default
}

/*---------------------------------------------------------------------------*/

static int sensor_value(int type) {    // restituisce la percentuale del periodo in cui l'onda è alta
	
	return 100*(period0-low_value)/period0;
}

/*---------------------------------------------------------------------------*/

static void sensor_notify() {
	// TODO
}


/*---------------------------------------------------------------------------*/

void dimmer_resource_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
	int8_t length;
	int value;
	uint8_t method = REST.get_method_type(request);
	if(method & METHOD_GET) {
		
		sprintf(buffer, "%d", sensor_value(0));
		length = strlen(buffer)+1;
		REST.set_header_content_type(response, REST.type.APPLICATION_JSON); 
		REST.set_header_etag(response, (uint8_t *) &length, 1);
		REST.set_response_status(response, REST.status.CREATED);
		REST.set_response_payload(response, buffer, length);
	}
	if(method & METHOD_PUT) {
		
		const char *tmpbuf;
		REST.get_post_variable(request, "duty", &tmpbuf); 
		value = atoi(tmpbuf);
		low_value = ((period0*(100-value))/100);  
		sensor_configure (period0,low_value);
		
		REST.set_response_status(response, REST.status.CHANGED);
		
	}
	  else
		REST.set_response_status(response, REST.status.BAD_REQUEST);
}

/*---------------------------------------------------------------------------*/

/*void dimmer_resource_periodic_handler(resource_t *r) {
	static int event_counter;
	char buffer[16];
	sprintf(buffer, "%d", sensor_value(0));
	coap_packet_t notification[1];
	coap_init_message(notification, COAP_TYPE_CON, REST.status.OK, 0);
	coap_set_payload(notification, buffer, strlen(buffer)+1);
	REST.notify_subscribers(r, event_counter++, notification);
}*/

/*---------------------------------------------------------------------------*/

struct Driver DIMMER_DRIVER = {
	.name = "dimmer sensor",
	.init = sensor_init, 
	.notify = sensor_notify
};

/*---------------------------------------------------------------------------*/

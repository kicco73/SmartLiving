#include <string.h>
#include "contiki.h"
#include "dimmer.h"
#include "dev/sky-sensors.h"
#include "lib/sensors.h"
#include "sys/etimer.h"
#include "erbium.h"
#include "er-coap-13.h"
#include "net/uip-debug.h"

static int period0 = 400;       // periodo dell'onda quadra in battiti di CK
static uint16_t low_aux = 0;
static process_event_t dimmer_put_event;

RESOURCE(dimmer_resource, METHOD_GET | METHOD_PUT, "dimmer", "title=\"dimmer status\";rt=\"Text\"");

PROCESS(dimmer_set, "dimmer_set");

PROCESS_THREAD(dimmer_set, event, data) {
	static struct etimer etimer;
	static int16_t delta = 0;
	static char counter;
	PROCESS_BEGIN();

	while(1) {
		PRINTF("*** WAITING FOR PUT EVENT!\n");
		PROCESS_WAIT_EVENT_UNTIL(event == dimmer_put_event);
		PRINTF("*** PUT EVENT!! low_aux = %u\n", low_aux);
		delta = ((int)low_aux - (int)TBCCR2) / 32;
		for(counter = 32; counter; counter--) {
			PRINTF("*** PUT EVENT!! TBCCR2 = %u\n", TBCCR2);
			PRINTF("*** PUT EVENT!!\n");
			etimer_set(&etimer, CLOCK_SECOND / 40 );
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&etimer));
			if(counter-1)
				TBCCR2 += delta;
			else
				TBCCR2 = low_aux;
		}
	}
	PROCESS_END();
}

/*---------------------------------------------------------------------------*/

static int sensor_configure(int period, int low) {

	if (!(period > 65535) || (period < 0) || (low < 0) || (low > period)){  
		TBCCR0 = (uint16_t) period;                // period è il periodo dell'onda quadra in battiti di Ck
		TBCCR2 = (uint16_t) low;                  // low è la durata in cui l'onda quadra è bassa in battiti di Ck
		period0 = period;
		low_aux = low;
		return 1;
	}
	return 0;
}

/*---------------------------------------------------------------------------*/

static void sensor_init() {
	rest_activate_resource(&resource_dimmer_resource);
	dimmer_put_event = process_alloc_event();

	P4SEL |= 0x04;              // Configurazione delle porte presa dai Datasheet del Microcontrollore
	P4DIR |= 0x04;              // SEL = 1 indica l'utilizzo del T7/TB2, DIR = 1 indica che il segnale è in uscita

	//START TIMER: si attiva automaticamente

	TBCTL = TBSSEL_1 | TBCLR; // setto il timer ACLK + Timer clean
	TBCCTL2 = OUTMOD_3;         // setto la modalità di uscita per la soglia 1 come: Set/Reset
	TBCTL |= MC_1;              // setto la Up Mode (TBR cresce fino al raggiungimento della soglia) per il Timer B
	sensor_configure(period0, period0+1);  // setto il dimmer con un valore di default
	process_start(&dimmer_set, NULL);
}

/*---------------------------------------------------------------------------*/

static uint16_t sensor_value(int type) {    // restituisce la percentuale del periodo in cui l'onda è alta
	return 100*(period0-TBCCR2)/period0;
}

/*---------------------------------------------------------------------------*/

void dimmer_resource_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
	int8_t length;
	uint16_t value;
	uint8_t method = REST.get_method_type(request);
	if(method & METHOD_GET) {
		sprintf(buffer, "%d", sensor_value(0));
		length = strlen(buffer)+1;
		REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
		REST.set_header_etag(response, (uint8_t *) &length, 1);
		REST.set_response_status(response, REST.status.OK);
		REST.set_response_payload(response, buffer, length);
	} else if(method & METHOD_PUT) {
		const char *tmpbuf;
		REST.get_post_variable(request, "v", &tmpbuf);
		value = (uint16_t) atoi(tmpbuf);
		low_aux = ((period0*(100-value))/100)+1;
		process_post(&dimmer_set, dimmer_put_event, NULL);
		REST.set_response_status(response, REST.status.CHANGED);
	} else
		REST.set_response_status(response, REST.status.BAD_REQUEST);
}

/*---------------------------------------------------------------------------*/

struct Driver DIMMER_DRIVER = {
	.name = "dimmer",
	.description = "dimmer actuator",
	.unit = "%",
	.type = "dimmer",
	.init = sensor_init,
};

/*---------------------------------------------------------------------------*/

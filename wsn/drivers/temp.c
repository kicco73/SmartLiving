#include <string.h>
#include "contiki.h"
#include "erbium.h"
#include "er-coap-13.h"
#include "uip-debug.h"
#include "lib/sensors.h"
#include "dev/sky-sensors.h"
#include "temp.h"
#include "dev/tmp102.h"

PERIODIC_RESOURCE(temp_resource, METHOD_GET, "temp", "title=\"temperature sensor\";rt=\"Text\";obs", 60*CLOCK_SECOND);
static void read_temp(char* buf){
 int16_t tempint;
  uint16_t tempfrac;
  int16_t raw;
  uint16_t absraw;
  int16_t sign;
  char minus;
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
    sprintf(buf, "%c%d.%d", minus, tempint, tempfrac);
}

/*---------------------------------------------------------------------------*/

static void sensor_init() {
  	tmp102_init();
	rest_activate_periodic_resource(&periodic_resource_temp_resource);
}

/*---------------------------------------------------------------------------*/

void temp_resource_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
	int8_t length;
	uint8_t method = REST.get_method_type(request);
	if(method & METHOD_GET) {
		read_temp(buffer);
		length = strlen(buffer)+1;
		REST.set_header_content_type(response, REST.type.APPLICATION_JSON); 
		REST.set_header_etag(response, (uint8_t *) &length, 1);
		REST.set_response_status(response, REST.status.OK);
		REST.set_response_payload(response, buffer, length);
	} else
		REST.set_response_status(response, REST.status.BAD_REQUEST);
}

/*---------------------------------------------------------------------------*/
void temp_resource_periodic_handler(resource_t *r) {
	static int event_counter;
	char buffer[16];
	PRINTF("*** temp_resource_periodic_handler(): called!\n");
	read_temp(buffer);
	coap_packet_t notification[1];
	coap_init_message(notification, COAP_TYPE_CON, REST.status.OK, 0);
	coap_set_payload(notification, buffer, strlen(buffer)+1);
	REST.notify_subscribers(r, event_counter++, notification);
	PRINTF("*** temp_resource_periodic_handler(): done!\n");
}

/*---------------------------------------------------------------------------*/

struct Driver TEMP_DRIVER = {
	.name = "temp",
	.description = "temperature sensor",
	.unit = "C",
	.type = "sensor",
	.init = sensor_init, 
};

/*---------------------------------------------------------------------------*/

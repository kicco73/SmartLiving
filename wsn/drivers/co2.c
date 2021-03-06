#include <string.h>
#include "contiki.h"
#include "erbium.h"
#include "er-coap-13.h"
#include "lib/sensors.h"
#include "dev/sky-sensors.h"
#include "co2.h"
#define INPUT_CHANNEL (1 << INCH_3) 
#define INPUT_REFERENCE SREF_1
#define CO2_MEM ADC12MEM3
#define CO2 1

#define RL 20000
#define R0 880000

const struct sensors_sensor co2_sensor;
PERIODIC_RESOURCE(co2_resource, METHOD_GET, "co2", "title=\"CO2 sensor\";rt=\"Text\";obs", 9*CLOCK_SECOND);



/*---------------------------------------------------------------------------*/

static void sensor_init() {
	SENSORS_ACTIVATE(co2_sensor);
	rest_activate_periodic_resource(&periodic_resource_co2_resource);
}

/*---------------------------------------------------------------------------*/

static int sensor_value(int type) {
	return CO2_MEM;
}

/*---------------------------------------------------------------------------*/

static int sensor_status(int type) {
  return sky_sensors_status(INPUT_CHANNEL, type);
}

/*---------------------------------------------------------------------------*/

static int sensor_configure(int type, int c) {
  return sky_sensors_configure(INPUT_CHANNEL, INPUT_REFERENCE, type, c);
}

/*---------------------------------------------------------------------------*/

static void read_co2(char* buf){

static uint32_t co2_1000;

co2_1000=sensor_value(0);//valore misurato x 1000
sprintf(buf,"%d",((co2_1000*2500)/4095));	
}

/*---------------------------------------------------------------------------*/

void co2_resource_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
	int8_t length;
	uint8_t method = REST.get_method_type(request);
	if(method & METHOD_GET) {
		read_co2(buffer);
		length = strlen(buffer)+1;
		REST.set_header_content_type(response, REST.type.APPLICATION_JSON); 
		REST.set_header_etag(response, (uint8_t *) &length, 1);
		REST.set_response_status(response, REST.status.OK);
		REST.set_response_payload(response, buffer, length);
	} else
		REST.set_response_status(response, REST.status.BAD_REQUEST);
}

/*---------------------------------------------------------------------------*/
void co2_resource_periodic_handler(resource_t *r) {
	static int event_counter;
	char buffer[16];
	read_co2(buffer);
	coap_packet_t notification[1];
	coap_init_message(notification, COAP_TYPE_NON, REST.status.OK, 0);
	coap_set_payload(notification, buffer, strlen(buffer)+1);
	REST.notify_subscribers(r, event_counter++, notification);
}


/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/

SENSORS_SENSOR(co2_sensor, "CO2 sensor", sensor_value, sensor_configure, sensor_status);

struct Driver CO2_DRIVER = {
	.name = "co2",
	.description = "CO2 sensor",
	.unit = "ppm",
	.type = "sensor",
	.init = sensor_init, 
};

/*---------------------------------------------------------------------------*/

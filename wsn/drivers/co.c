#include <string.h>
#include "contiki.h"
#include "erbium.h"
#include "er-coap-13.h"
#include "lib/sensors.h"
#include "dev/sky-sensors.h"
#include "co.h"
#define INPUT_CHANNEL  1 << INCH_5
#define INPUT_REFERENCE SREF_1
#define CO_MEM ADC12MEM5
#define CO 0

#define RL 20000
#define R0 880000

const struct sensors_sensor co_sensor;
PERIODIC_RESOURCE(co_resource, METHOD_GET, "co1", "title=\"CO sensor\";rt=\"Text\";obs", 7*CLOCK_SECOND);

static unsigned int co;

/*---------------------------------------------------------------------------*/

static void sensor_init() {
	SENSORS_ACTIVATE(co_sensor);
	rest_activate_periodic_resource(&periodic_resource_co_resource);
}

/*---------------------------------------------------------------------------*/

static int sensor_value(int type) {
	return CO_MEM;
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

static void read_co(char *buf){
static uint32_t sens_1000;
	
	co=sensor_value(0);
   	sens_1000 = 122850/co;  // ho moltiplicato per 1000
	if (sens_1000>800) sprintf(buf,"%d",100);
	else if (sens_1000>500)
		sprintf(buf,"%lu", (uint32_t)((-300*sens_1000)+250000)/1000);
	else sprintf(buf, "%lu",(uint32_t)(-3600*sens_1000+1900000)/1000);
}


/*---------------------------------------------------------------------------*/

void co_resource_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
	int8_t length;
	uint8_t method = REST.get_method_type(request);
	if(method & METHOD_GET) {
		read_co(buffer);
		length = strlen(buffer)+1;
		REST.set_header_content_type(response, REST.type.APPLICATION_JSON); 
		REST.set_header_etag(response, (uint8_t *) &length, 1);
		REST.set_response_status(response, REST.status.OK);
		REST.set_response_payload(response, buffer, length);
	} else
		REST.set_response_status(response, REST.status.BAD_REQUEST);
}

/*---------------------------------------------------------------------------*/

void co_resource_periodic_handler(resource_t *r) {
	static int event_counter;
	char buffer[16];
	//PRINTF("*** co_resource_periodic_handler(): called!\n");
	read_co(buffer);
	coap_packet_t notification[1];
	coap_init_message(notification, COAP_TYPE_NON, REST.status.OK, 0);
	coap_set_payload(notification, buffer, strlen(buffer)+1);
	REST.notify_subscribers(r, event_counter++, notification);
}


/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/

SENSORS_SENSOR(co_sensor, "CO sensor", sensor_value, sensor_configure, sensor_status);

struct Driver CO_DRIVER = {
	.name = "co1",
	.description = "CO sensor",
	.unit = "ppm",
	.type = "sensor",
	.init = sensor_init, 
};

/*---------------------------------------------------------------------------*/

#include <string.h>
#include "contiki.h"
#include "erbium.h"
#include "er-coap-13.h"
#include "lib/sensors.h"
#include "dev/sky-sensors.h"
#include "dimmer.h"

const struct sensors_sensor dimmer_sensor;

/*---------------------------------------------------------------------------*/

static void sensor_init() {
	// TODO
	//SENSORS_ACTIVATE(power_sensor);
	//rest_activate_periodic_resource(&periodic_resource_fan_resource);
}

/*---------------------------------------------------------------------------*/

static int sensor_value(int type) {
	// TODO
	return 0;
}

/*---------------------------------------------------------------------------*/

static int sensor_status(int type) {
	// TODO
  	return 0;
}

/*---------------------------------------------------------------------------*/

static int sensor_configure(int type, int c) {
	// TODO
	return 0;
}

/*---------------------------------------------------------------------------*/

void dimmer_resource_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
	int8_t length;
	uint8_t method = REST.get_method_type(request);
	if(method & METHOD_GET) {
		sprintf(buffer, "%d", sensor_value(0));
		length = strlen(buffer)+1;
		REST.set_header_content_type(response, REST.type.APPLICATION_JSON); 
		REST.set_header_etag(response, (uint8_t *) &length, 1);
		REST.set_response_status(response, REST.status.OK);
		REST.set_response_payload(response, buffer, length);
	} else
		REST.set_response_status(response, REST.status.BAD_REQUEST);
}

/*---------------------------------------------------------------------------*/

void dimmer_resource_periodic_handler(resource_t *r) {
	static int event_counter;
	char buffer[16];
	sprintf(buffer, "%d", sensor_value(0));
	coap_packet_t notification[1];
	coap_init_message(notification, COAP_TYPE_CON, REST.status.OK, 0);
	coap_set_payload(notification, buffer, strlen(buffer)+1);
	REST.notify_subscribers(r, event_counter++, notification);
}

/*---------------------------------------------------------------------------*/

SENSORS_SENSOR(dimmer_sensor, "dimmer actuator", sensor_value, sensor_configure, sensor_status);
PERIODIC_RESOURCE(dimmer_resource, METHOD_GET|METHOD_PUT, "dimmer", "title=\"dimmer actuator\";rt=\"Text\";obs", 5*CLOCK_SECOND);

struct Driver DIMMER_DRIVER = {
	.name = "dimmer",
	.description = "dimmer actuator",
	.unit = "%",
	.type = "dimmer",
	.init = sensor_init, 
};

/*---------------------------------------------------------------------------*/

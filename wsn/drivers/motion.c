#include <string.h>
#include "contiki.h"
#include "erbium.h"
#include "er-coap-13.h"
#include "lib/sensors.h"
#include "dev/sky-sensors.h"
#include "motion.h"

const struct sensors_sensor motion_sensor;

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

static void sensor_notify() {
	// TODO
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

void motion_resource_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
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

SENSORS_SENSOR(motion_sensor, "motion sensor", sensor_value, sensor_configure, sensor_status);
PERIODIC_RESOURCE(motion_resource, METHOD_GET, "on/off", "title=\"motion sensor;rt=\"Text\";obs", 5*CLOCK_SECOND);

const struct Driver MOTION_DRIVER = {
	.name = "motion sensor",
	.init = sensor_init, 
	.notify = sensor_notify
};

/*---------------------------------------------------------------------------*/

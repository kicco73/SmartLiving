#include "contiki.h"
#include "power.h"

#define INPUT_CHANNEL	(1 << INCH_0)

/*---------------------------------------------------------------------------*/

static void sensor_init() {
	SENSORS_ACTIVATE(power_sensor);
	rest_activate_periodic_resource(&periodic_resource_power_resource);
}

/*---------------------------------------------------------------------------*/

static int sensor_value(int type) {
	return AD12MEM0;
}

/*---------------------------------------------------------------------------*/

static void sensor_notify() {
	power_resource_periodic_handler(&periodic_resource_power_resource);
}

/*---------------------------------------------------------------------------*/

static int sensor_status(int type) {
  return sky_sensors_status(INPUT_CHANNEL, type);
}

/*---------------------------------------------------------------------------*/

static int sensor_configure(int type, int c) {
  return sky_sensors_configure(INPUT_CHANNEL, SREF_1, type, c);
}

/*---------------------------------------------------------------------------*/

void power_resource_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
	uint8_t length;
	uint8_t method = REST.get_method_type(request);
	if(method & METHOD_GET) {
		sprintf(buffer, "%ld", sensor_value(0));
		length = strlen(buffer)+1;
		REST.set_header_content_type(response, REST.type.APPLICATION_JSON); 
		REST.set_header_etag(response, (uint8_t *) &length, 1);
		REST.set_response_status(response, REST.status.OK);
		REST.set_response_payload(response, buffer, length);
	} else
		REST.set_response_status(response, REST.status.BAD_REQUEST);
}

/*---------------------------------------------------------------------------*/

void power_resource_periodic_handler(resource_t *r) {
	static int event_counter;
	char *buffer[16];
	sprintf(buffer, "%ld", sensor_value(0));
	coap_packet_t notification[1];
	coap_init_message(notification, COAP_TYPE_CON, REST.status.OK, 0);
	coap_set_payload(notification, buffer, strlen(buffer)+1);
	REST.notify_subscribers(r, event_counter++, notification);
}

/*---------------------------------------------------------------------------*/

SENSORS_SENSOR(power_sensor, "power", sensor_value, sensor_configure, sensor_status);
PERIODIC_RESOURCE(power_resource, METHOD_GET, "power W", "title=\"power sensor resource\";rt=\"Text\";obs", 5*CLOCK_SECOND);

const struct Driver POWER_DRIVER = struct driver {
	.init = init, 
	.notify = notify,
	.sensor = &power_sensor
};

/*---------------------------------------------------------------------------*/

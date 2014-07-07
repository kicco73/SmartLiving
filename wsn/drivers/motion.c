#include <string.h>
#include "contiki.h"
#include "erbium.h"
#include "er-coap-13.h"
#include "lib/sensors.h"
#include "dev/sky-sensors.h"
#include "net/uip-debug.h"
#include "motion.h"

#define INPUT_CHANNEL      (1 << 7)
#define INPUT_REFERENCE    SREF_1
#define MOTION_MEM    ADC12MEM7

const struct sensors_sensor motion_sensor;

PERIODIC_RESOURCE(motion_resource, METHOD_GET, "motion", "title=\"motion sensor\";rt=\"Text\";obs", 1*CLOCK_SECOND);
/*---------------------------------------------------------------------------*/

static void sensor_init() {
	SENSORS_ACTIVATE(motion_sensor);
	rest_activate_periodic_resource(&periodic_resource_motion_resource);
}

/*---------------------------------------------------------------------------*/

static int sensor_value(int type) {
	return MOTION_MEM;
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
void motion_resource_periodic_handler(resource_t *r) {
	static int event_counter;
	char buffer[16];
	PRINTF("*** motion_resource_periodic_handler(): called!\n");
	int value = 4095-sensor_value(0);
	sprintf(buffer, "%d", value);
	printf("misura presenza: %d\n",value);
	coap_packet_t notification[1];
	coap_init_message(notification, COAP_TYPE_NON, REST.status.OK, 0);
	coap_set_payload(notification, buffer, strlen(buffer)+1);
	REST.notify_subscribers(r, event_counter++, notification);
	PRINTF("*** motion_resource_periodic_handler(): done\n");
}


/*---------------------------------------------------------------------------*/

SENSORS_SENSOR(motion_sensor, "motion", sensor_value, sensor_configure, sensor_status);

struct Driver MOTION_DRIVER = {
	.name = "motion",
	.description = "motion sensor",
	.unit = "V",
	.type = "sensor",
	.init = sensor_init, 
};

/*---------------------------------------------------------------------------*/

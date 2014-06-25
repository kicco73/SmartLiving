#include <string.h>
#include "contiki.h"
#include "erbium.h"
#include "er-coap-13.h"
#include "lib/sensors.h"
#include "dev/sky-sensors.h"
#include "sound.h"
#define INPUT_CHANNEL      (1 << INCH_0)
#define INPUT_REFERENCE    SREF_1
#define SOUND_MEM    ADC12MEM0

const struct sensors_sensor sound_sensor;

PERIODIC_RESOURCE(sound_resource, METHOD_GET, "sound", "title=\"sound sensor\";rt=\"Text\";obs", 5*CLOCK_SECOND);

/*---------------------------------------------------------------------------*/

static void sensor_init() {

	SENSORS_ACTIVATE(sound_sensor);
	rest_activate_periodic_resource(&periodic_resource_sound_resource);
}

/*---------------------------------------------------------------------------*/

static int sensor_value(int type) {

  	return SOUND_MEM;
}

/*---------------------------------------------------------------------------*/

static void sensor_notify() {
	// TODO
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

void sound_resource_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
	int8_t length;
	uint8_t method = REST.get_method_type(request);
	if(method & METHOD_GET) {
		sprintf(buffer, "%d", 200*sensor_value(0));
		length = strlen(buffer)+1;
		REST.set_header_content_type(response, REST.type.APPLICATION_JSON); 
		REST.set_header_etag(response, (uint8_t *) &length, 1);
		REST.set_response_status(response, REST.status.OK);
		REST.set_response_payload(response, buffer, length);
	} else
		REST.set_response_status(response, REST.status.BAD_REQUEST);
}

/*---------------------------------------------------------------------------*/
void sound_resource_periodic_handler(resource_t *r) {
	static int event_counter;
	char buffer[16];
	sprintf(buffer, "%d", 200*sensor_value(0));
	coap_packet_t notification[1];
	coap_init_message(notification, COAP_TYPE_CON, REST.status.OK, 0);
	coap_set_payload(notification, buffer, strlen(buffer)+1);
	REST.notify_subscribers(r, event_counter++, notification);
}


/*---------------------------------------------------------------------------*/

SENSORS_SENSOR(sound_sensor, "sound sensor", sensor_value, sensor_configure, sensor_status);


const struct Driver SOUND_DRIVER = {
	.name = "sound sensor",
	.init = sensor_init, 
	.notify = sensor_notify
};

/*---------------------------------------------------------------------------*/

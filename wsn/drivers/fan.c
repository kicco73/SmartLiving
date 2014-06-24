#include <string.h>
#include "contiki.h"
#include "erbium.h"
#include "er-coap-13.h"
#include "lib/sensors.h"
#include "dev/sky-sensors.h"
#include "fan.h"
#include "dev/relay-phidget.h"
#include "dev/i2cmaster.h"

static int value;

const struct sensors_sensor fan_sensor;

/*---------------------------------------------------------------------------*/
RESOURCE(fan_resource, METHOD_GET|METHOD_PUT, "fan", "title=\"fan status\";rt=\"Text\"");

static void sensor_init() {
	relay_enable(7);
	rest_activate_resource(&resource_fan_resource);
}

/*---------------------------------------------------------------------------*/

static void sensor_notify() {
	// TODO
}

/*---------------------------------------------------------------------------*/

void fan_resource_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
	int8_t length;
	uint8_t method = REST.get_method_type(request);
	if(method & METHOD_GET) {
		sprintf(buffer, "%d", value);
		length = strlen(buffer)+1;
		REST.set_header_content_type(response, REST.type.APPLICATION_JSON); 
		REST.set_header_etag(response, (uint8_t *) &length, 1);
		REST.set_response_status(response, REST.status.OK);
		REST.set_response_payload(response, buffer, length);}
          else if(method & METHOD_PUT){
		const char *tmpbuf;
		REST.get_post_variable(request, "v", &tmpbuf);
		value = atoi(tmpbuf);
		if (value) relay_on();
		else relay_off();
		REST.set_response_status(response, REST.status.CHANGED);
	} else
		REST.set_response_status(response, REST.status.BAD_REQUEST);
}

/*---------------------------------------------------------------------------*/

const struct Driver FAN_DRIVER = {
	.name = "fan relay",
	.init = sensor_init, 
	.notify = sensor_notify
};

/*---------------------------------------------------------------------------*/

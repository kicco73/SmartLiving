#include <string.h>
#include "contiki.h"
#include "erbium.h"
#include "er-coap-13.h"
#include "uip-debug.h"
#include "lib/sensors.h"
#include "dev/sky-sensors.h"
#include "light.h"
#include "dev/light-ziglet.h"
#include "dev/i2cmaster.h"

PERIODIC_RESOURCE(light_resource, METHOD_GET, "light", "title=\"light sensor\";rt=\"Text\";obs", 11*CLOCK_SECOND);

/*---------------------------------------------------------------------------*/

static void sensor_init() {

  	light_ziglet_init();
	rest_activate_periodic_resource(&periodic_resource_light_resource);
}

/*---------------------------------------------------------------------------*/

void light_resource_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
	int8_t length;
	uint8_t method = REST.get_method_type(request);
	if(method & METHOD_GET) {
		sprintf(buffer, "%d", light_ziglet_read());
		length = strlen(buffer)+1;
		REST.set_header_content_type(response, REST.type.APPLICATION_JSON); 
		REST.set_header_etag(response, (uint8_t *) &length, 1);
		REST.set_response_status(response, REST.status.OK);
		REST.set_response_payload(response, buffer, length);
	} else
		REST.set_response_status(response, REST.status.BAD_REQUEST);
}

/*---------------------------------------------------------------------------*/

void light_resource_periodic_handler(resource_t *r) {
	static int event_counter;
	char buffer[16];
	PRINTF("*** light_resource_periodic_handler(): called!\n");
	sprintf(buffer, "%d", light_ziglet_read());
	coap_packet_t notification[1];
	coap_init_message(notification, COAP_TYPE_NON, REST.status.OK, 0);
	coap_set_payload(notification, buffer, strlen(buffer)+1);
	REST.notify_subscribers(r, event_counter++, notification);
	PRINTF("*** light_resource_periodic_handler(): done!\n");
}

/*---------------------------------------------------------------------------*/

struct Driver LIGHT_DRIVER = {
	.name = "light",
	.description = "light sensor",
	.unit = "lux",
	.type = "sensor",
	.init = sensor_init, 
};

/*---------------------------------------------------------------------------*/

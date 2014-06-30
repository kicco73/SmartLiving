/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */
/**
 * \file
 *         border-router
 * \author
 *         Niclas Finne <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 *         Nicolas Tsiftes <nvt@sics.se>
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/uip.h"
#include "net/uip-ds6.h"
#include "net/rpl/rpl.h"

#include "erbium.h"
#include "er-coap-13-engine.h"

#include "net/netstack.h"
#include "dev/slip.h"
#include "dev/leds.h"
//#include "dev/button-sensor.h"
#include "sys/etimer.h"

#if 0
#include "drivers/power.h"
#include "drivers/light.h"
#include "drivers/sound.h"
#include "drivers/co.h"
#include "drivers/co2.h"
#include "drivers/fan.h"
#include "drivers/dimmer.h"
#include "drivers/temp.h"
#include "drivers/motion.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define FAR __attribute__ ((section ("far_rom")))

// COAP_DEFAULT_PORT = 5683
#define REMOTE_PORT	UIP_HTONS(COAP_DEFAULT_PORT)

#define HTTP_ON_UPDATE 1

#define MAX_NUM_RESOURCES 10

#define LED_TOGGLE_INTERVAL CLOCK_SECOND >> 2

#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

uint16_t dag_id[] = {0x1111, 0x1100, 0, 0, 0, 0, 0, 0x0011};

extern uip_ds6_nbr_t uip_ds6_nbr_cache[];
extern uip_ds6_route_t uip_ds6_routing_table[];

static uip_ipaddr_t prefix;
static uint8_t prefix_set;

static coap_packet_t request[1];
static process_event_t put_event;
static int num_res = 0;
static char to_observe[MAX_NUM_RESOURCES];

struct resource {
	uip_ipaddr_t address;
	char n[8];
	char v[8];
	char u[8];
	char rt[8];
};
 
static struct resource rd[MAX_NUM_RESOURCES];

PROCESS(border_router_process, "BR");
PROCESS(webserver_nogui_process, "WS");
PROCESS(coap_put_process, "COAP");
#if HTTP_ON_UPDATE
PROCESS(on_update_process, "HTTP");

AUTOSTART_PROCESSES(&border_router_process,&webserver_nogui_process,&coap_put_process,&on_update_process);
#else
AUTOSTART_PROCESSES(&border_router_process,&webserver_nogui_process);
#endif


#if WEBSERVER==0
/* No webserver */
AUTOSTART_PROCESSES(&border_router_process);
#elif WEBSERVER>1
/* Use an external webserver application */
#include "webserver-nogui.h"
AUTOSTART_PROCESSES(&border_router_process,&webserver_nogui_process);
#else
/* Use simple webserver with only one page for minimum footprint.
 * Multiple connections can result in interleaved tcp segments since
 * a single static buffer is used for all segments.
 */
#include "httpd-simple.h"
/* The internal webserver can provide additional information if
 * enough program flash is available.
 */
#define WEBSERVER_CONF_LOADTIME 0
#define WEBSERVER_CONF_FILESTATS 0
#define WEBSERVER_CONF_NEIGHBOR_STATUS 0
/* Adding links requires a larger RAM buffer. To avoid static allocation
 * the stack can be used for formatting; however tcp retransmissions
 * and multiple connections can result in garbled segments.
 * TODO:use PSOCk_GENERATOR_SEND and tcp state storage to fix this.
 */
#define WEBSERVER_CONF_ROUTE_LINKS 0
#if WEBSERVER_CONF_ROUTE_LINKS
#define BUF_USES_STACK 1
#endif
#if 0
static driver_t driver[] = {
#ifdef WITH_POWER_SENSOR
	&POWER_DRIVER,
#endif
#ifdef WITH_LIGHT_SENSOR
	&LIGHT_DRIVER,
#endif
#ifdef WITH_SOUND_SENSOR
	&SOUND_DRIVER,
#endif
#ifdef WITH_CO_SENSOR
	&CO_DRIVER,
#endif
#ifdef WITH_CO2_SENSOR
	&CO2_DRIVER,
#endif
#ifdef WITH_FAN_SENSOR
	&FAN_DRIVER,
#endif
#ifdef WITH_DIMMER_SENSOR
	&DIMMER_DRIVER,
#endif
#ifdef WITH_TEMP_SENSOR
	&TEMP_DRIVER,
#endif
#ifdef WITH_MOTION_SENSOR
	&MOTION_DRIVER,
#endif
};
#endif
static int put_resource_idx;
static char put_value[8];
static char coap_success;

/*---------------------------------------------------------------------------*/

static int find_resource(char *name) {
	int i;
	for(i=0;i<num_res;i++) {
//		puts(rd[i].n);
		if(!strcmp(name, rd[i].n)) 
			return i;
	}
	return -1;
}

RESOURCE(register_resource, METHOD_PUT, "register", "title=\"R\";rt=\"Text\"");


/*---------------------------------------------------------------------------*/

void register_resource_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
	uint8_t length;
	int i; // i: place where to put the new resource
	static const uint8_t *chunk;
	char *pch, *ptr = NULL;
	char *v = "0";
	struct resource new_resource;
	
	uip_ipaddr_t new_ip;
	
	REST.set_header_content_type(response, REST.type.TEXT_PLAIN); 
	REST.set_header_etag(response, (uint8_t *) &length, 1);
	coap_get_payload(request, &chunk);

	pch = strtok_r((char*) chunk, "\n\t", &ptr);
	uiplib_ipaddrconv(pch, &new_ip);

	while((pch = strtok_r(NULL, "\n\t", &ptr))) {
		memset(&new_resource, 0, sizeof(new_resource));
		strncpy(new_resource.n, pch, sizeof(new_resource.n)-1);

		// Resource already present, then overwrite
		i = find_resource(new_resource.n);
		printf("S:'%s'\n", new_resource.n);
		if(i == -1) {
			printf("N:%s\n", new_resource.n);
			// Not enough space in memory for this new resource
			if(num_res == MAX_NUM_RESOURCES) {
				REST.set_response_status(response, REST.status.SERVICE_UNAVAILABLE);
				return;
			}
			i = num_res++;
		} else v = rd[i].v;

		uip_ipaddr_copy(&(new_resource.address), &new_ip);
		strncpy(new_resource.v, v, sizeof(new_resource.v)-1);
		strncpy(new_resource.u, strtok_r(NULL, "\n\t", &ptr), sizeof(new_resource.u)-1);
		strncpy(new_resource.rt, strtok_r(NULL, "\n\t", &ptr), sizeof(new_resource.rt)-1);
		
		/*puts("ad:");
		uip_debug_ipaddr_print(&uip_ds6_if.addr_list[i].ipaddr);
		puts("");*/
		//printf(": %s %s\n", new_resource.n, new_resource.rt);
		rd[i] = new_resource;
		to_observe[i] = !strcmp(new_resource.rt, "sensor") ; // observe only sensors
	}
	REST.set_response_status(response, REST.status.OK);
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(webserver_nogui_process, ev, data)
{
  PROCESS_BEGIN();

  httpd_init();
	
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
    httpd_appcall(data);
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

static
PT_THREAD(generate_routes(struct httpd_state *s))
{
	PSOCK_BEGIN(&s->sout);

	int i;

	if(s->method == GET) {

		if(!strcmp(s->filename, "/")) { // GET all resources
			int max = sizeof(s->http_output_payload)-2, len = 1;
			strcpy(s->http_output_payload,"[");
			for(i=0; i < num_res; i++) {
				len = strlen(s->http_output_payload);
				snprintf(s->http_output_payload+len, max - len, 
					"{\"n\":\"%s\",\"v\":%s,\"u\":\"%s\",\"rt\":\"%s\"}%c",
					rd[i].n, rd[i].v, rd[i].u, rd[i].rt, i < num_res-1? ',' : '\0');
			}
			strcpy(s->http_output_payload+strlen(s->http_output_payload),"]");
		} else { // GET a single resource
			i = find_resource(s->filename);
			//printf("G: %s, %d\n", s->filename, i);
			if(i != -1) {
				sprintf(s->http_output_payload,"{\"n\":\"%s\",\"v\":%s}",rd[i].n,rd[i].v);
			} else {
				s->http_header = HTTP_HEADER_404;
				strcpy(s->http_output_payload, "{}");
			}
		}

	} else { // PUT value in resource
	    char *pch = strtok(s->filename, "?");

		// First find the resource
		i = find_resource(pch);
		printf("P: %s, %d\n", pch, i);

		if(i != -1) {
			if((pch = strtok(NULL, "?")) != NULL && (pch = strtok(pch, "=")) != NULL && !strcmp(pch, "v") && (pch = strtok(NULL, "="))) {
				printf("n: %s, v: %s\n", rd[i].n, pch);
				put_resource_idx = i;
				strcpy(put_value, pch);
				process_post(&coap_put_process, put_event, NULL);
				s->http_header = HTTP_HEADER_200;
			} else
				s->http_header = HTTP_HEADER_400;
		} else
			s->http_header = HTTP_HEADER_404;
		strcpy(s->http_output_payload, "{}");
	}
	PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/

httpd_simple_script_t
httpd_simple_get_script(const char *name)
{
  return generate_routes;
}

/*---------------------------------------------------------------------------*/
#endif /* WEBSERVER */

void
request_prefix(void)
{
  /* mess up uip_buf with a dirty request... */
  uip_buf[0] = '?';
  uip_buf[1] = 'P';
  uip_len = 2;
  slip_send();
  uip_len = 0;
}

/*---------------------------------------------------------------------------*/

void
set_prefix_64(uip_ipaddr_t *prefix_64)
{
  uip_ipaddr_t ipaddr;
  memcpy(&prefix, prefix_64, 16);
  memcpy(&ipaddr, prefix_64, 16);
  prefix_set = 1;
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);
}

/*---------------------------------------------------------------------------*/

void client_observing_handler(coap_observee_t * subject, void *notification, coap_notification_flag_t flag) {
	uint8_t i, len;
	char buf[8];
	const uint8_t *chunk;

	if(flag) { // Consider only "NOTIFICATION_OK", which ignores "Added x/x" messages
		len = sizeof(buf);
		sprintf(buf, "/%s", subject->url);
		buf[len-1] = 0;

		i = find_resource(buf);
		len = coap_get_payload(notification, &chunk);

		strncpy(rd[i].v, chunk, sizeof(rd[i].v)-1);
		rd[i].v[sizeof(rd[i].v)-1] = 0;
	}

#if HTTP_ON_UPDATE
	process_post(&on_update_process, put_event, &i);
#endif
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(border_router_process, ev, data)
{
  static struct etimer et, observeETimer;
  rpl_dag_t *dag;
  static int i;
	static char led_period = 0;

  PROCESS_BEGIN();

/* While waiting for the prefix to be sent through the SLIP connection, the future
 * border router can join an existing DAG as a parent or child, or acquire a default 
 * router that will later take precedence over the SLIP fallback interface.
 * Prevent that by turning the radio off until we are initialized as a DAG root.
 */
  prefix_set = 0;
  NETSTACK_MAC.off(0);

  PROCESS_PAUSE();

  //PRINTF("RPL-Border router started\n");
#if 0
   /* The border router runs with a 100% duty cycle in order to ensure high
     packet reception rates.
     Note if the MAC RDC is not turned off now, aggressive power management of the
     cpu will interfere with establishing the SLIP connection */
  NETSTACK_MAC.off(1);
#endif
 
  /* Request prefix until it has been received */
  while(!prefix_set) {
    etimer_set(&et, CLOCK_SECOND);
    request_prefix();
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }

  dag = rpl_set_root(RPL_DEFAULT_INSTANCE,(uip_ip6addr_t *)dag_id);
  if(dag != NULL) {
    rpl_set_prefix(dag, &prefix, 64);
    //PRINTF("created a new RPL dag\n");
  }

  /* Now turn the radio on, but disable radio duty cycling.
   * Since we are the DAG root, reception delays would constrain mesh throughbut.
   */
  NETSTACK_MAC.off(1);

  etimer_set(&observeETimer, LED_TOGGLE_INTERVAL);

	memset(rd, 0, MAX_NUM_RESOURCES*sizeof(struct resource));

	put_event = process_alloc_event();
	// Initialize CoAP client
	coap_receiver_init();

	// Initialize REST server and REST resources
	rest_init_engine();
	rest_activate_event_resource(&resource_register_resource);
#if 0
	for(i = 0; i < sizeof(driver)/sizeof(driver_t); i++) {
		PRINTF("%s: initializing driver\n", driver[i]->name);
		driver[i]->init();
	}
#endif

	while(1) {
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&observeETimer));
#if 1
		if(led_period%2 || led_period >= num_res*2) {
			leds_off(LEDS_ALL);
		} else
			leds_on(LEDS_GREEN);

		if(led_period == 21) {
			for(i=0; i < num_res; i++) {
				if(to_observe[i]) {
					coap_obs_request_registration(&(rd[i].address), REMOTE_PORT, (rd[i].n)+1, client_observing_handler, NULL);
					printf("o4 %s\n", rd[i].n);
					to_observe[i] = 0;
				}
			}
		}
#endif
		etimer_reset(&observeETimer);
		led_period = (led_period + 1) % 22;
	}

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/

static void client_put_handler(void *response) {
	//int i;
  int status = ((coap_packet_t*)response)->code;

	if(status == REST.status.OK || status == REST.status.CHANGED) {
		strcpy(rd[put_resource_idx].v, put_value);
		coap_success = 1;
	} else {
		coap_success = 0;
	}
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(coap_put_process, ev, data)
{
	PROCESS_BEGIN();

	static char buf[16];
	static int i;
	
	while(1) {
		PROCESS_WAIT_EVENT_UNTIL(ev == put_event);
		coap_init_message(request, COAP_TYPE_CON, COAP_PUT, 0);
		coap_set_header_uri_path(request, rd[put_resource_idx].n+1);
		sprintf(buf, "v=%s", put_value);
		coap_set_payload(request, buf, strlen(buf));
//		puts("a:");
		uip_debug_ipaddr_print(&(rd[i].address));
		printf("\ni:%d\n", i);
//		puts(rd[put_resource_idx].n+1);
//		puts(buf);
		COAP_BLOCKING_REQUEST(&(rd[i].address), REMOTE_PORT, request, client_put_handler);
	}

	PROCESS_END();
}

#if HTTP_ON_UPDATE
void handle_request(struct psock *ps, int *i) {
		PSOCK_BEGIN(ps);

		static char buf[128];
		char buf2[128];
		sprintf(buf2, "[{\"n\":\"%s\",\"v\":%s}]\n", rd[*i].n, rd[*i].v);

		snprintf(buf, sizeof(buf)-1, "POST /resources/onUpdate HTTP/1.0\nContent-Length: %d\nContent-type: application/json\n\n%s", strlen(buf2), buf2);
		SEND_STRING(ps, buf);

		PSOCK_CLOSE(&ps);

		PSOCK_END(ps);
}

PROCESS_THREAD(on_update_process, ev, data)
{
	PROCESS_BEGIN();

	static struct psock ps;
	static char buffer[128];
	static uip_ip6addr_t webapp;
	uip_ip6addr(&webapp, 0xaaaa,0,0,0,0,0,0,1);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == put_event);
		puts("U!");
		tcp_connect(&webapp, UIP_HTONS(80), NULL);
		
		PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
		if(uip_connected()) {
		  PSOCK_INIT(&ps, (uint8_t *) buffer, sizeof(buffer));
			while(!(uip_aborted() || uip_closed() || uip_timedout())) {
				PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
				handle_request(&ps, data);
			}
    } 
  }
  PROCESS_END();
}
#endif

/*---------------------------------------------------------------------------*/
#if 0
PROCESS_THREAD(button_on_update_process, ev, data) {
	static int i;
	static struct etimer timer;
	PROCESS_BEGIN();
	SENSORS_ACTIVATE(button_sensor);
	PRINTF("Button process started\n");
	while(1) {
		PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor);
		PRINTF("*** FORCING DELIVERY OF ALL RESOURCES\n");
		leds_on(LEDS_RED);

		for(i = 0; i < sizeof(driver)/sizeof(driver_t); i++) {
			PRINTF("%s: forcing notification\n", driver[i]->name);
			driver[i]->notify();
		}

		etimer_set(&timer, CLOCK_SECOND >> 1);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
		leds_off(LEDS_RED);
	}
	PROCESS_END();
}
#endif

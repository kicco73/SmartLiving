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
#include "er-coap-13.h"

#include "net/netstack.h"
#include "dev/slip.h"
#include "dev/leds.h"
#include "dev/button-sensor.h"
#include "sys/etimer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// COAP_DEFAULT_PORT = 5683
#define REGISTER_PORT	UIP_HTONS(COAP_DEFAULT_PORT)
#define GET_SENSOR_PORT	UIP_HTONS(COAP_DEFAULT_PORT+1)
#define PUT_SENSOR_PORT	UIP_HTONS(COAP_DEFAULT_PORT+2)

#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

uint16_t dag_id[] = {0x1111, 0x1100, 0, 0, 0, 0, 0, 0x0011};

extern uip_ds6_nbr_t uip_ds6_nbr_cache[];
extern uip_ds6_route_t uip_ds6_routing_table[];

static uip_ipaddr_t prefix;
static uint8_t prefix_set;

static coap_packet_t request[1];

struct resource {
	uip_ipaddr_t address;
	char *n;
	char *v;
	char *u;
	char *rt;
};
 
static struct resource rd[]={
	{.address=0, .n="/accelerometer", .v="0", .u="m/s^2", .rt="sensor"},
	{.address=0, .n="/light", .v="0", .u="lux", .rt="dimmer"},
	{.address=0, .n="/fan", .v="0", .u="", .rt="switch"}
}; 

PROCESS(border_router_process, "Border router process");

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

static char *put_resource;
static char *put_value;
static char coap_success;

// Handle CoAP PUT response
void client_put_handler(void *response) {
	// FIXME: update resource in local RD
	int i;
  int status = ((coap_packet_t*)response)->code;

	if(status == REST.status.OK || status == REST.status.CHANGED) {
		i = find_resource(put_resource);
		if(i != -1) {
			rd[i].v = put_value;
			coap_success = 1;
		}
	} else {
		coap_success = 0;
	}
}

  //PRINTA("Reply: %d %.*s\n", status, len, (char*) chunk);

// Handle CoAP GET response
void client_get_handler(void *response) {
	// TODO: add resource to local RD, and verify duplicates?
	PRINTA("client_get_handler()");
}

void register_sensor_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
	// FIXME: Request resources' list to the new sensor
	uint8_t length;
	REST.set_header_content_type(response, REST.type.APPLICATION_JSON); 
	REST.set_header_etag(response, (uint8_t *) &length, 1);
	REST.set_response_status(response, REST.status.OK);
	leds_toggle(LEDS_BLUE);
}

EVENT_RESOURCE(register_sensor, METHOD_POST, "register", "title=\"register_resource\";rt=\"Text\"");

int find_resource(char *name) {
	int i;
	int rd_size=sizeof(rd)/sizeof(rd[0]);

	for(i=0;i<rd_size;i++) {
		if(strcmp(name, rd[i].n) == 0) {
			return i;
		}
	}
	return -1;
}

PROCESS(webserver_nogui_process, "Web server");
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

// TODO: to remove
//PROCESS(button_on_update_process, "Send HTTP request with button process");

AUTOSTART_PROCESSES(&border_router_process,&webserver_nogui_process/*,&button_on_update_process*/);

static
PT_THREAD(generate_routes(struct httpd_state *s))
{
  static int i;
  char buf[128], *pch;
	int rd_size=sizeof(rd)/sizeof(rd[0]);

	if(s->method == GET) {

		if(!strncmp(s->filename, "/" ,2)) { // GET all resources
			strcpy(s->http_output_payload,"[");	  
			for(i=0;i<rd_size;i++) {
				sprintf(s->http_output_payload+strlen(s->http_output_payload),"{\"n\":\"%s\",\"v\":%s,\"u\":\"%s\",\"rt\":\"%s\"}%c",rd[i].n,rd[i].v,rd[i].u,rd[i].rt,i+1<rd_size?',':']');
			}

		} else { // GET a single resource
			i = find_resource(s->filename);
			if(i != -1) {
				sprintf(s->http_output_payload,"{\"n\":\"%s\",\"v\":%s}",rd[i].n,rd[i].v);
			} else {
				s->http_header = HTTP_HEADER_404;
				sprintf(s->http_output_payload, "{}");
			}
		}

	} else { // PUT value in resource
		pch = strtok(s->filename, "?");

		// First find the resource
		for(i=0;i<rd_size;i++) if(strcmp(pch, rd[i].n) == 0) break;

		if(i != rd_size) {
			pch = strtok(NULL, "?");
			if(pch != NULL) {
				pch = strtok(pch, "=");
				if(pch != NULL && strcmp(pch, "v") == 0) {
					pch = strtok(NULL, "=");
					if(pch != NULL) {
						put_resource = rd[i].n;
						put_value = pch;
						coap_init_message(request, COAP_TYPE_CON, COAP_PUT, 0);
  					coap_set_header_uri_path(request, put_resource+1);
						sprintf(buf, "v=%s", put_value);
  					coap_set_header_uri_query(request, buf);
						//COAP_BLOCKING_REQUEST(&(rd[i].address), PUT_SENSOR_PORT, request, client_put_handler);
						s->http_header = coap_success ? HTTP_HEADER_200 : HTTP_HEADER_502;
						sprintf(s->http_output_payload, "{}");
					}	else {
						s->http_header = HTTP_HEADER_400;
						sprintf(s->http_output_payload, "{}");
					}
				} else {
					s->http_header = HTTP_HEADER_400;
					sprintf(s->http_output_payload, "{}");
				}
			} else {
				s->http_header = HTTP_HEADER_400;
				sprintf(s->http_output_payload, "{}");
			}
		} else {
			s->http_header = HTTP_HEADER_404;
			sprintf(s->http_output_payload, "{}");
		}
	}
}
/*---------------------------------------------------------------------------*/
httpd_simple_script_t
httpd_simple_get_script(const char *name)
{
  return generate_routes;
}

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
PROCESS_THREAD(border_router_process, ev, data)
{
  static struct etimer et, ledETimer;
  rpl_dag_t *dag;

  PROCESS_BEGIN();

/* While waiting for the prefix to be sent through the SLIP connection, the future
 * border router can join an existing DAG as a parent or child, or acquire a default 
 * router that will later take precedence over the SLIP fallback interface.
 * Prevent that by turning the radio off until we are initialized as a DAG root.
 */
  prefix_set = 0;
  NETSTACK_MAC.off(0);

  PROCESS_PAUSE();

  PRINTF("RPL-Border router started\n");
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
    PRINTF("created a new RPL dag\n");
  }

  /* Now turn the radio on, but disable radio duty cycling.
   * Since we are the DAG root, reception delays would constrain mesh throughbut.
   */
  NETSTACK_MAC.off(1);
  
#if DEBUG
  print_local_addresses();
#endif

  etimer_set(&ledETimer, CLOCK_SECOND >> 1);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&ledETimer));
    etimer_restart(&ledETimer);
    leds_toggle(LEDS_GREEN);
  }

	// Initialize CoAP client
	coap_receiver_init();

	// Initialize REST server and REST resources
	rest_init_engine();
	rest_activate_event_resource(&resource_register_sensor);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/*static int
handle_request(struct psock *ps)
{
  PSOCK_BEGIN(ps);

  PSOCK_SEND_STR(ps, "POST /resources/onUpdate HTTP/1.0\n");
  PSOCK_SEND_STR(ps, "Content-type: application/json\n");
  PSOCK_SEND_STR(ps, "\n");
	PSOCK_SEND_STR(ps, "[{\"n\":\"/light\",\"v\":0.35}]\n");
  
  PSOCK_END(ps);
}

PROCESS_THREAD(button_on_update_process, ev, data)
{
	PROCESS_BEGIN();

	static struct psock ps;
	static char buffer[128];
	static uip_ip6addr_t webapp;
	uip_ip6addr(&webapp, 0xaaaa,0,0,0,0,0,0,1);

  SENSORS_ACTIVATE(button_sensor);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor);
		
		tcp_connect(&webapp, UIP_HTONS(80), NULL);
		printf("Connecting...\n");
		PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
		if(uip_aborted() || uip_timedout() || uip_closed()) {
			printf("Could not establish connection\n");
		} else if(uip_connected()) {
			printf("Connected.\n");

			PSOCK_INIT(&ps, buffer, sizeof(buffer));

			do {
				handle_request(&ps);
				PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
			} while(!(uip_closed() || uip_aborted() || uip_timedout()));

			PSOCK_CLOSE(&ps);

			printf("Connection close.\n");
		}
  }
  PROCESS_END();
}*/

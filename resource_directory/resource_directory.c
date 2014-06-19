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
#ifdef PRINTA
#undef PRINTA
#define PRINTA 
#endif

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

// Handle CoAP PUT response
void client_put_handler(void *response) {
	// FIXME: update resource in local RD
  /*const uint8_t *chunk;
	char *n = "\"n\":", *v = "\"v\":", *value = NULL, *name = NULL;
  PRINTA("Ho la risposta\n");
  int len = coap_get_payload(response, &chunk);
  int status = ((coap_packet_t*)response)->code;

	static char *pch;
	pch = strtok(chunk, "{}");
	pch = strtok(NULL, ",");
	while(pch != NULL) {
		if(strncmp(pch, v, strlen(v)) == 0) {
			value = pch+strlen(v);
		} else if(strncmp(pch, n, strlen(v)) == 0) {
			name = pch+strlen(n);
		}
		pch = strtok(NULL, ",");
	}
	// Correct response received
	if(value != NULL && name != NULL) {
	}
}

  PRINTA("Reply: %d %.*s\n", status, len, (char*) chunk);*/
}

// Handle CoAP GET response
void client_get_handler(void *response) {
	// TODO: add resource to local RD, and verify duplicates?
	PRINTA("clien");
}

void register_sensor_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
	// FIXME: Request resources' list to the new sensor
	uint8_t length;
	REST.set_header_content_type(response, REST.type.APPLICATION_JSON); 
	REST.set_header_etag(response, (uint8_t *) &length, 1);
	REST.set_response_status(response, REST.status.OK);
	leds_toggle(LEDS_BLUE);
}

EVENT_RESOURCE(register_sensor, METHOD_POST, "register", "title=\"register resource\";rt=\"Text\"");

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
AUTOSTART_PROCESSES(&border_router_process,&webserver_nogui_process);

const char this_http[] = "http://[aaaa::c30c:0:0:1]";

static
PT_THREAD(generate_routes(struct httpd_state *s))
{
  static int i;
  char buf[512], *pch;
	static int rd_size=sizeof(rd)/sizeof(rd[0]);

  PSOCK_BEGIN(&s->sout);

	if(s->method == GET) {

		if(!strncmp(s->filename, "/" ,2)) { // GET all resources
			strcpy(buf,"[");	  
			for(i=0;i<rd_size;i++) {
				sprintf(buf+strlen(buf),"{\"n\":\"%s%s\",\"v\":%s,\"u\":\"%s\",\"rt\":\"%s\"}%c",this_http,rd[i].n,rd[i].v,rd[i].u,rd[i].rt,i+1<rd_size?',':']');
			}
			SEND_STRING(&s->sout,buf);

		} else { // GET a single resource
			for(i=0;i<rd_size;i++) {
				if(strcmp(s->filename, rd[i].n) == 0) {
					sprintf(buf,"{\"n\":\"%s%s\",\"v\":%s}",this_http,rd[i].n,rd[i].v);
					SEND_STRING(&s->sout,buf);
					break;
				}
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
						// FIXME: I tried to skip the "/", not sure if it works
						char *resource = rd[i].n;
						resource += 1;
						coap_init_message(request, COAP_TYPE_CON, COAP_PUT, 0);
  					coap_set_header_uri_path(request, resource);
						sprintf(buf, "v=%s", pch);
  					coap_set_header_uri_query(request, buf);
						//COAP_BLOCKING_REQUEST(&(rd[i].address), PUT_SENSOR_PORT, request, client_put_handler);
					}	else
						SEND_STRING(&s->sout,"{\"message\" : \"missing parameter's value\"}");
				} else
					SEND_STRING(&s->sout,"{\"message\" : \"invalid parameters\"}");
			} else
				SEND_STRING(&s->sout,"{\"message\" : \"missing parameters\"}");
		} else
			SEND_STRING(&s->sout,"{\"message\" : \"resource not found\"}");
	}

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
httpd_simple_script_t
httpd_simple_get_script(const char *name)
{

  return generate_routes;
}

#endif /* WEBSERVER */

/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTA("Server IPv6 addresses:\n");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      PRINTA(" ");
      uip_debug_ipaddr_print(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTA("\n");
    }
  }
}
/*---------------------------------------------------------------------------*/
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
  static struct etimer et;
  static struct etimer ledETimer;
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
  
#if DEBUG
  print_local_addresses();
#endif

  etimer_set(&ledETimer, CLOCK_SECOND >> 1);

  /*while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&ledETimer));
    etimer_restart(&ledETimer);
    leds_toggle(LEDS_GREEN);
  }*/

	// Initialize CoAP client
	coap_receiver_init();

	// Initialize REST server and REST resources
	rest_init_engine();
	rest_activate_event_resource(&resource_register_sensor);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

#undef RF_CHANNEL
#define RF_CHANNEL      21
#define REMOTE_PORT     UIP_HTONS(COAP_DEFAULT_PORT)

//#undef RPL_CONF_DIO_REDUNDANCY
//#define RPL_CONF_DIO_REDUNDANCY 5

//#undef RPL_CONF_DIO_INTERVAL_MIN 
//#define RPL_CONF_DIO_INTERVAL_MIN 30

//#undef RPL_CONF_DIO_INTERVAL_DOUBLINGS
//#define RPL_CONF_DIO_INTERVAL_DOUBLINGS 5

#undef IEEE802154_CONF_PANID
#undef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC     nullrdc_driver
#undef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC     nullmac_driver
#undef REST_MAX_CHUNK_SIZE
#define REST_MAX_CHUNK_SIZE    256
#undef COAP_MAX_OPEN_TRANSACTIONS
#define COAP_MAX_OPEN_TRANSACTIONS   4
//#undef COAP_MAX_OBSERVERS
//#define COAP_MAX_OBSERVERS 1

/* Save some memory for the sky platform. */
#undef NBR_TABLE_CONF_MAX_NEIGHBORS
#define NBR_TABLE_CONF_MAX_NEIGHBORS     10
#undef UIP_CONF_MAX_ROUTES
#define UIP_CONF_MAX_ROUTES   10
#undef UIP_CONF_BUFFER_SIZE
#define UIP_CONF_BUFFER_SIZE    472


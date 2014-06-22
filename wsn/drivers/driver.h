#ifndef __DRIVER_H__
#define __DRIVER_H__

#include "dev/sky-sensors.h"

typedef struct Driver {
	void (*init)();
  	void (*notify)();
	struct sensors_sensor *sensor;
} *driver_t;

#endif

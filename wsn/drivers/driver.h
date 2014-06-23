#ifndef __DRIVER_H__
#define __DRIVER_H__

typedef struct Driver {
	char* name;
	void (*init)();
  	void (*notify)();
} *driver_t;

#endif

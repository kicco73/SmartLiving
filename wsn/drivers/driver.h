#ifndef __DRIVER_H__
#define __DRIVER_H__

typedef struct Driver {
	char* name;
	char* description;
	char* unit;
	char* type;
	void (*init)();
} *driver_t;

#endif

#include "sound-sensor.h"
#include "dev/sky-sensors.h"
#include "contiki.h"

#define INPUT_CHANNEL      (1 << INCH_0)
#define INPUT_REFERENCE    SREF_1
#define SOUND_MEM    ADC12MEM0

const struct sensors_sensor sound_sensor;

/*---------------------------------------------------------------------------*/
static int value(int type)
{
  return SOUND_MEM;
}
/*---------------------------------------------------------------------------*/
static int configure(int type, int c)
{
  return sky_sensors_configure(INPUT_CHANNEL, INPUT_REFERENCE, type, c);
}
/*---------------------------------------------------------------------------*/
static int status(int type)
{
  return sky_sensors_status(INPUT_CHANNEL, type);
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(sound_sensor, SOUND_SENSOR, value, configure, status);
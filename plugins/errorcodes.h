// plugin error codes
// if your plugin needs others, just add

// used for various plugins
#define NOERROR 0
#define ERROR_DEV_NOTFOUND 7

// used for actuator_external.so
#define ERROR_ON_CMDNOTFOUND 1
#define ERROR_OFF_CMDNOTFOUND 2
#define ERROR_CHECK_CMDNOTFOUND 3
#define ERROR_CHECK_CMDFAILED 4

// used for actuator_gpio.so
#define IOERROR 5

// used for sensor_onewire.so and and actuator_onewire.so
#define OWINIT_ERROR 6


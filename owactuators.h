#ifndef NO1W
/* Supported actuators */
static const char* actuators[] = {"DS2406","DS2408","DS2413",NULL};

static const char* ds2406ports[] = {"PIO.A","PIO.B",NULL};
static const char* ds2408ports[] = {"PIO.0","PIO.1","PIO.2","PIO.3","PIO.4","PIO.5","PIO.6","PIO.7",NULL};
static const char* ds2413ports[] = {"PIO.A","PIO.B",NULL};

/* possible output ports on actors */
__attribute__ ((unused)) static const char** actuator_ports[] = {ds2406ports,ds2408ports,ds2413ports};
#endif

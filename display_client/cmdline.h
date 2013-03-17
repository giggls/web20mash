#ifndef __cmdline__
#define __cmdline__
/*****
  command line parser interface -- generated by clig 
  (http://wsd.iitb.fhg.de/~geg/clighome/)

  The command line parser `clig':
  (C) 1995-2004 Harald Kirsch (clig@geggus.net)
*****/

typedef struct s_Cmdline {
  /***** -bd: run Program as a daemon in background */
  char daemonP;
  /***** -d: i2c device LED is connected to */
  char i2cdevP;
  char* i2cdev;
  int i2cdevC;
  /***** -g: gpio port for key */
  char gpioP;
  char* gpio;
  int gpioC;
  /***** -u: url for mashctld state */
  char urlP;
  char* url;
  int urlC;
  /***** -dbg: enable debug output */
  char debugP;
  /***** -a: i2c address of LED device */
  char i2caddrP;
  int i2caddr;
  int i2caddrC;
  /***** uninterpreted command line parameters */
  int argc;
  /*@null*/char **argv;
} Cmdline;


extern char *Program;
extern void usage(void);
extern /*@shared*/Cmdline *parseCmdline(int argc, char **argv);

#endif


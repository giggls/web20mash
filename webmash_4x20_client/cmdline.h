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
  /***** -k: gpio ports connected to keys (Menu, up, down, Enter) */
  char keysP;
  int *keys;
  int keysC;
  /***** -lcd: gpio ports for LCD */
  char lcdP;
  int *lcd;
  int lcdC;
  /***** -u: base url for mashctld state */
  char urlP;
  char* url;
  int urlC;
  /***** -dbg: enable debug output */
  char debugP;
  /***** -n: enable display of network interface information (IP, MAC, ...)] */
  char netinfoP;
  /***** -l: display language to use e.g. de_DE.UTF-8 */
  char langP;
  char* lang;
  int langC;
  /***** -b: Banner to be displayed e.g. mybrewery */
  char bannerP;
  char* banner;
  int bannerC;
  /***** -mc: base path of message catalog */
  char messagecatP;
  char* messagecat;
  int messagecatC;
  /***** -db: debounce delay for keys */
  char debounceP;
  int debounce;
  int debounceC;
  /***** -rs: electromagnetic sensitivity workaroud:\n          force display reset on stirring device state change */
  char ems_stirrerP;
  /***** -rh: electromagnetic sensitivity workaroud:\n          force display reset on heating device state change */
  char ems_heaterP;
  /***** uninterpreted command line parameters */
  int argc;
  /*@null*/char **argv;
} Cmdline;


extern char *Program;
extern void usage(void);
extern /*@shared*/Cmdline *parseCmdline(int argc, char **argv);

#endif


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <owcapi.h>
#include "owsensors.h"
#include "owactuators.h"

static void die(char* fmt, ...) {
  va_list ap;
    
  va_start(ap, fmt);
  vfprintf(stderr,fmt, ap);
  va_end(ap);
  exit(EXIT_FAILURE);
}

static int stringInArray(char * str, const char **arr) {
  const char **i;
  int c=0;
  for (i=arr; *i; i++) {
    if (0==strcmp(*i,str))
      return c;
    c++;
  }
  return -1;
}

static void outSensorActuatorList() {
  char buf[255];
  char *s1,*s2,*tok;
  const char **i;
  size_t slen1,slen2;
  int pos;

  OW_get("/",&s1,&slen1);
    
  tok=strtok(s1,",");
  printf("sensors:\n");
  while (tok != NULL) {
    /* if a sensor id is found scan for a supported sensor */
    if (tok[2] == '.') {
      sprintf(buf,"/%stype",tok);
      OW_get(buf,&s2,&slen2);
      pos=stringInArray(s2,sensors);
      if (-1 != pos) {
	tok[strlen(tok)-1]=0;
	printf("%s: %s\n",sensors[pos],tok);
      }
    }
    tok=strtok(NULL,",");
  }
  free(s1);
  free(s2);
  
  OW_get("/",&s1,&slen1);
  
  tok=strtok(s1,",");
  printf("\nactuators:\n");
  while (tok != NULL) {
    /* if an actuators id is found scan for type */
    if (tok[2] == '.') {
      sprintf(buf,"/%stype",tok);
      OW_get(buf,&s2,&slen2);
      pos = stringInArray(s2,actuators);
      if (-1 != pos) {
	tok[strlen(tok)-1]=0;
	printf("%s:",actuators[pos]);
	for (i=actuator_ports[pos]; *i; i++)
	  printf(" %s/%s",tok,*i);
        printf("\n");
      }
    }
    tok=strtok(NULL,",");
  }
  free(s1);
  free(s2);
}

int main(int argc, char **argv) {
  if (argc != 2)
    die("usage: %s owparms\n\nExamples:\n%s -u\n%s localhost:4304\n%s --i2c=/dev/i2c-1:18 --no_PPM\n",
        argv[0],argv[0],argv[0],argv[0]);

  if(OW_init(argv[1]) !=0)
    die("Error connecting owserver on %s\n",argv[1]);

  outSensorActuatorList();
  OW_finish();
  exit(EXIT_SUCCESS);
}
                  
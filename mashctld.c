/* 

mashctld

a web-controllable two-level temperature and mash process
controler for 1-wire sensor (DS18S20) and various actuators

(c) 2011 Sven Geggus <sven@geggus.net>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.

main source file

Here is how the mash process state machine for the 3-rest infusion
brewing process works. The first rest can be skipped by setting its
mesh time to 0:

state   function
0       no mash process running (generic control function)
1       raise the temperature up to protein rest (german: Eiweissrat)
2       keep temperature at protein rest temperature for configured time
3       raise the temperature up to 1. rest (german: Maltoserast)
4       keep temperature at 1. rest temperature for configured time
5       raise the temperature up to 2. rest (german: 2. Verzuckerungsrast)
6       keep temperature at 2. rest temperature for configured time
7       pseudo state for signaling end of mash process

*/


#include "mashctld.h"

#define PAGE "<html><head><title>File not found</title></head><body>File not found</body></html>"

			   char *indexfile="index.html";
magic_t magic_cookie;

static int acgen;

/* clig command line Parameters*/  
Cmdline *cmd;

/* runtime configuration file with full path */
char cfgfp[PATH_MAX + 1];    
  
struct configopts cfopts;
struct processstate pstate;

static void resetMashProcess() {
  pstate.mash=0;
  pstate.control=0;
  pstate.relais=0;
  pstate.tempMust=cfopts.tempMust;
  setRelais(0);                   
}

/* extract extension from filename */
const char *getExt (const char *fspec) {
  char *e = strrchr (fspec, '.');
  if (e == NULL)
    e = "";
  return e;
}

static ssize_t file_reader (void *cls, uint64_t pos, char *buf, size_t max) {
  FILE *file = cls;

  (void) fseek (file, pos, SEEK_SET);
  return fread (buf, 1, max, file);
}

static void free_file_callback (void *cls) {
  FILE *file = cls;
  fclose (file);
}

static ssize_t sync_response_generator (void *cls, uint64_t pos, char *buf, size_t max) {
  int * gp = cls;
  ssize_t ret=0;
  
  UNUSED(pos);

  if (*gp == -1)
    return MHD_CONTENT_READER_END_OF_STREAM;
  if (*gp == acgen) 
    return 0; /* no data */

  ret=snprintf (&buf[ret], max,
		"{\n  \"curtemp\": %5.1f,\n  \"musttemp\": %5.1f,\n  "
		"\"rstate\": %d,\n  \"ctrl\": %d,\n  \"mpstate\": %d,\n  "
		"\"resttimer\": %f,\n  "
                "\"resttime\": [ %lu, %lu, %lu ],\n  "
		"\"resttemp\": [ %.2f, %.2f, %.2f ]\n}\n",
  		pstate.tempCurrent,pstate.tempMust,
		pstate.relais,pstate.control,pstate.mash,pstate.resttime/60.0,
		cfopts.resttime[0], cfopts.resttime[1], cfopts.resttime[2],
		cfopts.resttemp[0], cfopts.resttemp[1], cfopts.resttemp[2]);

  *gp = -1;
    
  return ret;
}


static void sync_response_free_callback (void *cls) {
  free (cls);
}

static int answer_to_connection (void *cls,
				 struct MHD_Connection *connection,
				 const char *url,
				 const char *method,
				 const char *version,
				 const char *upload_data,
				 size_t *upload_data_size, void **ptr) {
  static int aptr;
  struct MHD_Response *response;
  int ret;
  FILE *file;
  int *gp;
  struct stat buf;
  static char mdata[1024];
  const char *relative_url;
  const char *magic_full;

  // available request types
  bool setctl,setmust,getstate,getfile,setmpstate,setrest,setactuator;

  // ignore explicitely unused parameters
  // eliminate compiler warnings
  UNUSED(cls);
  UNUSED(version);
  UNUSED(upload_data);
  UNUSED(upload_data_size);

  setmust=0;
  getstate=0;
  getfile=0;
  setctl=0;
  setmpstate=0;
  setrest=0;
  setactuator=0;

  if (0 != strcmp (method, MHD_HTTP_METHOD_GET))
    return MHD_NO;              /* unexpected method */
  if (&aptr != *ptr)
    {
      /* do never respond on first call */
      *ptr = &aptr;
      return MHD_YES;
    }
  *ptr = NULL;                  /* reset when done */


  if (0 == strncmp(url, "/getstate",9)) {
    getstate=1;
  } else {
    if (0 == strncmp(url, "/setmust/",9)) {
      setmust=1;
    } else if (0 == strncmp(url, "/setctl/",8)) {
      setctl=1;
    } else if (0 == strncmp(url, "/setmpstate/",12)) {
      setmpstate=1;
    } else if (0 == strncmp(url, "/setrest/",9)) {
      setrest=1;
    } else if (0 == strncmp(url, "/setactuator/",13)) {
      setactuator=1;
    } else {
      getfile=1;
    }
  }
  if (cmd->debugP)
    fprintf(stderr,"requested URL: %s\n",url);

  /* getstate is synchroniced to data acquisition */
  if (getstate) {
    gp = malloc(sizeof (int));
    *gp = acgen; /* current generation -- to only send rounds from connect on */
    if (NULL == gp)
      return MHD_NO;
    response = MHD_create_response_from_callback (MHD_SIZE_UNKNOWN, 
						  32 * 1024,     /* 32k page size */
						  &sync_response_generator,
						  gp,
						  &sync_response_free_callback);
    if (response == NULL) {
      free (gp);
      return MHD_NO;
    }

    MHD_add_response_header(response, "Content-Type", "application/json");                                  
    /* everything else should be delivered immediately */
  } else {
    
    if (setmust) {

      float must;
      if (1 != sscanf(url,"/setmust/%f",&must)) {
	if (cmd->debugP)
	  fprintf(stderr,"error setting must temperature\n");
	snprintf(mdata,1024,
		 "<html><body>Error setting must value!</body></html>");
      } else {
	if (must>MAXTEMP) must=MAXTEMP;
	if (must<MINTEMP) must=MINTEMP;
	pstate.tempMust=must;
	if (cmd->debugP)
	  fprintf(stderr,"setting must temperature to: %f\n",must);  
	//doControl();            
	snprintf(mdata,1024,
		 "<html><body>OK setting must value to %f</body></html>",must);
      }
      response = MHD_create_response_from_data(strlen(mdata),
					       (void*) mdata,
					       MHD_NO,
					       MHD_NO);
      MHD_add_response_header(response,
			      "Content-Type", "text/html; charset=UTF-8");
      
      ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
      return ret;
    }

    // enable or disable control function
    // this is only possible without a running mash process
    if (setctl) {

      int ctl;
      if ((1 != sscanf(url,"/setctl/%d",&ctl)) || (pstate.mash!=0)) {
	if (cmd->debugP)
	  fprintf(stderr,"error setting control\n");
	snprintf(mdata,1024,
		 "<html><body>Error setting control value!</body></html>");
      } else {
	pstate.control=ctl;
	// relais need to be off without control
	if (ctl==0) {
	  pstate.relais=0;
	};
	if (cmd->debugP)
	  fprintf(stderr,"setting control to: %d\n",ctl);  
	snprintf(mdata,1024,
		 "<html><body>OK setting control to %d</body></html>",ctl);
      }
      response = MHD_create_response_from_data(strlen(mdata),
					       (void*) mdata,
					       MHD_NO,
					       MHD_NO);
      MHD_add_response_header(response,
			      "Content-Type", "text/html; charset=UTF-8");
      
      ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
      return ret;
    }
     
    if (setactuator) {

      int state;
      if ((1 != sscanf(url,"/setactuator/%d",&state)) || (pstate.control!=0)) {
	if (cmd->debugP)
	  fprintf(stderr,"error setting actuator value\n");
	snprintf(mdata,1024,
		 "<html><body>Error setting actuator value!</body></html>");
      } else {
	setRelais(state);
	if (cmd->debugP)
	  fprintf(stderr,"setting actor to: %d\n",state);  
	snprintf(mdata,1024,
		 "<html><body>OK setting actor to %d</body></html>",state);
      }
      response = MHD_create_response_from_data(strlen(mdata),
					       (void*) mdata,
					       MHD_NO,
					       MHD_NO);
      MHD_add_response_header(response,
			      "Content-Type", "text/html; charset=UTF-8");
      
      ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
      return ret;
    }

    if (setrest) { 
      float val;
      int restno;
      char rtype[5];
      int ret;
      bool valid;
      
      valid=true;
      
      if (pstate.mash!=0) valid=false;
      if (3 != sscanf(url,"/setrest/%4s/%d/%f",rtype,&restno,&val)) {
        valid=false;
      } else {
        if ((0!=strcmp("temp",rtype)) && (0!=strcmp("time",rtype))) valid=false;
        if ((restno < 1) || (restno >3)) valid=false;
      }
      if (!valid) {
	if (cmd->debugP)
	  fprintf(stderr,"error setting resttime/resttemp value\n");
	snprintf(mdata,1024,
		 "<html><body>Error setting rest value, should be /setrest/&lt;time or temp&gt;/&lt;no&gt;/&lt;val&gt;</body></html>");
      } else {
        char key[8];
        sprintf(key,"rest%s%d",rtype,restno);
        // now we have to set resttime or resttemp
        if (0==strcmp("time",rtype)) {
          if (cmd->debugP)
            fprintf(stderr,"setting rest time %d to %u\n",restno,(unsigned) val);
          cfopts.resttime[restno-1]=(unsigned) val;
          ini_putl("mash-process", key, cfopts.resttime[restno-1], cfgfp);
          snprintf(mdata,1024,
		   "<html><body>OK setting rest time %d to %u</body></html>",restno,(unsigned) val);
        } else {
          if (cmd->debugP)
            fprintf(stderr,"setting rest temperature %d to %f\n",restno,val);
          cfopts.resttemp[restno-1]=val;
          ini_putf("mash-process", key, cfopts.resttemp[restno-1], cfgfp);
          snprintf(mdata,1024,
		   "<html><body>OK setting rest temperature %d to %f</body></html>",restno,val);
        }
      }
      response = MHD_create_response_from_data(strlen(mdata),
					       (void*) mdata,
					       MHD_NO,
					       MHD_NO);
      MHD_add_response_header(response,
			      "Content-Type", "text/html; charset=UTF-8");
      
      ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
      return ret;
    }

    if (setmpstate) {

      int mpstate=-1;
      int res;
      res=sscanf(url,"/setmpstate/%d",&mpstate);

      if ((1 != res) || (mpstate >7) || (mpstate <0)) {
	if (cmd->debugP)
	  fprintf(stderr,"error setting mash process state to %d\n",mpstate);
	snprintf(mdata,1024,
		 "<html><body>Error setting mash process state to %d!</body></html>",mpstate);
      } else {
	if (cmd->debugP)
	  fprintf(stderr,"setting mash process state to: %d\n",mpstate);  

	pstate.mash=mpstate;
	// if mash process is set to 0 control needs to be turned of as well
	// otherwise we need control to be turned on
	if (mpstate == 0) {
	  resetMashProcess();
        } else {
          pstate.control=1;

          // on even state we need to set starttime
          if (!(mpstate % 2)) {
	    pstate.starttime=time(NULL);
	    unsigned index;
	    index=(mpstate-1)/2;
	    if (cmd->debugP)
	      fprintf(stderr,"setting timer for rest%d to %ld minutes\n",index+1,cfopts.resttime[index]);
          }
	}

	snprintf(mdata,1024,
		 "<html><body>OK setting mash process state to %d</body></html>",mpstate);
      }
      response = MHD_create_response_from_data(strlen(mdata),
					       (void*) mdata,
					       MHD_NO,
					       MHD_NO);
      MHD_add_response_header(response,
			      "Content-Type", "text/html; charset=UTF-8");
      
      ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
      return ret;
    }

    if (strlen(url)==1 && url[0]=='/') {
      relative_url=indexfile;  
    } else {
      int i=0;
      /* remove leading / from url */
      while (url[i]=='/') i++;
      relative_url=url+i; 
    }
    if (cmd->debugP)
      fprintf(stderr,"trying to open file: %s\n",relative_url);
    if ((0 == stat(relative_url, &buf)) && (S_ISREG (buf.st_mode)) )
      file = fopen(relative_url, "rb");
    else
      file = NULL;
 
    if (file == NULL) {
      response = MHD_create_response_from_buffer (strlen (PAGE),
						  (void *) PAGE,
						  MHD_RESPMEM_PERSISTENT);
      ret = MHD_queue_response (connection, MHD_HTTP_NOT_FOUND, response);
      MHD_destroy_response (response);
      return ret;
    } else {
      file = fopen (relative_url, "rb");
      response = MHD_create_response_from_callback (buf.st_size,
						    32 * 1024,     /* 32k page size */
						    &file_reader,
						    file,
						    &free_file_callback);
      if (response == NULL) {
	free (file);
	return MHD_NO;
      }

      /* mime types we know better than libmagic are for .js and .css */
      if (strncmp(getExt(relative_url),".js",3)==0) {
	if (cmd->debugP)
	  fprintf(stderr,"setting mime type to %s\n", "text/javascript");
	MHD_add_response_header(response,"Content-Type", "text/javascript");
      } else {
	if (strncmp(getExt(relative_url),".css",4)==0) {
	  if (cmd->debugP)
	    fprintf(stderr,"setting mime type to %s\n", "text/css");
	  MHD_add_response_header(response,"Content-Type", "text/css");
	} else {
	  /* use libmagic for the rest */
	  magic_full = magic_file(magic_cookie, relative_url);
	  if (cmd->debugP)
	    fprintf(stderr,"setting mime type to %s\n", magic_full);
	  MHD_add_response_header(response,"Content-Type", magic_full);
	}
      }
    }
  }

  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);    
  return ret;
}

/* cyclically called control and state machine function */
void acq_and_ctrl() {
  uint64_t endtime;
  static bool expired;

  /* acquire temperature */
  pstate.tempCurrent=getTemp();

  /* if mash process is in running adjust process parameters */
  if ((pstate.mash>0) && (pstate.mash<7)) {
    unsigned index;
    
    index=(pstate.mash-1)/2;
    if (index <3) {
      pstate.tempMust=cfopts.resttemp[index];  
    }
    expired=false;

    if (pstate.mash % 2) {
      pstate.resttime=0;
      if (pstate.tempCurrent >= pstate.tempMust) {
	pstate.starttime=time(NULL);
	if (cmd->debugP)
	  fprintf(stderr,"setting timer for rest%d to %ld minutes\n",index+1,cfopts.resttime[index]);
        pstate.resttime=60*cfopts.resttime[index];
	pstate.mash++;
      }
    } else {
      endtime=pstate.starttime+(60*cfopts.resttime[index]);
      pstate.resttime=endtime-time(NULL);
      if (pstate.resttime <= 0) {
	if (cmd->debugP)
	  fprintf(stderr,"timer for rest%d expired\n",index+1);
	pstate.resttime=0;
	pstate.mash++;
      }
    }
  }

  if (expired) {
    resetMashProcess();
  }

  if (pstate.mash==7) {
    expired=true;
  }

  /* Run two-level control if desired */
  if (pstate.control)
    doControl();

  if (cmd->debugP) {
    if (pstate.mash) {
      fprintf(stderr,"temp: must:%5.1f cur:%5.1f (relais:%d, control:%d, mash:%d, timer: %.2f)\n",
	      pstate.tempMust,pstate.tempCurrent,pstate.relais,pstate.control,
	      pstate.mash, pstate.resttime/60.0);
    } else {
      fprintf(stderr,"temp: must:%5.1f cur:%5.1f (relais:%d, control:%d)\n",
	      pstate.tempMust,pstate.tempCurrent,pstate.relais,pstate.control);
    }
  }
}

int main(int argc, char **argv) {
  struct MHD_Daemon *d;
  fd_set rs;
  fd_set ws;
  fd_set es;
  int max,rtcfd;
  unsigned long data;
  int tirq,acqdelay=4;
  FILE *cfile;
  
  cmd = parseCmdline(argc, argv);
  
  /* parse the configfile if available and readable */
  cfile=fopen(cmd->configfile,"r");
  if (cfile==NULL) {
    fprintf(stderr,"error reading configfile >%s<, using defaults!\n",cmd->configfile);
  } else {
    fclose(cfile);
    realpath(cmd->configfile, cfgfp);
  }
  readconfig(cfgfp);
  
  // default is no control mash process off
  pstate.control=0;
  pstate.mash=0;
  pstate.tempMust=cfopts.tempMust;
  pstate.resttime=0;
  pstate.ttrigger=0;

  if(OW_init(cfopts.owparms) !=0) {
    fprintf(stderr,"Error connecting owserver on %s\n",cfopts.owparms);
    exit(EXIT_FAILURE);
  }
  
  if (cmd->listP) {
    printSensorActuatorList();
    OW_finish();
    exit(EXIT_SUCCESS);
  }
  
  /* check if requested sensor is available on the bus */
  if (false==search4Device(cfopts.sensor,"DS18S20")) {
    fprintf(stderr,"%s is unavailable or not a DS18S20 sensor\n",cfopts.sensor);
    exit(EXIT_FAILURE);
  }

  if (cfopts.extactuator==false) {
    /* check if requested 1-wire actuator is available on the bus */
    if (false==search4Device(cfopts.actuator,"DS2406")) {
      fprintf(stderr,"%s is unavailable or not a DS2406 actuator\n",cfopts.actuator);
      exit(EXIT_FAILURE);
    }
  }
  
  if (-1==chdir(cfopts.webroot)) {
    fprintf(stderr,"Unable to chdir to >%s<\n",cfopts.webroot);
    exit(EXIT_FAILURE);
  }
 
  /* initialize libmagic */
  magic_cookie = magic_open(MAGIC_MIME);
  if (magic_cookie == NULL) {
    printf("unable to initialize magic library\n");
    exit(EXIT_FAILURE);
  }
  if (magic_load(magic_cookie, NULL) != 0) {
    printf("cannot load magic database - %s\n", magic_error(magic_cookie));
    magic_close(magic_cookie);
    exit(EXIT_FAILURE);
  }

  d = MHD_start_daemon(MHD_NO_FLAG,
		       cfopts.port,
		       NULL, NULL, &answer_to_connection, PAGE, MHD_OPTION_END);
  if (d == NULL)
    return 1;

  rtcfd = open(cfopts.rtcdev, O_RDONLY);
  if (rtcfd ==  -1) {
    fprintf(stderr,"error opening %s\n",cfopts.rtcdev);
    exit(EXIT_FAILURE);
  }
  /* Turn on update interrupts (one per second) */
  ioctl(rtcfd, RTC_UIE_ON, 0);

  tirq=acqdelay-1;
  setRelais(0);

  acq_and_ctrl();
  while (1) {
    max = 0;
    FD_ZERO (&rs);
    FD_ZERO (&ws);
    FD_ZERO (&es);

    if (MHD_YES != MHD_get_fdset (d, &rs, &ws, &es, &max))
      break; /* fatal internal error */

    FD_SET(rtcfd,&rs);
    if (rtcfd >max) max=rtcfd;
    
    select (max + 1, &rs, &ws, &es, NULL);
    if (FD_ISSET(rtcfd,&rs)) {
      read(rtcfd, &data, sizeof(unsigned long));
      tirq++;
      if (tirq==acqdelay) {
	acgen++; /* count up generation to trigger transmission */
	acq_and_ctrl();
	tirq=0;
      }
    }
    /* NOTE: *always* run MHD_run() in external select loop! */
    MHD_run (d);    
  }
  MHD_stop_daemon (d);
  return 0;
}

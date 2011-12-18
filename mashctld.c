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
static bool terminate=false;
static bool isdaemon=false;

/* clig command line Parameters*/  
Cmdline *cmd;

/* runtime configuration file with full path */
char cfgfp[PATH_MAX + 1];    
  
struct configopts cfopts;
struct processstate pstate;

static void resetMashProcess() {
  pstate.mash=0;
  pstate.control=0;
  pstate.relay=0;
  pstate.tempMust=cfopts.tempMust;
  setRelay(0);                   
}

const char* actuatorname[2] = {"cooler", "heater"};

void debug(char* fmt, ...) {
  va_list ap;
  
  va_start(ap, fmt);
  if (isdaemon)
    vsyslog(LOG_DEBUG, fmt, ap);
  else
    vfprintf(stderr,fmt, ap);
  va_end(ap);
}

void die(char* fmt, ...) {
  va_list ap;
  
  va_start(ap, fmt);
  if (isdaemon)
    vsyslog(LOG_ERR, fmt, ap);
  else
    vfprintf(stderr,fmt, ap);
  va_end(ap);
  exit(EXIT_FAILURE);
}

void errorlog(char* fmt, ...) {
  va_list ap;
    
  va_start(ap, fmt);
  if (isdaemon)
    vsyslog(LOG_ERR, fmt, ap);
  else
    vfprintf(stderr,fmt, ap);
  va_end(ap);
}

void signalHandler() {
  fflush(stdout);
  terminate=true;
}

void daemonize() {
  if (fork()!=0) exit(0);
  setsid();
  if (fork()!=0) exit(0);
  umask(0);
  close(0);
  close(1);
  close(2);
  /*STDIN*/
  open("/dev/null",O_RDONLY);
  /*STDOUT*/
  open("/dev/null",O_WRONLY);
  /*STDERR*/
  open("/dev/null",O_WRONLY);
}

/* extract extension from filename */
const char *getExt (const char *fspec) {
  char *e = strrchr (fspec, '.');
  if (e == NULL)
    e = "";
  return e;
}

static void strtolower(char* str, unsigned len) {
  unsigned i;
  for (i=0;i<len;i++) {
    str[i]=tolower(str[i]);
  }
}

// determine url name to be used i18n version
char *getintlname(const char *alang, const char *ourl, char *nurl[]) {
  struct stat buf;
  char *myalang;
  char *url;
  char *tok;
  
  url=*nurl;
  myalang=malloc((strlen(alang)+1)*sizeof(char));
  strcpy(myalang,alang);
  tok=strtok(myalang,",");
  while (tok != NULL) {
    char *tail;
    strtolower(tok,strlen(tok));
    tail=url+strlen(ourl);
    tail[0]='.';
    strncpy(tail+1,tok,3);
    tail[3]='\0';
    if (cmd->debugP)
      debug("getintlname: looking for file: %s\n",url);
    if ((0 == stat(url, &buf)) && (S_ISREG (buf.st_mode)) ) {
      if (cmd->debugP)
	printf("getintlname: OK, found file %s\n",url);
      break;
    } else {
      if (cmd->debugP)
	printf("getintlname: no file named: %s\n",url);
    }
    tok=strtok(NULL,",");
  }
  if (tok==NULL) {
    char *tail;
    if (cmd->debugP)
      debug("getintlname: requested languages unavailable using default: %s\n",url,DEFAULTLANG);
    tail=url+strlen(ourl);
    tail[0]='.';
    strncpy(tail+1,DEFAULTLANG,3);
    tail[3]='\0';	  
  }
  free(myalang);
  return(url);
}

static ssize_t file_reader (void *cls, uint64_t pos, char *buf, size_t max) {
  FILE *file = cls;

  (void) fseek (file, pos, SEEK_SET);
  return fread (buf, 1, max, file);
}

static void free_file_callback (void *cls) {
  FILE *file = cls;
  fclose(file);
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
		"\"acttype\": \"%s\",\n  "
		"\"resttimer\": %f,\n  "
                "\"resttime\": [ %ju, %ju, %ju ],\n  "
		"\"resttemp\": [ %.2f, %.2f, %.2f ]\n}\n",
  		pstate.tempCurrent,pstate.tempMust,
		pstate.relay,pstate.control,pstate.mash,
		actuatorname[cfopts.acttype],
		pstate.resttime/60.0,
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
  char *nurl;
  char *user;
  char *pass;
  int fail=0;

  // available request types
  bool setctl,setmust,getstate,getfile,setmpstate,setrest,setactuator,setallmash,setacttype;

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
  setallmash=0;
  setacttype=0;

  if (0 != strcmp (method, MHD_HTTP_METHOD_GET))
    return MHD_NO;              /* unexpected method */
  if (&aptr != *ptr)
    {
      /* do never respond on first call */
      *ptr = &aptr;
      return MHD_YES;
    }
  *ptr = NULL;                  /* reset when done */

  if (cfopts.authactive) {
    pass = NULL;
    user = MHD_basic_auth_get_username_password (connection, &pass);
    fail = ( (user == NULL) ||
	     (0 != strcmp (user, cfopts.username)) ||
	     (0 != strcmp (pass, cfopts.password)) );  
    if (user != NULL) free (user);
    if (pass != NULL) free (pass);
  }
  if (fail) {
    const char *page = "<html><body>Go away.</body></html>";
    response = MHD_create_response_from_buffer (strlen (page), (void *) page, 
						MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_basic_auth_fail_response (connection,"Web 2.0 Mash", response);
  } else {

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
      } else if (0 == strncmp(url, "/setallmash/",12)) {
	setallmash=1;
      } else if (0 == strncmp(url, "/setacttype/",12)) {
	setacttype=1;
      } else {
	getfile=1;
      }
    }
    if (cmd->debugP)
      debug("requested URL: %s\n",url);

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
	if ((1 != sscanf(url,"/setmust/%f",&must)) || (pstate.mash!=0)) {
	  if (cmd->debugP)
	    debug("error setting must temperature\n");
	  snprintf(mdata,1024,
		   "<html><body>Error setting must value!</body></html>");
	} else {
	  if (must>MAXTEMP) must=MAXTEMP;
	  if (must<MINTEMP) must=MINTEMP;
	  if (pstate.tempMust!=must) {
	    if (cmd->debugP)
	      debug("updating inifile must temperature: %f\n",must);
	    ini_putf("control","tempMust", must, cfgfp);
	  }
	  pstate.tempMust=must;
	  if (cmd->debugP)
	    debug("setting must temperature to: %f\n",must);  
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
	    debug("error setting control\n");
	  snprintf(mdata,1024,
		   "<html><body>Error setting control value!</body></html>");
	} else {
	  pstate.control=ctl;
	  // relay need to be off without control
	  if (ctl==0) {
	    setRelay(0);
	    pstate.relay=0;
	  };
	  if (cmd->debugP)
	    debug("setting control to: %d\n",ctl);  
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
   
      /* manually change actuator value
         this only works when control is turned off */
      if (setactuator) {

	int astate;
	bool valid;

	if ((1 != sscanf(url,"/setactuator/%d",&astate)) || (pstate.control!=0)) {
	  valid=false;
	} else {
	  if ((astate == 0) || (astate == 1)) {
	    valid=true;
	  } else {
	    valid=false;
	  }
	}
	
	if (valid==false) {
	  if (cmd->debugP)
	    debug("error setting actuator state\n");
	  snprintf(mdata,1024,
		   "<html><body>Error setting actuator state!</body></html>");
	} else {
	  pstate.relay=astate;
	  setRelay(astate);
	  if (cmd->debugP)
	    debug("setting actuator state to: %d\n",astate);  
	  snprintf(mdata,1024,
		   "<html><body>OK setting actuator state to %d</body></html>",astate);
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

      /* set actuator type ("cooler" or "heater")
         "cooler" conflicts with mash-control but can for example be used for
         controling a fridge during fermentation of lager beers */
      if (setacttype) {
	char sacttype[7];
	int acttype;
	bool valid;

	strncpy(sacttype,url+12,6);
	sacttype[6]='\0';
	if (strcmp(sacttype,"heater")==0) {
	  valid=true;
	  acttype=ACT_HEATER;
	} else {
	  if (strcmp(sacttype,"cooler")==0) {
	    valid=true;
	    acttype=ACT_COOLER;
	  } else {
	    valid=false;
	  }
	}

	if ((valid==false) || (pstate.control!=0)) {
	  if (cmd->debugP)
	    debug("error setting actuator value\n");
	  snprintf(mdata,1024,
		   "<html><body>Error setting actuator value!</body></html>");
	} else {
	  if (cfopts.acttype!=acttype) {
	    cfopts.acttype=acttype;
	    if (cmd->debugP)
	      debug("updating inifile actuatortype: %s\n",sacttype);
	    ini_puts("control", "actuatortype", sacttype, cfgfp);
	  }
	  if (cmd->debugP)
	    debug("setting actuator to: %s\n",sacttype);  
	  snprintf(mdata,1024,
		   "<html><body>OK setting actuator to %s</body></html>",sacttype);
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

      // set rest temperature or rest time
      // Syntax: /setrest/<temp>or<time>/<nr>/<value>
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
	    debug("error setting resttime/resttemp value\n");
	  snprintf(mdata,1024,
		   "<html><body>Error setting rest value, should be /setrest/&lt;time or temp&gt;/&lt;no&gt;/&lt;val&gt;</body></html>");
	} else {
	  char key[10];
	  sprintf(key,"rest%s%d",rtype,restno);
	  // now we have to set resttime or resttemp
	  if (0==strcmp("time",rtype)) {
	    if (cmd->debugP)
	      debug("setting rest time %d to %u\n",restno,(unsigned) val);
	    if (val>MAXTIME) val=MAXTIME;
	    cfopts.resttime[restno-1]=(unsigned) val;
	    if (cmd->debugP)
	      debug("updateing ini-file %s with value %u\n",key,cfopts.resttime[restno-1]);
	    ini_putl("mash-process", key, cfopts.resttime[restno-1], cfgfp);
	    snprintf(mdata,1024,
		     "<html><body>OK setting rest time %d to %u</body></html>",restno,(unsigned) val);
	  } else {
	    if (cmd->debugP)
	      debug("setting rest temperature %d to %f\n",restno,val);
	    if (val>MAXTEMP) val=MAXTEMP;
	    if (val<MINTEMP) val=MINTEMP;
	    cfopts.resttemp[restno-1]=val;
	    if (cmd->debugP)
	      debug("updateing ini-file %s with value %f\n",key,cfopts.resttime[restno-1]);
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

      // set al values at once: musttemp, resttime(1-3) and resttemp(1-3=
      // Syntax: /setallmash/<time1>/<temp1>/<time2>/<temp2>/<time3>/<temp3>
      if (setallmash) { 
	float vtemp[3];
	unsigned vtime[3];
	int ret;
	bool valid;
      
	valid=true;
      
	if (pstate.mash!=0) valid=false;
	if (6 != sscanf(url,"/setallmash/%u/%f/%u/%f/%u/%f",
			&vtime[0],&vtemp[0],&vtime[1],&vtemp[1],&vtime[2],&vtemp[2])) {
	  valid=false;
	}
	if (!valid) {
	  if (cmd->debugP)
	    debug("error setting all rest and control values values\n");
	  snprintf(mdata,1024,
		   "<html><body>Error setting all rest and control values<br />"
		   "Syntax: /setallmash/&lt;time1&gt;/&lt;temp1&gt;/&lt;time2&gt;/&lt;temp2&gt;/&lt;time3&gt;/&lt;temp3&gt;"
		   "</body></html>");
	} else {
	  int i;
	  char key[10];
	  // now we have to adjust all requested settings
	  for (i=0;i<3;i++) {
	    sprintf(key,"resttime%d",i+1);
	    if (vtime[i]>MAXTIME) vtime[i]=MAXTIME;
	    if (cfopts.resttime[i]!=vtime[i]) {
	      if (cmd->debugP)
		debug("updateing ini-file %s with value %u\n",key,vtime[i]);
	      ini_putl("mash-process", key, vtime[i],cfgfp);
	    }
	    cfopts.resttime[i]=vtime[i];
	    sprintf(key,"resttemp%d",i+1);
	    if (vtemp[i]>MAXTEMP) vtemp[i]=MAXTEMP;
	    if (vtemp[i]<MINTEMP) vtemp[i]=MINTEMP;
	    if (cfopts.resttemp[i]!=vtemp[i]) {
	      if (cmd->debugP)
		debug("updateing ini-file %s with value %f\n",key,vtemp[i]);
	      ini_putf("mash-process", key, vtemp[i],cfgfp);
	    }
	    cfopts.resttemp[i]=vtemp[i];          
	  }            
          snprintf(mdata,1024,
		   "<html><body>OK setting all rest and control values</body></html>");
	  if (cmd->debugP)
	    debug("OK calling /setallmash/%u/%f/%u/%f/%u/%f\n",vtime[0],vtemp[0],vtime[1],vtemp[1],vtime[2],vtemp[2]);
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
	    debug("error setting mash process state to %d\n",mpstate);
	  snprintf(mdata,1024,
		   "<html><body>Error setting mash process state to %d!</body></html>",mpstate);
	} else {
	  if (cmd->debugP)
	    debug("setting mash process state to: %d\n",mpstate);  

	  pstate.mash=mpstate;
	  // if mash process is set to 0 control needs to be turned of as well
	  // otherwise we need control to be turned on
	  if (mpstate == 0) {
	    resetMashProcess();
	  } else {
	    // in any mash process state we need to use a heater as actuator
	    cfopts.acttype=ACT_HEATER;
	    pstate.control=1;

	    // on even state we need to set starttime
	    if (!(mpstate % 2)) {
	      pstate.starttime=time(NULL);
	      unsigned index;
	      index=(mpstate-1)/2;
	      if (cmd->debugP)
		debug("setting timer for rest%d to %jd minutes\n",index+1,cfopts.resttime[index]);
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

      nurl=NULL;
      /* if clause is a poor man's i18n, in else clause we have no i18n at all 
	 evaluate just the first two digits of a language string ignoring
	 anything else
      */
      if (strncmp(getExt(relative_url),".html",5)==0) {
	const char *alang;

	alang=MHD_lookup_connection_value(connection, MHD_HEADER_KIND, MHD_HTTP_HEADER_ACCEPT_LANGUAGE);
	/* no language header given */
	if (alang==NULL) {
	  if (cmd->debugP)
	    debug("no language header in http-request setting default: %s\n",DEFAULTLANG);
	  alang=DEFAULTLANG;
	}
	nurl=malloc(sizeof(char)*(strlen(relative_url)+4));
	strcpy(nurl,relative_url);
	relative_url=getintlname(alang,relative_url,&nurl);
      }
	
      if (cmd->debugP)
	debug("trying to open file: %s\n",relative_url);
      if ((0 == stat(relative_url, &buf)) && (S_ISREG (buf.st_mode)) )
	file = fopen(relative_url, "rb");
      else
	file = NULL;

      if (file == NULL) {
	if (NULL!=nurl) free(nurl);
	response = MHD_create_response_from_buffer (strlen (PAGE),
						    (void *) PAGE,
						    MHD_RESPMEM_PERSISTENT);
	ret = MHD_queue_response (connection, MHD_HTTP_NOT_FOUND, response);
	MHD_destroy_response (response);
	return ret;
      } else {
	response = MHD_create_response_from_callback (buf.st_size,
						      32 * 1024,     /* 32k page size */
						      &file_reader,
						      file,
						      &free_file_callback);
	if (response == NULL) {
	  fclose(file);
	  return MHD_NO;
	}

	/* mime types we know better than libmagic are for .js and .css */
	if (strncmp(getExt(relative_url),".js",3)==0) {
	  if (cmd->debugP)
	    debug("setting mime type to %s\n", "text/javascript");
	  MHD_add_response_header(response,"Content-Type", "text/javascript");
	} else {
	  if (strncmp(getExt(relative_url),".css",4)==0) {
	    if (cmd->debugP)
	      debug("setting mime type to %s\n", "text/css");
	    MHD_add_response_header(response,"Content-Type", "text/css");
	  } else {
	    /* use libmagic for the rest */
	    magic_full = magic_file(magic_cookie, relative_url);
	    if (cmd->debugP)
	      debug("setting mime type to %s\n", magic_full);
	    MHD_add_response_header(response,"Content-Type", magic_full);
	  }
	}
	if (NULL!=nurl) free(nurl);
      }
    }

    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  }
  MHD_destroy_response (response);    
  return ret;
}

static float get_elapsed_time(void) {
  static struct timespec start;
  struct timespec curr;
  static int first_call = 1;
  int secs, nsecs;

  if (first_call) {
    first_call = 0;
    if (clock_gettime(CLOCK_MONOTONIC, &start) == -1)
      die("error calling clock_gettime\n");
  }

  if (clock_gettime(CLOCK_MONOTONIC, &curr) == -1)
    die("error calling clock_gettime\n");

  secs = curr.tv_sec - start.tv_sec;
  nsecs = curr.tv_nsec - start.tv_nsec;

  if (!first_call)
    start=curr;
  
  return((nsecs+secs*1000000000.0)/1000000000.0);
}


int init_timerfd(int seconds) {
  int fd;
  struct timespec now;
  struct itimerspec new_value;
  
  if (clock_gettime(CLOCK_REALTIME, &now) == -1)
    die("error in clock_gettime\n");

  fd = timerfd_create(CLOCK_REALTIME, 0);
  if (fd == -1)
    die("error calling  timerfd_create\n");
    
  /* Create a CLOCK_REALTIME absolute timer */
  new_value.it_value.tv_sec = now.tv_sec + seconds;
  new_value.it_value.tv_nsec = now.tv_nsec;
  new_value.it_interval.tv_sec = seconds;
  new_value.it_interval.tv_nsec = 0;

  if (timerfd_settime(fd, TFD_TIMER_ABSTIME, &new_value, NULL) == -1)
    die("error calling timerfd_settime\n");
  
  return(fd);
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
    if (index <3) pstate.tempMust=cfopts.resttemp[index];  
    expired=false;

    if (pstate.mash % 2) /* power heating until rest is reached */ {
      pstate.resttime=0;
      // condition to start rest
      if (pstate.tempCurrent >= pstate.tempMust) {
	pstate.starttime=time(NULL);
	/* this is a special case here:
	   if resttime is 0 we need to set tempMust to the value of the
	   next rest to skip the rest and prevent the heating relay from
	   flickering */
	if ((index <2) && (cfopts.resttime[index] == 0)) {
	  pstate.tempMust=cfopts.resttemp[index+1];
	}
	if (cmd->debugP)
	  debug("setting timer for rest%d to %jd minutes\n",index+1,cfopts.resttime[index]);
        pstate.resttime=60*cfopts.resttime[index];
	pstate.mash++;
      }
    } else /* rest states */ {
      endtime=pstate.starttime+(60*cfopts.resttime[index]);
      pstate.resttime=endtime-time(NULL);
      // rest until timer expired
      if (pstate.resttime <= 0) {
	if (cmd->debugP)
	  debug("timer for rest%d expired\n",index+1);
        // set must temperature to the next value
        if ((index <2)) {
          pstate.tempMust=cfopts.resttemp[index+1];
        }
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
      debug("clock: %.02f temp: must:%5.1f cur:%5.1f (relay:%d, control:%d, mash:%d, timer: %.2f)\n",
	    get_elapsed_time(), pstate.tempMust,pstate.tempCurrent,pstate.relay,pstate.control,
	    pstate.mash, pstate.resttime/60.0);
    } else {
      debug("clock: %.02f temp: must:%5.1f cur:%5.1f (relay:%d, control:%d)\n",
	    get_elapsed_time(),pstate.tempMust,pstate.tempCurrent,pstate.relay,pstate.control);
    }
  }
}

int main(int argc, char **argv) {
  struct MHD_Daemon *dv4;
  struct MHD_Daemon *dv6;
  fd_set rs;
  fd_set ws;
  fd_set es;
  int max,timfd;
  uint64_t exp;
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

  if(OW_init(cfopts.owparms) !=0)
    die("Error connecting owserver on %s\n",cfopts.owparms);
  
  if (cmd->listP) {
    printSensorActuatorList();
    OW_finish();
    exit(EXIT_SUCCESS);
  }
  
  /* check if requested sensor is available on the bus */
  if (false==search4Device(cfopts.sensor,"DS18S20"))
    die("%s is unavailable or not a DS18S20 sensor\n",cfopts.sensor);

  if (cfopts.extactuator==false) {
    /* check if requested 1-wire actuator is available on the bus */
    if (false==search4Device(cfopts.actuator,"DS2406"))
      die("%s is unavailable or not a DS2406 actuator\n",cfopts.actuator);
  }
  
  if (-1==chdir(cfopts.webroot))
    die("Unable to chdir to >%s<\n",cfopts.webroot);
 
  /* initialize libmagic */
  magic_cookie = magic_open(MAGIC_MIME);
  if (magic_cookie == NULL)
    die("unable to initialize magic library\n");

  if (magic_load(magic_cookie, NULL) != 0) {
    magic_close(magic_cookie);
    die("cannot load magic database - %s\n", magic_error(magic_cookie));
  }

  dv6 = MHD_start_daemon(MHD_USE_IPv6,
		         cfopts.port,
		         NULL, NULL, &answer_to_connection, PAGE, MHD_OPTION_END);
  if (dv6 == NULL)
    if (cmd->debugP)
      debug("Error running IPv6 HTTP-server\n");
    
  dv4 = MHD_start_daemon(MHD_NO,
                         cfopts.port,
                         NULL, NULL, &answer_to_connection, PAGE, MHD_OPTION_END);
  
  if (dv4 == NULL)
    if (cmd->debugP)
      debug("Error running IPv4 HTTP-server\n");
  
  if ((dv4 == NULL) && (dv6 == NULL))
    die("error starting http server\n");


  timfd=init_timerfd(DELAY);
  if (cmd->debugP)
    get_elapsed_time();

  setRelay(0);

  signal(SIGINT,signalHandler);
  signal(SIGTERM,signalHandler);
  
  if (cmd->daemonP) {
    isdaemon=true;
    openlog(Program,LOG_PID,LOG_DAEMON);
    daemonize();
  }
  acq_and_ctrl();
  
  while (1) {
    if (terminate) {
      setRelay(0);
      break;
    }
    max = 0;
    FD_ZERO (&rs);
    FD_ZERO (&ws);
    FD_ZERO (&es);

    if (dv4!=NULL)
      if (MHD_YES != MHD_get_fdset (dv4, &rs, &ws, &es, &max))
        break; /* fatal internal error */
      
    if (dv6!=NULL)
      if (MHD_YES != MHD_get_fdset (dv6, &rs, &ws, &es, &max))
        break;

    FD_SET(timfd,&rs);
    if (timfd >max) max=timfd;
    
    select (max + 1, &rs, &ws, &es, NULL);
    if (FD_ISSET(timfd,&rs)) {
      read(timfd, &exp, sizeof(uint64_t));
      acgen++; /* count up generation to trigger transmission */
      acq_and_ctrl();
    }
    /* NOTE: *always* run MHD_run() in external select loop! */
    if (dv4!=NULL) MHD_run(dv4);
    if (dv6!=NULL) MHD_run(dv6);
    
  }
  if (dv4!=NULL) MHD_stop_daemon(dv4);
  if (dv6!=NULL) MHD_stop_daemon(dv6);
  return 0;
}

/* 

display_client

First version of a non Webbrowser client software vor mashctld

I is currently unable to do anything but to display the current
temperature on adafruit 7-segment LED HT16k33 Backpack

However it should be easy to extend to allow connecting key-actions
via gpio.

(c) 2013 Sven Geggus <sven-web20mash@geggus.net>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY;without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.

*/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include "cmdline.h"
#include "7-segm-ht16k33.h"

/* curl stuff */
#include <curl/curl.h>
/* simple json parser */
#include "jsmn.h"
#define jsmnTOKnum	50

#define DEBUG 1

#define DISPCUR 0
#define DISPMUST 1

int dispval=DISPCUR;
char curtemp[]="---.-";
char musttemp[]="---.-";

static bool isdaemon=false;

static int i2cfd;

/* clig command line Parameters*/  
Cmdline *cmd;

void debug(char* fmt, ...) {
  va_list ap;
  
  if (cmd->debugP) {
    va_start(ap, fmt);
    if (isdaemon)
      vsyslog(LOG_DEBUG, fmt, ap);
    else
      vfprintf(stderr,fmt, ap);
    va_end(ap);
  }
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

static size_t updateDisplayCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  size_t realsize = size * nmemb;
  static size_t buflen=0;
  static char *buf;

  // jsmn stuff
  jsmn_parser parser;
  jsmntok_t tokens[jsmnTOKnum] ;
  int token_index;
  bool is_curtemp=false;
  bool is_musttemp=false;

  (void)userp;

  // make shure buffer is always large enough
  if ((realsize+1)>buflen) {
    buflen=realsize+1;
    buf=realloc(buf,realsize+1);
  }
 // copy content to a proper 0-terminated string
  memcpy(buf,contents,realsize);
  buf[realsize]='\0';
  
  debug("-------------------- JSON:\n%s--------------------------\n",buf);

  jsmn_init(&parser);
  switch ( jsmn_parse( &parser, buf, tokens, jsmnTOKnum ) ) {
  case JSMN_ERROR_NOMEM:
    debug("Not enough JSON tokens were provided");
    break ;
  case JSMN_ERROR_INVAL:
    debug("Invalid character inside JSON string");
    break ;
  case JSMN_ERROR_PART:
    debug("The string is not a full JSON packet, more bytes expected");
    break ;
  case JSMN_SUCCESS:
    break ;
  }
  for ( token_index = 0 ; token_index < parser.toknext ; ++token_index ) {
    int length = tokens[token_index].end - tokens[token_index].start ;
    char * tok_start = &buf[tokens[token_index].start] ;
    char temp_null = tok_start[length] ;
    tok_start[length] = '\0' ; // null terminate temporarily

    switch ( tokens[token_index].type ) {
    case JSMN_PRIMITIVE:
      //printf("Primative %s\n",tok_start);
      if (is_curtemp) {
	is_curtemp=false;
	debug("read curtemp value: %s\n",tok_start);
	strncpy(curtemp,tok_start,5);
	curtemp[5]='\0';
	if (dispval==DISPCUR) 
	  update_ht16k33_7segm_display(tok_start,false,i2cfd);
      }
      if (is_musttemp) {
	is_musttemp=false;
	debug("read musttemp value: %s\n",tok_start);
	strncpy(musttemp,tok_start,5);
	musttemp[5]='\0';
	if (dispval==DISPMUST)
	  update_ht16k33_7segm_display(tok_start,true,i2cfd);
      }
      break ;
    case JSMN_OBJECT:
      //printf("Object %s\n",tok_start);
      break ;
    case JSMN_ARRAY:
      //printf("Array %s\n",tok_start);
      break ;
    case JSMN_STRING:
      // see if this is one of our wanted keys
      //printf("Key <%s>\n",tok_start);
      if ( strcmp( "curtemp", tok_start ) == 0 ) {
	is_curtemp=true;
      }
      if ( strcmp( "musttemp", tok_start ) == 0 ) {
	is_musttemp=true;
      }
      break ;
    }
    tok_start[length] = temp_null ; // restore char
  }
  return realsize;
}

int main(int argc, char **argv) {
  CURL *http_handle;
  CURLM *multi_handle;
  cmd = parseCmdline(argc, argv);
  int gpiofd;
  int still_running; /* keep number of running handles */

  // open gpio if requested
  if (cmd->gpioP) {
    gpiofd = open(cmd->gpio, O_RDONLY);
    if (-1==gpiofd) die("unable to open gpio %s\n",cmd->gpio);
  }

  if (cmd->daemonP) {
    isdaemon=true;
    openlog(Program,LOG_PID,LOG_DAEMON);
    daemonize();
  }

  http_handle = curl_easy_init();

  /* set the options (I left out a few, you'll get the point anyway) */
  curl_easy_setopt(http_handle, CURLOPT_URL, cmd->url);
  
  /* send all data to this function  */ 
  curl_easy_setopt(http_handle, CURLOPT_WRITEFUNCTION, updateDisplayCallback);
  
  /* init a multi stack */
  multi_handle = curl_multi_init();
  // fetch process state json in an endless loop

  // init i2c display
  i2cfd=init_ht16k33_7segm_display(cmd->i2cdev,cmd->i2caddr);

  while (1) {   

       /* add the individual transfers */
    curl_multi_add_handle(multi_handle, http_handle);

    /* we start some action by calling perform right away */
    curl_multi_perform(multi_handle, &still_running);
    do {
      struct timeval timeout;
      int rc; /* select() return code */

      fd_set fdread;
      fd_set fdwrite;
      fd_set fdexcep;
      int maxfd = -1;

      long curl_timeo = -1;

      FD_ZERO(&fdread);
      FD_ZERO(&fdwrite);
      FD_ZERO(&fdexcep);
      if (cmd->gpioP)
	FD_SET(gpiofd,&fdexcep);

      /* set a suitable timeout to play around with */
      timeout.tv_sec = 1;
      timeout.tv_usec = 0;

      curl_multi_timeout(multi_handle, &curl_timeo);
      if(curl_timeo >= 0) {
        timeout.tv_sec = curl_timeo / 1000;
        if(timeout.tv_sec > 1)
          timeout.tv_sec = 1;
        else
          timeout.tv_usec = (curl_timeo % 1000) * 1000;
      }

      /* get file descriptors from the transfers */
      curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);

      rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);

      switch(rc) {
      case -1:
        /* select error */
        still_running = 0;
        printf("select() returns error, this is badness\n");
        break;
      case 0:
      default:
	if (cmd->gpioP) {
	  if (FD_ISSET(gpiofd, &fdexcep )) {
	    char c;
	    // this delay is for debouncing of gpio
	    usleep(100000);
	    lseek(gpiofd,0,SEEK_SET);
	    read(gpiofd, &c, 1 );
	    if ('0'==c) {
	      if (dispval==DISPCUR) {
		dispval=DISPMUST;
		update_ht16k33_7segm_display(musttemp,true,i2cfd);
	      } else {
		dispval=DISPCUR;
		update_ht16k33_7segm_display(curtemp,false,i2cfd);
	      }
	    }
	  } else {
	    /* timeout or readable/writable sockets */
	    curl_multi_perform(multi_handle, &still_running);
	  }
	} else {
	  /* timeout or readable/writable sockets */
	  curl_multi_perform(multi_handle, &still_running);
	}
        break;
      }
    } while(still_running);
    curl_multi_remove_handle(multi_handle,http_handle);
  }

  curl_easy_cleanup(http_handle);

  return 0;
}

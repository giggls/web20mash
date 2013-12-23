/* 

display_client

Non-webbrowser client for mashctld using a HD44780U compatible LCD
and 4 keys connected via GPIO

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
#include <libgen.h>
#include <locale.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>

#include <wiringPi.h>
#include <lcd.h>

#define LCD_COLS 20
#define LCD_ROWS 4

#include "4x20_client.h"
#include "cmdline.h"
#include "mashstates.h"
#include "menusettings.h"
#include "menustrings.h"
#include "menuitems.h"
#include "getifinfo.h"

/* curl stuff */
#include <curl/curl.h>
/* simple json parser */
#include "jsmn.h"
#define jsmnTOKnum 50

#define KEY_MENU 0
#define KEY_UP 1
#define KEY_DOWN 2
#define KEY_ENTER 3 
#define DEBOUNCE 200000

// Menu states
// display temperature and/or current mash state
#define MSTATE_PSTATE 0
// display various sub-menu actions
#define MSTATE_SELECT 1

#define MENUTYPE_SELECTION 0
#define MENUTYPE_SETTINGS 1

struct s_menusettings menusettings[NUMMENUS];

struct s_pstate pstate;

static bool isdaemon=false;
static int lcdHandle ;

int start_state=1;
int iodinealert=0;
int mpstate=42;

// this will be set to 1 after we got the first set of data from mashctld
int ready=0;
    
// The menu currently displayed
static int menustate=MSTATE_PSTATE;
// the menu displayed bevore the current one
static int previous_menu=-1;

/* ~ symbol ist right arrow in HD44780U charset */
static char right_arrow[2]="~";

// variables for network information menus
char ifnames[MAXINTERFACES][21];
char mac_info[MAXINTERFACES][4][21];
char blank20[21];
char ip_info[MAXINTERFACES+2][MAXADDRS][21];
char ip6g_info[MAXINTERFACES*2+2][MAXADDRS][21];
char ip6l_info[MAXINTERFACES][4][21];

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

void err_print(char* fmt, ...) {
  va_list ap;
  
  va_start(ap, fmt);
  if (isdaemon)
    vsyslog(LOG_ERR, fmt, ap);
  else
    vfprintf(stderr,fmt, ap);
  va_end(ap);
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

void displayPstate() {
  char scratch[21];
  
  debug("LCD: drawing Pstate Info Menu\n");
  // clear if previous menu has been of another type
  if (previous_menu != MSTATE_PSTATE)
    lcdClear(lcdHandle);
  
  sprintf(scratch,"%s%cC",pstate.curtemp,0xdf);
  lcdPosition(lcdHandle, 7, 0);
  lcdPuts(lcdHandle, scratch);
  
  lcdPosition(lcdHandle, 0,2);
  if (pstate.mpstate==0 || pstate.mpstate >8) {
    if (pstate.mpstate == 9) {
      debug("ECLIENT: resetting client side stuff (state 9)\n");
      iodinealert = 0;
      start_state=1;
      menusettings[3].arrow_pos=0;
      menusettings[3].start_pos=0;
    }
    if (cmd->bannerP)
      sprintf(scratch,cmd->banner);
    else
      sprintf(scratch,"    fangobr%cu.de",0xe1);
    lcdPuts(lcdHandle,scratch);
    lcdPosition(lcdHandle, 0,3);
    lcdPuts(lcdHandle,BLANK20);
  } else {
    if (pstate.mpstate%2) {
      sprintf(scratch,"%s..",gettext(mashstate[pstate.mpstate]));
    } else {
      sprintf(scratch,"%s:",gettext(mashstate[pstate.mpstate]));
    }
    sprintf(scratch,"%-20s",scratch);
    lcdPuts(lcdHandle,scratch);
    lcdPosition(lcdHandle, 0,3);
    if (pstate.mpstate%2) {
      sprintf(scratch,"@ %.1f%cC",pstate.resttemp[pstate.mpstate/2],0xdf);
    } else {
      int resttime;
      resttime=pstate.resttime[(pstate.mpstate-1)/2]-pstate.resttimer;
      sprintf(scratch,"%d(%d)min @ %.1f%cC",resttime,pstate.resttime[(pstate.mpstate-1)/2],pstate.resttemp[(pstate.mpstate-1)/2],0xdf);
    }
    sprintf(scratch,"%-20s",scratch);
    lcdPuts(lcdHandle,scratch);
  }
  if ((pstate.mpstate == 0) && (mpstate!=0)) {
      debug("ECLIENT: resetting client side stuff (init)\n");
      iodinealert = 0;
      start_state=1;
      menusettings[3].arrow_pos=0;
      menusettings[3].start_pos=0;    
  }
  mpstate=pstate.mpstate;
}

// set initial states of a menu
// we consider menus with just one item to be a settings menu
// rather than a selection menu
void init_menu(int menu_number,struct s_menusettings *settings) {
  int i;
  settings->number=menu_number;
  settings->arrow_pos=0;
  settings->start_pos=0;
  // do not show an arrow on pseudo selection
  // menues for network information
  if (menu_number <32)
    settings->arrow=true;
  else
    settings->arrow=false;
  
  settings->menutext=(char **)menu_txt[menu_number];
  // setup menu text and count items
  for (i=0;settings->menutext[i]!=NULL;i++);
  settings->numitems=i;
  // default type is selection
  // need to change this for other menu type later
  settings->menutype=MENUTYPE_SELECTION;
  // set defaults for min,max and increment anyway
  settings->increment=1;
  settings->imin=0;
  settings->imax=1;
  settings->fmin=0.0;
  settings->fmax=1.0;
}

// draw a selection type menu
void draw_selection_menu(struct s_menusettings *settings) {
  int i;
  debug("LCD: (re)drawing selection menu #%d, starting at position %d\n",settings->number,settings->start_pos);
  lcdClear(lcdHandle);

  for (i=settings->start_pos;i<settings->start_pos+LCD_ROWS;i++) {
    // break if menu is shorter than LCD_ROWS
    if (i>=settings->numitems) break;
    if (settings->arrow) {
      lcdPosition(lcdHandle, 2,i-settings->start_pos);
    } else {
      lcdPosition(lcdHandle, 0,i-settings->start_pos);
    }
    lcdPuts(lcdHandle,gettext(settings->menutext[i]));
  }
  if (settings->arrow) {
    lcdPosition(lcdHandle,0,settings->arrow_pos);
    lcdPuts(lcdHandle,right_arrow);
  }
}

// draw a selection type menu
void draw_settings_menu(struct s_menusettings *settings) {
  char line[21];
  debug("LCD: drawing settings menu #%d\n",settings->number);
  lcdClear(lcdHandle);
  lcdPosition(lcdHandle,0,0);
  sprintf(line,"%s:",gettext(settings->menutext[0]));
  lcdPuts(lcdHandle,line);
  lcdPosition(lcdHandle,0,2);
  switch (settings->datatype) {
    case MSETTINGS_TYPE_INT:
      sprintf(line,"%3d",settings->ival);
      break;
    case MSETTINGS_TYPE_FLOAT:
      sprintf(line,"%3.1f",settings->fval);
      break;
    case MSETTINGS_TYPE_BOOL:
      if (0==settings->ival)
        sprintf(line,MSTATE_OFF);
      else
        sprintf(line,MSTATE_ON);
      break;
    default:
      ;
  }
  lcdPuts(lcdHandle,line);
}

// draw a menu and call action function if desired
void draw_menu(struct s_menusettings *settings) {
  if (settings->menutype==MENUTYPE_SELECTION) {
    draw_selection_menu(settings);  
  } else {
    draw_settings_menu(settings);
  }
}

void call_menu_action(struct s_menusettings *settings) {
  if (menu_action[settings->number] != NULL) {
    debug("LCD: called ACTION on menu %d\n",settings->number);
    (*menu_action[settings->number]) (settings);
  }
}

void update_selection_menu(int direction,struct s_menusettings *settings) {
  int oldpos;
  debug("LCD: update_selection_menu %d\n",settings->number);
  
  oldpos=settings->arrow_pos;
  
  settings->arrow_pos+=direction;
  if (settings->arrow_pos<0) settings->arrow_pos=0;
  if (settings->arrow_pos > (LCD_ROWS)-1) settings->arrow_pos=(LCD_ROWS)-1;
  // this is for menus shorter than LCD_ROWS
  if (settings->arrow_pos > settings->numitems-1) settings->arrow_pos=settings->numitems-1;
  if (oldpos != settings->arrow_pos) {
    // delete old arrow
    lcdPosition(lcdHandle,0,oldpos);
    lcdPuts(lcdHandle," ");    
    draw_selection_menu(settings);
  } else {
    if ((0==settings->arrow_pos) && settings->start_pos >0) {
      settings->start_pos--;
      draw_selection_menu(settings);
    }
    if ((LCD_ROWS-1)==settings->arrow_pos) {
      if ((1+settings->start_pos+settings->arrow_pos) < settings->numitems) {
        settings->start_pos++;
        draw_selection_menu(settings);
      }
    }
  }
}

void update_settings_menu(int direction,struct s_menusettings *settings) {
  char line[21];
  debug("LCD: update_settings_menu %d (%d)\n",settings->number,direction);
  // this can be done only if process is not running
  if (pstate.mpstate==0 || pstate.mpstate >9) {
  switch (settings->datatype) {
    case MSETTINGS_TYPE_FLOAT:
      settings->fval=settings->fval+(-1*direction*settings->increment);
      if ((settings->fval) < (settings->fmin)) settings->fval=settings->fmin;
      if ((settings->fval) > (settings->fmax)) settings->fval=settings->fmax;
      break;
    case MSETTINGS_TYPE_INT:
      settings->ival=settings->ival+(-1*direction*settings->increment);
      if ((settings->ival) < (settings->imin)) settings->ival=settings->imin;
      if ((settings->ival) > (settings->imax)) settings->ival=settings->imax;
      break;
    case MSETTINGS_TYPE_BOOL:
      if (settings->ival)
        settings->ival=0;
      else
        settings->ival=1;
      break;
    default:
      ;
    }
    lcdPosition(lcdHandle,0,2);
    switch (settings->datatype) {
      case MSETTINGS_TYPE_INT:
        sprintf(line,"%3d",settings->ival);
        break;
      case MSETTINGS_TYPE_FLOAT:
        sprintf(line,"%3.1f",settings->fval);
        break;
      case MSETTINGS_TYPE_BOOL:
        if (0==settings->ival)
          sprintf(line,MSTATE_OFF);
        else
          sprintf(line,MSTATE_ON);
        break;
      default:
        ;
    } 
    lcdPuts(lcdHandle,line);
  }
}

void update_menu(int direction,struct s_menusettings *settings) {
  if (settings->menutype==MENUTYPE_SELECTION) {
    update_selection_menu(direction,settings);
  } else {
    update_settings_menu(direction,settings);
  }
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
  bool is_mpstate=false;
  bool is_resttime=false;
  bool is_resttemp=false;
  bool is_resttimer=false;
  bool is_ctrl=false;
  bool is_rstate=false;
  bool is_stirring=false;

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
	strncpy(pstate.curtemp,tok_start,10);
	pstate.curtemp[9]='\0';
      }
      if (is_musttemp) {
	is_musttemp=false;
      }
      if (is_mpstate) {
	is_mpstate=false;
	sscanf(tok_start,"%d",&pstate.mpstate);
      }
      if (is_resttimer) {
        is_resttimer=false;
        sscanf(tok_start,"%f",&pstate.resttimer);
      }
      if (is_ctrl) {
        is_ctrl=false;
        sscanf(tok_start,"%d",&pstate.ctrl);  
      }
      if (is_stirring) {
        is_stirring=false;
        sscanf(tok_start,"%d",&pstate.stirring);
      }
      break ;
    case JSMN_OBJECT:
      //debug("PARSER: Object %s\n",tok_start);
      break ;
    case JSMN_ARRAY:
      // debug("PARSER: Array %s\n",tok_start);
      if (is_resttemp) {
        is_resttemp=false;
        if (tokens[token_index].size!=4) {
          err_print("ERROR: resttemp array must be 4\n");
          return 0;
        }
        sscanf(tok_start,"[ %f, %f, %f, %f ]",
          &pstate.resttemp[0],&pstate.resttemp[1],
          &pstate.resttemp[2],&pstate.resttemp[3]);
      }
      if (is_resttime) {
        is_resttime=false;
        if (tokens[token_index].size!=4) {
          err_print("ERROR: resttime array must be 4\n");
          return 0;
        }
        sscanf(tok_start,"[ %d, %d, %d, %d ]",
          &pstate.resttime[0],&pstate.resttime[1],
          &pstate.resttime[2],&pstate.resttime[3]);
      }
      if (is_rstate) {
        is_rstate=false;
        if (tokens[token_index].size!=2) {
          err_print("ERROR: rstate array must be 2\n");
          return 0;
        }
        sscanf(tok_start,"[ %d, %d ]",
          &pstate.rstate[0],&pstate.rstate[1]);
      }
      break;
    case JSMN_STRING:
      //debug("PARSER: Key >%s<\n",tok_start);
      // see if this is one of our wanted keys
      if ( strcmp( "curtemp", tok_start ) == 0 ) {
	is_curtemp=true;
      }
      if ( strcmp( "musttemp", tok_start ) == 0 ) {
	is_musttemp=true;
      }
      if ( strcmp( "mpstate", tok_start ) == 0 ) {
        is_mpstate=true;
      }
      if ( strcmp( "resttime", tok_start ) == 0 ) {
        is_resttime=true;
      }
      if ( strcmp( "resttemp", tok_start ) == 0 ) {
        is_resttemp=true;
      }
      if ( strcmp( "resttimer", tok_start ) == 0 ) {
        is_resttimer=true;
      }
      if ( strcmp( "ctrl", tok_start ) == 0 ) {
        is_ctrl=true;
      }
      if ( strcmp( "rstate", tok_start ) == 0 ) {
        is_rstate=true;
      }
      if ( strcmp( "stirring", tok_start ) == 0 ) {
        is_stirring=true;
      }
      break ;
    }
    tok_start[length] = temp_null ; // restore char
  }
  ready=1;
  if (menustate==MSTATE_PSTATE) {
    // this is a little bit of a hack
    // if stirring support is disabled in the server also disable the menu here
    if (pstate.stirring==0) {
      menusettings[5].numitems=1;
    } else {
      menusettings[5].numitems=2;
    }
    int i;
    // update client rest settings from current mashctld settings
    for (i=0;i<4;i++) menusettings[resttime_menus[i]].ival=pstate.resttime[i];
    for (i=0;i<4;i++) menusettings[resttemp_menus[i]].fval=pstate.resttemp[i];
    // update client actuator state settings from current mashctld settings
    menusettings[24].ival=pstate.rstate[0];
    menusettings[25].ival=pstate.rstate[1];
    // if we do a iodine test jump to start_menu
    if ((pstate.mpstate == 7) && (iodinealert == 0) && (pstate.resttemp[3] > pstate.resttemp[2])) {
      previous_menu=menustate;
      menustate=4;
      start_state=7;
      iodinealert = 1;
      draw_menu(&menusettings[menustate]);
    } else {
      displayPstate();
    }
  } else if (menustate==4) {
    if ((pstate.mpstate >= 7) && (iodinealert == 1) && (pstate.ctrl==1)) {
      previous_menu=menustate;
      menustate=MSTATE_PSTATE;
      displayPstate();
    }
  }
  return realsize;
}

int main(int argc, char **argv) {
  CURL *http_handle;
  CURLM *multi_handle;
  cmd = parseCmdline(argc, argv);
  int i,keyfds[4];
  int still_running; /* keep number of running handles */
  
  // i10n stuff
  if (cmd->langP) {
    setlocale(LC_MESSAGES, cmd->lang);
  } else {
    setlocale(LC_MESSAGES, "");
  }
  if (cmd->messagecatP) 
    bindtextdomain( basename(argv[0]), cmd->messagecat);
  else
    bindtextdomain( basename(argv[0]), NULL);
  // HD44780 uses its own 8-bit character set, we use latin1 and manually translate
  // 8-bit characters in .po files
  bind_textdomain_codeset( basename(argv[0]), "ISO-8859-1");
  textdomain( basename(argv[0]) );
  
  // initialize menus
  for (i=1;i<NUMMENUS;i++) {
    init_menu(i,&menusettings[i]);
  }
  
  sprintf(blank20,"%s",BLANK20);
  
  // initialize data types of settings menus
  // also chacnge menutype
  for (i=0;i<4;i++) {
    menusettings[resttime_menus[i]].datatype=MSETTINGS_TYPE_INT;
    menusettings[resttime_menus[i]].increment=1;
    menusettings[resttime_menus[i]].imin=0;
    menusettings[resttime_menus[i]].imax=MAX_REST_TIME;
    menusettings[resttime_menus[i]].menutype=MENUTYPE_SETTINGS;
    
    menusettings[resttemp_menus[i]].datatype=MSETTINGS_TYPE_FLOAT;
    menusettings[resttemp_menus[i]].increment=0.5;
    menusettings[resttemp_menus[i]].fmin=0;
    menusettings[resttemp_menus[i]].fmax=100;
    menusettings[resttemp_menus[i]].menutype=MENUTYPE_SETTINGS;
  }
  
  menusettings[24].datatype=MSETTINGS_TYPE_BOOL;
  menusettings[24].menutype=MENUTYPE_SETTINGS;
  menusettings[25].datatype=MSETTINGS_TYPE_BOOL;
  menusettings[25].menutype=MENUTYPE_SETTINGS;
  
  // show network Information menu only if requested
  if (!cmd->netinfoP)
    menusettings[1].numitems--;
  
  // open gpio ports for keys
  for (i=0;i<4;i++) {
    char gpio[100],c;
    sprintf(gpio,"/sys/class/gpio/gpio%d/value",cmd->keys[i]);
    keyfds[i] = open(gpio, O_RDONLY);
    if (-1==keyfds[i]) die("unable to open gpio %s\n",cmd->keys[i]);
    // clear first pseudo button press
    read(keyfds[i], &c, 1 );
  }
  
  wiringPiSetupSys();
  lcdHandle = lcdInit (LCD_ROWS, LCD_COLS, cmd->lcd[0], cmd->lcd[1],cmd->lcd[2], cmd->lcd[3],cmd->lcd[4],cmd->lcd[5],4,0,0,0,0) ;
  if (lcdHandle < 0) {
    fprintf (stderr, "%s: lcdInit failed\n", argv [0]) ;
    return -1 ;
  }

  lcdPosition(lcdHandle, 0,0);
  lcdPuts(lcdHandle,WAITTEXT);
  
  if (cmd->daemonP) {
    isdaemon=true;
    openlog(Program,LOG_PID,LOG_DAEMON);
    daemonize();
  }

  http_handle = curl_easy_init();

  /* set the options (I left out a few, you'll get the point anyway) */
  char state_url[1024];
  snprintf(state_url,1024,"%s/%s",cmd->url,"getstate");
  state_url[1023]='\0';
  curl_easy_setopt(http_handle, CURLOPT_URL, state_url);
  
  /* send all data to this function  */ 
  curl_easy_setopt(http_handle, CURLOPT_WRITEFUNCTION, updateDisplayCallback);
  
  /* init a multi stack */
  multi_handle = curl_multi_init();
  // fetch process state json in an endless loop

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
      for (i=0;i<4;i++)
        FD_SET(keyfds[i],&fdexcep);

      /* set a suitable timeout to play around with */
      timeout.tv_sec = 10;
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
        char c;
      case -1:
        /* select error */
        still_running = 0;
        err_print("select() returns error, this is badness\n");
        break;
      case 0: /* timeout */ 
        break;
      default: /* action */ 
	  if (FD_ISSET(keyfds[KEY_UP], &fdexcep )) {
	    // this delay is for debouncing of gpio
	    usleep(DEBOUNCE);
	    lseek(keyfds[KEY_UP],0,SEEK_SET);
	    read(keyfds[KEY_UP], &c, 1 );
	    debug("pressed key KEY_UP\n");
	    if (ready) {
	      if (menustate>0) {
	        update_menu(-1,&menusettings[menustate]);
              }
	    }
	  } else if (FD_ISSET(keyfds[KEY_DOWN], &fdexcep )) {
	      // this delay is for debouncing of gpio
	      usleep(DEBOUNCE);
	      lseek(keyfds[KEY_DOWN],0,SEEK_SET);
	      read(keyfds[KEY_DOWN], &c, 1 );
	      debug("pressed key KEY_DOWN\n");
	      if (ready) {
	        if (menustate>0) {
	          update_menu(1,&menusettings[menustate]);
                }
	      }
          } else if (FD_ISSET(keyfds[KEY_ENTER], &fdexcep )) {
              int item;
              // this delay is for debouncing of gpio
              usleep(DEBOUNCE);
              lseek(keyfds[KEY_ENTER],0,SEEK_SET);
              read(keyfds[KEY_ENTER], &c, 1 );
              debug("pressed key KEY_ENTER\n");
              if (ready) {
                if (menustate>0) {
                  item=menusettings[menustate].start_pos+menusettings[menustate].arrow_pos;
                  call_menu_action(&menusettings[menustate]);
                  // call next menu
                  if (0!=next_menu[menustate][item]) {
                      previous_menu=menustate;
                      menustate=next_menu[menustate][item];
                      if (menusettings[menustate].menutext[0]!=NULL) {
                        draw_menu(&menusettings[menustate]);
                      } else {
                        item=menusettings[menustate].start_pos+menusettings[menustate].arrow_pos;
                        call_menu_action(&menusettings[menustate]);
                        previous_menu=menustate;
                        menustate=next_menu[menustate][item];
                        if (menustate!=MSTATE_PSTATE) {
                          draw_menu(&menusettings[menustate]);
                        } else {
                          displayPstate();
                        }
                      }
                  }
                }
              }
          } else if (FD_ISSET(keyfds[KEY_MENU], &fdexcep )) {
              // this delay is for debouncing of gpio
              usleep(DEBOUNCE);
              lseek(keyfds[KEY_MENU],0,SEEK_SET);
              read(keyfds[KEY_MENU], &c, 1 );
              debug("pressed key KEY_MENU\n");
              if (ready) {
                if (menustate==MSTATE_PSTATE) {
                  previous_menu=menustate;
                  menustate=MSTATE_SELECT;
                  draw_menu(&menusettings[MSTATE_SELECT]);
                } else {
                  previous_menu=menustate;
                  menustate=MSTATE_PSTATE;
                  displayPstate();
                }
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

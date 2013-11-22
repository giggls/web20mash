// all the functions to be called from settings menus

#include <stdio.h>
#include <curl/curl.h>
#include "4x20_client.h"
#include "cmdline.h"
#include "menusettings.h"

extern Cmdline *cmd;
extern struct s_menusettings menusettings[NUMMENUS];
extern struct s_pstate pstate;
extern int start_state;
extern int iodinealert;

void fetch_url(char *url) {
  CURL *curl;
  CURLcode res;
  
  curl = curl_easy_init();
  
  debug("fetch_url: calling url %s\n",url);
  
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url);
               
   /* Perform the request, res will get the return code */ 
   res = curl_easy_perform(curl);
   /* Check for errors */ 
   if(res != CURLE_OK)
   
   die("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
  
  /* always cleanup */ 
  curl_easy_cleanup(curl);
  }
  return;  
}

void set_rest_timetemp() {
 char url[1024];

  // this can be done only if process is not running
  if (pstate.mpstate==0 || pstate.mpstate >9) {     
 
   snprintf(url,1024,"%s/setallmash/%.1f/%d/%.1f/%d/%.1f/%d/%.1f/%d",
     cmd->url,menusettings[resttemp_menus[0]].fval,menusettings[resttime_menus[0]].ival,
     menusettings[resttemp_menus[1]].fval,menusettings[resttime_menus[1]].ival,
     menusettings[resttemp_menus[2]].fval,menusettings[resttime_menus[2]].ival,
     menusettings[resttemp_menus[3]].fval,menusettings[resttime_menus[3]].ival);
   url[1023]='\0';
   fetch_url(url);
 }
}

void set_start_1() {
  debug("LCD: called ACTION function set_start_1\n");
  start_state=1;  
}

void set_start_2() {
  debug("LCD: called ACTION function set_start_2\n");
  start_state=2;  
}

void set_start_3() {
  debug("LCD: called ACTION function set_start_3\n");
  start_state=3;  
}

void set_start_4() {
  debug("LCD: called ACTION function set_start_4\n");
  start_state=4;  
}
  
void set_start_5() {
  debug("LCD: called ACTION function set_start_5\n");
  start_state=5;  
}
  
void set_start_6() {
  debug("LCD: called ACTION function set_start_6\n");
  start_state=6;  
}

void set_start_7() {
  debug("LCD: called ACTION function set_start_7\n");
  iodinealert = 1;
  start_state=7;  
}
  
void set_start_8() {
  debug("LCD: called ACTION function set_start_8\n");
  start_state=8;  
}

void start_process() {
  char url[1024];
  
  snprintf(url,1024,"%s/setmpstate/%d",cmd->url,start_state);
  url[1023]='\0';
  fetch_url(url);
}

void stop_process() {
  char url[1024];
  
  iodinealert =0;
  snprintf(url,1024,"%s/setmpstate/0",cmd->url);
  url[1023]='\0';
  fetch_url(url);
}

void toggle_actuator0(struct s_menusettings *settings) {
  char url[1024];
  if (pstate.ctrl==0) {
    snprintf(url,1024,"%s/setactuator/0/%d",cmd->url,settings->ival);
    url[1023]='\0';
    fetch_url(url);
  } else {
    debug("do not change actuator of heater, control running\n");
  }
}

void toggle_actuator1(struct s_menusettings *settings) {
  char url[1024];
  if (pstate.ctrl==0) {
    snprintf(url,1024,"%s/setactuator/1/%d",cmd->url,settings->ival);
    url[1023]='\0';
    fetch_url(url);
  } else {
    debug("do not change actuator of stirring device, control running\n");
  }
}



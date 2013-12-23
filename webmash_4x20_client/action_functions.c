// all the functions to be called from settings menus
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "4x20_client.h"
#include "cmdline.h"
#include "menusettings.h"
#include "menustrings.h"
#include "getifinfo.h"


extern Cmdline *cmd;
extern struct s_menusettings menusettings[NUMMENUS];
extern struct s_pstate pstate;
extern int start_state;
extern int iodinealert;
extern char* menu_txt[NUMMENUS][NUMITEMS];
extern int next_menu[NUMMENUS][NUMITEMS];

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

int split_ipv6(char *src,char *dst1, char *dst2) { 
  unsigned char buf[sizeof(struct in6_addr)];
  int i;
  
  if (!inet_pton(AF_INET6, src, buf)) return -1;
  for (i=0;i<8;i+=2) { 
    dst1+=sprintf(dst1,"%02x%02x:",buf[i],buf[i+1]);
  }           
  for (;i<14;i+=2) {  
    dst2+=sprintf(dst2,"%02x%02x:",buf[i],buf[i+1]);
  }
  sprintf(dst2,"%02x%02x",buf[i],buf[i+1]);
  return 0;
}

void netinfo() {
  char *url;
  int numif,i,j,numitems;
  struct s_ip_interface_info ip_interface_info[MAXINTERFACES];
  // global variables for info text
  extern char ifnames[MAXINTERFACES][21];
  extern char mac_info[MAXINTERFACES][4][21];
  extern char blank20[21];
  extern char ip_info[MAXINTERFACES+2][MAXADDRS][21];
  extern char ip6g_info[MAXINTERFACES*2+2][MAXADDRS][21];
  extern char ip6l_info[MAXINTERFACES][4][21];
  
  debug("netinfo menu action getting network interface information\n");
  
  url=malloc(sizeof(char)*(strlen(cmd->url)+11));
  strcpy(url,cmd->url);
  strcpy(url+strlen(cmd->url),"/getifinfo");
  debug("calling getifinfo with URL: %s\n",url);
  
  // query network information from mashctld via http
  // and parse (json) result
  numif=getifinfo(url,ip_interface_info);
  
  if (numif<1) {
    menusettings[27].numitems=1;
    sprintf(ifnames[0],"%s",gettext(IFNOTFOUND));
    menusettings[27].menutext[0]=ifnames[0];
    menusettings[27].arrow=false;
    next_menu[27][0]=1;
  } else {
    next_menu[27][0]=28;
    menusettings[27].arrow=true;
    menusettings[27].numitems=numif;
  }
  
  // now setup the whole meu structure stuff for all interfaces
  // Menu 27 ist the first menu with all (max 4) selectable interfaces
  // Menus 28-31 are the menus for the individual interfaces
  // we need to fill 32-35, 36-39, 40-43, 44-47 depending on the number
  // of interfaces 
  for (i=0;i<numif;i++) {
    sprintf(ifnames[i],"Interface %s",ip_interface_info[i].name);
    menusettings[27].menutext[i]=ifnames[i];

    // Fill Information menu for MAC-address (32+4*i)
    sprintf(mac_info[i][0],"%s %s:",gettext(MTEXTMAC),ip_interface_info[i].name);
    strcpy(mac_info[i][1],blank20);
    sprintf(mac_info[i][2],"%s",ip_interface_info[i].mac);
    strcpy(mac_info[i][3],blank20);
    
    menusettings[32+4*i].menutext[0]=mac_info[i][0];
    menusettings[32+4*i].menutext[1]=mac_info[i][1];
    menusettings[32+4*i].menutext[2]=mac_info[i][2];
    menusettings[32+4*i].menutext[3]=mac_info[i][3];
    menusettings[32+4*i].menutext[4]=NULL;
    
    // Fill Information menu for IPv4-address(es)
    sprintf(ip_info[i][0],"IPv4 %s:",ip_interface_info[i].name);
    menusettings[33+4*i].menutext[0]=ip_info[i][0];
    numitems=1;
    for (j=0; j<MAXADDRS;j++) {
      if ('\0'==ip_interface_info[i].v4_ip[j][0]) break;
      sprintf(ip_info[i][j+1],"%s",ip_interface_info[i].v4_ip[j]);
      menusettings[33+4*i].menutext[j+1]=ip_info[i][j+1];
      numitems++;
    }
    menusettings[33+4*i].numitems=numitems; 
    
    // Fill Information menu for IPv6 global unicast address(es)
    sprintf(ip6g_info[i][0],"IPv6 global %s:",ip_interface_info[i].name);
    menusettings[34+4*i].menutext[0]=ip6g_info[i][0];
    numitems=1;
    for (j=0; j<MAXADDRS;j++) {
      if ('\0'==ip_interface_info[i].v6_global_ip[j][0]) break;
      split_ipv6(ip_interface_info[i].v6_global_ip[j],ip6g_info[i][j*2+1],ip6g_info[i][j*2+2]);
      menusettings[34+4*i].menutext[j*2+1]=ip6g_info[i][j*2+1];
      menusettings[34+4*i].menutext[j*2+2]=ip6g_info[i][j*2+2];
      numitems+=2;
    }
    menusettings[34+4*i].numitems=numitems;
    
    // Fill Information menu for IPv6 link.local address
    sprintf(ip6l_info[i][0],"%s",gettext(MTEXTIP6L));
    sprintf(ip6l_info[i][1],"%s",ip_interface_info[i].name);
    sprintf(ip6l_info[i][2],"fe80::");
    strcpy(ip6l_info[i][3],ip_interface_info[i].v6_local_ip+6);
    menusettings[35+4*i].menutext[0]=ip6l_info[i][0];
    menusettings[35+4*i].menutext[1]=ip6l_info[i][1];
    menusettings[35+4*i].menutext[2]=ip6l_info[i][2];
    menusettings[35+4*i].menutext[3]=ip6l_info[i][3];
    menusettings[35+4*i].menutext[4]=NULL;
    
  }
  free(url);
}

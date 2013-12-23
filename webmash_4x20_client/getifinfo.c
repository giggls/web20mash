#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>

#include <curl/curl.h>

#include "jsmn.h"
#include "4x20_client.h"
#include "getifinfo.h"

#define ifjsmnTOKnum 50

static size_t CurlWritebufCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  size_t realsize = size * nmemb;
  char *mem=(char *) userp;

  if (realsize > 4095) {
    memcpy(mem, contents, 4095);
    mem[4095]='\0';
    return 4096;
  } else {
   memcpy(mem, contents, realsize);
   mem[realsize-1]='\0';
   return realsize;
  }
}

int getifinfo(char *url,struct s_ip_interface_info* ip_interface_info) {
  CURL *curl_handle;
  CURLcode res;

  char json[4096];
  // jsmn stuff
  jsmn_parser ifparser;
  jsmntok_t iftokens[ifjsmnTOKnum];
  int iftoken_index;
  int numif;
  bool is_interface;
  bool is_mac;
  bool is_ip_arr;
  bool is_ip;
  bool is_ip6local;
  bool is_ip6global_arr;
  bool is_ip6global;
  int arr_size;
  int arr_pos;
  
  curl_global_init(CURL_GLOBAL_ALL);

  /* init the curl session */
  curl_handle = curl_easy_init();

  /* specify URL to get */
  curl_easy_setopt(curl_handle, CURLOPT_URL, url);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, CurlWritebufCallback);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&json[0]);
  res = curl_easy_perform(curl_handle);

  /* check for errors */
  if(res != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
  }

  debug("-------------------- JSON:\n");
  debug("%s\n",json);
  debug("--------------------------\n");
  
  /* cleanup curl stuff */
  curl_easy_cleanup(curl_handle);

  /* we're done with libcurl, so clean it up */
  curl_global_cleanup();
  
  int i,j;
  // initialice structure
  for (i=0;i<MAXINTERFACES;i++) {
    for (j=0;j<MAXADDRS+1;j++) {
      ip_interface_info[i].v4_ip[j][0]='\0';
      ip_interface_info[i].v6_global_ip[j][0]='\0';
    }
  }
  
  jsmn_init(&ifparser);

  is_interface=false;
  is_mac=false;
  is_ip_arr=false;
  is_ip=false;
  is_ip6local=false;
  is_ip6global_arr=false;
  is_ip6global=false;
                
  switch ( jsmn_parse( &ifparser, json, iftokens, ifjsmnTOKnum ) ) {
  case JSMN_ERROR_NOMEM:
    err_print("Not enough JSON tokens were provided");
    break ;
  case JSMN_ERROR_INVAL:
    err_print("Invalid character inside JSON string");
    break ;
  case JSMN_ERROR_PART:
    err_print("The string is not a full JSON packet, more bytes expected");
    break ;
  case JSMN_SUCCESS:
    break ;
  }
  numif=0;
  arr_pos=-1;
  for ( iftoken_index = 0 ; iftoken_index < ifparser.toknext ; ++iftoken_index ) {
    int length = iftokens[iftoken_index].end - iftokens[iftoken_index].start ;
    char * tok_start = &json[iftokens[iftoken_index].start] ;
    char temp_null = tok_start[length] ;
    tok_start[length] = '\0' ; // null terminate temporarily

    switch ( iftokens[iftoken_index].type ) {
    case JSMN_PRIMITIVE:
      //debug("Primitive %s\n",tok_start);
      if (iftoken_index==0) {
        err_print("ERROR: outer object of json must be an array\n");
        return -1;
      }
      if (iftoken_index==1) {
        err_print("ERROR: json must be an array of objects\n");
        return -1;
      }
      break ;
    case JSMN_OBJECT:
      //debug("PARSER: Object %s\n",tok_start);
      if (iftoken_index==0) {
        err_print("ERROR: outer object of json must be an array\n");
        return -1;
      }
      numif++;
      if (numif >MAXINTERFACES) {
        err_print("maximum number of Interfaces (%u) exceeded\n",MAXINTERFACES);
        return -1;
      }
      break ;
    case JSMN_ARRAY:
      //debug("PARSER: Array of size %d %s\n",iftokens[iftoken_index].size,tok_start);
      if (is_ip_arr) {
        is_ip=true;
        arr_size=iftokens[iftoken_index].size;
      }
      if (is_ip6global_arr) {
        is_ip6global=true;
        arr_size=iftokens[iftoken_index].size;
      }
      if (iftoken_index==1) {
        err_print("ERROR: json must be an array of objects\n");
        return -1;
      }
      break;
    case JSMN_STRING:
      //debug("PARSER: Key >%s<\n",tok_start);
      if (iftoken_index==0) {
        err_print("ERROR: outer object of json must be an array\n");
        return -1;
      }
      if (iftoken_index==1) {
        err_print("ERROR: json must be an array of objects\n");
        return -1;
      }

      if (is_interface) {
        is_interface=false;
        strncpy(ip_interface_info[numif-1].name,tok_start,21);
        ip_interface_info[numif-1].name[20]='\0';
      }
      if (is_mac) {
        is_mac=false;
        strncpy(ip_interface_info[numif-1].mac,tok_start,18);
        ip_interface_info[numif-1].mac[17]='\0';
      }
      if (is_ip6local) {
        is_ip6local=false;
        strncpy(ip_interface_info[numif-1].v6_local_ip,tok_start,INET6_ADDRSTRLEN);
        ip_interface_info[numif-1].v6_local_ip[INET6_ADDRSTRLEN-1]='\0';
      }
      if (is_ip) {
        arr_pos++;
        if (arr_pos >= arr_size) {
          is_ip=false;
          is_ip_arr=false;
          arr_pos=-1;
          arr_size=0;
        } else {
          if (arr_pos+1 > MAXADDRS) {
            err_print("maximum number of addresses (%u) exceeded\n",MAXADDRS);
            is_ip=false;
            is_ip_arr=false;
            arr_pos=-1;
            arr_size=0;
            break;
          }
          //debug("Interface %d: processing IPv4 array element %d: %s\n",numif-1,arr_pos,tok_start);
          strncpy(ip_interface_info[numif-1].v4_ip[arr_pos],tok_start,16);
          ip_interface_info[numif-1].v4_ip[arr_pos][15]='\0';
        }
      }
      if (is_ip6global) {
          arr_pos++;
          if (arr_pos >= arr_size) {
            is_ip6global=false;
            is_ip6global_arr=false;
            arr_pos=-1;
            arr_size=0;
          } else {
          if (arr_pos+1 > MAXADDRS) {
            err_print("maximum number of addresses (%u) exceeded\n",MAXADDRS);
            is_ip6global=false;
            is_ip6global_arr=false;
            arr_pos=-1;
            arr_size=0;
            break;
          }
          //debug("Interface %d: processing IPv6 global array element %d: %s\n",numif-1,arr_pos,tok_start);
          strncpy(ip_interface_info[numif-1].v6_global_ip[arr_pos],tok_start,INET6_ADDRSTRLEN);
          ip_interface_info[numif-1].v6_global_ip[arr_pos][INET6_ADDRSTRLEN-1]='\0';
        }
      }
      
      if ( strcmp( "interface", tok_start ) == 0 ) {
        is_interface=true;
      }
      if ( strcmp( "mac", tok_start ) == 0 ) {
        is_mac=true;
      }
      if ( strcmp( "ip", tok_start ) == 0 ) {
        is_ip_arr=true;
      }
      if ( strcmp( "ip6local", tok_start ) == 0 ) {
        is_ip6local=true;
      }
      if ( strcmp( "ip6global", tok_start ) == 0 ) {
        is_ip6global_arr=true;
      }
      break ;
    }
    tok_start[length] = temp_null ; // restore char
  }

  return numif;
}


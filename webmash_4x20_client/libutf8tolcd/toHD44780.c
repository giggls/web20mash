/*
convert a file from utf-8 encoding to HD44780 encoding

*/
#include <stdio.h>
#include <regex.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include <langinfo.h>
#include <stdbool.h>
#include "utf8tohd44780.h"

int main(int argc, char **argv) {
  FILE *ifp;
  char *indata;
  size_t len = 0;
  regex_t preg;
  

  setlocale(LC_ALL, "");

  if ((argc <2) || (argc >3)) {
    fprintf(stderr,"usage: utf8tolcd <file> ?regex?\n");
    exit(EXIT_FAILURE);
  }
  
  if (argc == 3)
    regcomp(&preg, argv[2], REG_EXTENDED);
  
  ifp=fopen(argv[1],"rb");
  if (NULL==ifp) {
    fprintf(stderr,"unable to open file %s\n",argv[1]);
    exit(EXIT_FAILURE);
  }

  while (getline(&indata, &len, ifp) != -1) {
    int res;
    bool convert;
    
    if (argc == 3) {
      if (regexec(&preg, indata, 0, NULL, 0)==0) {
        convert=true;
      } else {
        convert=false;
      }
    } else {
      convert=true;
    }
    
    // convert only lines matching regex
    if (convert) {
      res=utf8str_to_hd44780((uint8_t **)&indata);
      if (res >0) {
        fprintf(stderr,"%s\n",utf8tohd44780_msg[res]);
        exit(EXIT_FAILURE);
      }
    }
    printf("%s",indata);
  }

  if (argc == 3)
    regfree(&preg);
  free(indata);
  fclose(ifp);
  exit(0);
}

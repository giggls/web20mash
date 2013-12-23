#include <stdio.h>
#include <string.h>
#include "gen_json_4interfaces.h"

int main() {
  char ifdata[4096];
  update_all_interf_info();
  fill_interf_json(ifdata,4096);
  fwrite(ifdata,sizeof(char),strlen(ifdata),stdout);
}

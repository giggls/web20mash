#include <arpa/inet.h>
#define MAXADDRS 5

struct s_ip_interface_info {
  char name[21];
  char mac[18];
  char v6_local_ip[INET6_ADDRSTRLEN];
  char v6_global_ip[MAXADDRS+1][INET6_ADDRSTRLEN];
  char v4_ip[MAXADDRS+1][16];
};

int getifinfo(char *url,struct s_ip_interface_info* ip_interface_info);

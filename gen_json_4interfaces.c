#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>

#include <libmnl/libmnl.h>
#include <linux/if.h>
#include <linux/if_link.h>
#include <linux/rtnetlink.h>

/*
Print IP-adresses of all Interfaces in json format
Alias Interface Names tb[IFA_LABEL] are currently ignored
*/

#define MAXADDRS 10
#define MAXINTERFACES 20
#define UNUSED(expr) do { (void)(expr); } while (0)

struct s_ip_interface_info {
  char name[21];
  char mac[18];
  char v6_local_ip[INET6_ADDRSTRLEN];
  char v6_global_ip[MAXADDRS][INET6_ADDRSTRLEN];
  char v4_ip[MAXADDRS][16];
  unsigned index;
  // relevant interfaces (for us) are non local(loopback) and running
  bool relevant;
};

struct s_ip_interface_info ip_interface_info[MAXINTERFACES];
int all_interface_count=0;
int up_interface_count=0;

static int getlink_data_attr_cb(const struct nlattr *attr, void *data) {
  const struct nlattr **tb = data;
  int type = mnl_attr_get_type(attr);
	
  /* skip unsupported attribute in user-space */
  if (mnl_attr_type_valid(attr, IFLA_MAX) < 0)
    return MNL_CB_OK;
		
  switch(type) {
  case IFLA_ADDRESS:
    if (mnl_attr_validate(attr, MNL_TYPE_BINARY) < 0) {
      perror("mnl_attr_validate");
      return MNL_CB_ERROR;
    }
    break;
  case IFLA_MTU:
   if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0) {
      perror("mnl_attr_validate");
      return MNL_CB_ERROR;
    }
    break;
  case IFLA_IFNAME:
    if (mnl_attr_validate(attr, MNL_TYPE_STRING) < 0) {
      perror("mnl_attr_validate2");
      return MNL_CB_ERROR;
    }
    break;
  }
  tb[type] = attr;
  return MNL_CB_OK;
}

static int getlink_data_cb(const struct nlmsghdr *nlh, void *data) {
  struct nlattr *tb[IFLA_MAX+1] = {0};
  struct ifinfomsg *ifm = mnl_nlmsg_get_payload(nlh);
  
  UNUSED(data);

  if (all_interface_count >= MAXINTERFACES) return MNL_CB_OK;

  mnl_attr_parse(nlh, sizeof(*ifm), getlink_data_attr_cb, tb);

  if (tb[IFLA_IFNAME]) {
    strncpy(ip_interface_info[all_interface_count].name,
	    mnl_attr_get_str(tb[IFLA_IFNAME]),20);
    ip_interface_info[all_interface_count].name[20]='\0';
  }

  // we care about running and non-loopback interfaces only
  if ((ifm->ifi_flags & IFF_RUNNING) && (0==(ifm->ifi_flags & IFF_LOOPBACK))) {
    ip_interface_info[all_interface_count].relevant=1;

    if (tb[IFLA_ADDRESS]) {
      uint8_t *hwaddr = mnl_attr_get_payload(tb[IFLA_ADDRESS]);
      if (mnl_attr_get_payload_len(tb[IFLA_ADDRESS])<6)
	    sprintf(ip_interface_info[all_interface_count].mac,"00:00:00:00:00:00");
      else
	sprintf(ip_interface_info[all_interface_count].mac,"%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
		hwaddr[0], hwaddr[1], hwaddr[2], hwaddr[3], hwaddr[4], hwaddr[5]);
    } else {
      sprintf(ip_interface_info[all_interface_count].mac,"00:00:00:00:00:00");
    }
    up_interface_count++;
  }
  ip_interface_info[all_interface_count].index=ifm->ifi_index;
  all_interface_count++;
  return MNL_CB_OK;
}

static int getaddr_data_attr_cb(const struct nlattr *attr, void *data) {
  const struct nlattr **tb = data;
  int type = mnl_attr_get_type(attr);

  /* skip unsupported attribute in user-space */
  if (mnl_attr_type_valid(attr, IFA_MAX) < 0)
    return MNL_CB_OK;

  switch(type) {
  case IFA_ADDRESS:
    if (mnl_attr_validate(attr, MNL_TYPE_BINARY) < 0) {
      perror("mnl_attr_validate");
      return MNL_CB_ERROR;
    }
    break;
  }
  tb[type] = attr;
  return MNL_CB_OK;
}

static int getaddr_data_cb(const struct nlmsghdr *nlh, void *data) {
  int i,j;
  struct nlattr *tb[IFLA_MAX+1] = {0};
  struct ifaddrmsg *ifa = mnl_nlmsg_get_payload(nlh);

  UNUSED(data);
  // we care about link-local(253) and global(0) addresses only
  if (((0==ifa->ifa_scope) || (253==ifa->ifa_scope))) {
    // lookup interface with the current index
    i=0;
    while(ip_interface_info[i].index!=ifa->ifa_index) {
      if (i >MAXINTERFACES) return MNL_CB_OK;
      i++;
    }
    // add IP-adress to ip_interface_info struct
    mnl_attr_parse(nlh, sizeof(*ifa), getaddr_data_attr_cb, tb);
    if (tb[IFA_ADDRESS]) {
      void *addr = mnl_attr_get_payload(tb[IFA_ADDRESS]);
      char out[INET6_ADDRSTRLEN];

      if (inet_ntop(ifa->ifa_family, addr, out, sizeof(out))) {
	j=0;
        if (ifa->ifa_family==AF_INET6) {
	  if (ifa->ifa_scope==253) {
	    strcpy(ip_interface_info[i].v6_local_ip,out);
	  } else {
	    // ignore TEMPORARY adresses in case of ipv6
	    if (!(ifa->ifa_flags & IFA_F_TEMPORARY)) {
	      while(ip_interface_info[i].v6_global_ip[j][0]!='\0') j++;
	      strcpy(ip_interface_info[i].v6_global_ip[j],out);
	    }
	  }
	} else {
	  while(ip_interface_info[i].v4_ip[j][0]!='\0') j++;
	  strcpy(ip_interface_info[i].v4_ip[j],out);
	}
      }
    }
  }
  return MNL_CB_OK;
}

int update_interf_info(char **relevant, int rel_len) {
  struct mnl_socket *nl;
  char buf[MNL_SOCKET_BUFFER_SIZE];
  struct nlmsghdr *nlh;
  struct rtgenmsg *rt;
  int i,j,ret;
  bool found;
  
  unsigned int seq, portid;
  
  /* initialice custom ip_interface_info structs */
  for (i=0;i<MAXINTERFACES;i++) {
    ip_interface_info[i].relevant=0;
    for (j=0;j<MAXADDRS;j++) {
      ip_interface_info[i].v6_global_ip[j][0]='\0';
      ip_interface_info[i].v4_ip[j][0]='\0';
    }
    ip_interface_info[i].v6_local_ip[0]='\0';    
  }
  all_interface_count=0;
  up_interface_count=0;


  // first we acquire link level Information
  nlh = mnl_nlmsg_put_header(buf);
  nlh->nlmsg_type	= RTM_GETLINK;
  nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
  nlh->nlmsg_seq = seq = time(NULL);
  rt = mnl_nlmsg_put_extra_header(nlh, sizeof(struct rtgenmsg));
  rt->rtgen_family = AF_PACKET;

  nl = mnl_socket_open(NETLINK_ROUTE);
  if (nl == NULL) {
    perror("mnl_socket_open");
    exit(EXIT_FAILURE);
  }

  if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
    perror("mnl_socket_bind");
    exit(EXIT_FAILURE);
  }
  portid = mnl_socket_get_portid(nl);

  if (mnl_socket_sendto(nl, nlh, nlh->nlmsg_len) < 0) {
    perror("mnl_socket_send");
    exit(EXIT_FAILURE);
  }

  ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
  while (ret > 0) {
    ret = mnl_cb_run(buf, ret, seq, portid, getlink_data_cb, NULL);
    if (ret <= MNL_CB_STOP)
      break;
    ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
  }
  if (ret == -1) {
    perror("error");
    exit(EXIT_FAILURE);
  }

  mnl_socket_close(nl);


  // now go for IP-layer (v4 and v6)
  nlh = mnl_nlmsg_put_header(buf);
  nlh->nlmsg_type       = RTM_GETADDR;
  nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
  nlh->nlmsg_seq = seq = time(NULL);
  rt = mnl_nlmsg_put_extra_header(nlh, sizeof(struct rtgenmsg));

  nl = mnl_socket_open(NETLINK_ROUTE);
  if (nl == NULL) {
    perror("mnl_socket_open");
    exit(EXIT_FAILURE);
  }

  if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
    perror("mnl_socket_bind");
    exit(EXIT_FAILURE);
  }
  portid = mnl_socket_get_portid(nl);

  if (mnl_socket_sendto(nl, nlh, nlh->nlmsg_len) < 0) {
    perror("mnl_socket_send");
    exit(EXIT_FAILURE);
  }

  ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
  while (ret > 0) {
    ret = mnl_cb_run(buf, ret, seq, portid, getaddr_data_cb, NULL);
    if (ret <= MNL_CB_STOP)
      break;
    ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
  }
  if (ret == -1) {
    perror("error");
    exit(EXIT_FAILURE);
  }

  mnl_socket_close(nl);
  
  // mark only requested interfaces as relevant
  if (relevant != NULL) {
    for (i=0;i<all_interface_count;i++) {
      if (ip_interface_info[i].relevant) {
        found=false;
        for (j=0;j<rel_len;j++) {
          if (0==strcmp(ip_interface_info[i].name,relevant[j])) {
            found=true;
            break;
          }
        }
        if (!found)
          ip_interface_info[i].relevant=false;
      }
    }
  }
  return 0;
}

int fill_buf(char **buf, size_t *size, char *fmt, ...) {
  size_t offset;
  va_list ap;
  va_start(ap, fmt);
  offset=vsnprintf(*buf, *size, fmt, ap);
  if (offset >=*size) return -1; *buf+=offset; *size-=offset;
  
  va_end(ap);
  return 0;
}

int fill_interf_json(char *buf,size_t maxlen) {
  int i,j,first;

  fill_buf(&buf,&maxlen,"[\n");  
  first=1;
  for (i=0;i<all_interface_count;i++) {
    if (ip_interface_info[i].relevant) {
      if (first==1) {
        first=0;
      } else {
        fill_buf(&buf,&maxlen,",\n");
      }
      fill_buf(&buf,&maxlen,"  {\n");
      fill_buf(&buf,&maxlen,"\t\"interface\": \"%s\",\n",ip_interface_info[i].name);
      fill_buf(&buf,&maxlen,"\t\"mac\": \"%s\",\n",ip_interface_info[i].mac);
      j=0;
      fill_buf(&buf,&maxlen,"\t\"ip\": [");
      while(ip_interface_info[i].v4_ip[j][0]!='\0') {
	fill_buf(&buf,&maxlen,"\"%s\"",ip_interface_info[i].v4_ip[j]);
	if (ip_interface_info[i].v4_ip[j+1][0]!='\0') {
	  fill_buf(&buf,&maxlen,", ");
        }
	j++;
      }
      fill_buf(&buf,&maxlen,"],\n");
      if (ip_interface_info[i].v6_local_ip[0]!='\0') {
	fill_buf(&buf,&maxlen,"\t\"ip6local\": \"%s\",\n",ip_interface_info[i].v6_local_ip);
      }
      j=0;
      fill_buf(&buf,&maxlen,"\t\"ip6global\": [");
      while(ip_interface_info[i].v6_global_ip[j][0]!='\0') {
	fill_buf(&buf,&maxlen,"\"%s\"",ip_interface_info[i].v6_global_ip[j]);
	if (ip_interface_info[i].v6_global_ip[j+1][0]!='\0') {
	  fill_buf(&buf,&maxlen,", ");
        }
	j++;
      }
      fill_buf(&buf,&maxlen,"]\n");
      fill_buf(&buf,&maxlen,"  }");
    }
    if (all_interface_count==i+1) {
      fill_buf(&buf,&maxlen,"\n");
    }
  }
  fill_buf(&buf,&maxlen,"]\n");
  return 0;
}

int update_all_interf_info() {
  return update_interf_info(NULL,0);
}

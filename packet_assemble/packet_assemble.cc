#inlcude <stdio.h>
#include <ap_axi_sdata.h>

struct ip_addr {
	unsigned char addr[4];
};

struct ip_header {
	unsigned char  ip_version:4;
	unsigned char  ip_length:4;
	unsigned char  ip_tos;
	unsigned char  ip_len[2];
	unsigned short ip_id;
	unsigned short ip_off;
	unsigned char  ip_ttl;
	unsigned char  ip_p;
	unsigned short ip_csum;
	struct ip_addr ip_src;
	struct ip_addr ip_dst;
};

struct ports {
	unsigned char s_port[2];
	unsigned char d_port[2];
};

struct eth_header {
	unsigned char dst_addr [6];
	unsigned char src_addr [6];
	unsigned char ip_type [2];	
};


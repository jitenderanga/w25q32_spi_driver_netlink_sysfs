#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<asm/types.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<linux/netlink.h>
#include<string.h>

#define MAX_SIZE 1024

struct sockaddr_nl source;

int netlink_w25q_init(void);
void netlink_w25q_dest(int);
void w25q_set_offset(int, char *);
void w25q_get_offset(int);
void w25q_write_dev(int , char *);
void w25q_read_dev(int);



#include"lib_spi.h"
int netlink_w25q_init(void){
	int fd, ret;

	//Create a Netlink Socket
	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_TEST);
	if (fd < 0){
		perror("Socket");
		return -1;
	}
	
	//Set the Source Details 
	memset(&source, 0, sizeof(source));
	source.nl_family = AF_NETLINK;
	source.nl_pid = getpid();	//id to identify socket
	source.nl_groups = 0;		//unicast 

	//bind socket to netlink socket layer
	ret = bind(fd, (struct sockaddr *)&source, sizeof(source));
	if (ret < 0){
		perror("bind");
		return -1;
	}

	return fd;
}

void netlink_w25q_dest(int fd){
	printf("\nAll Links to Device Have been Removed\n");
	close(fd);
}

//##################################################################################
//Setting Offset Code
void w25q_set_offset(int sockfd, char buf[]){
struct sockaddr_nl dest;
struct nlmsghdr *nhl = NULL;
struct iovec iov;
//struct msghdr msg;
struct msghdr msg = { &dest, sizeof(dest), &iov, 1, NULL, 0, 0};
char *newbuf, *temp;
int size;

memset(&dest, 0, sizeof(dest));
dest.nl_family = AF_NETLINK;
dest.nl_pid = 0;			//message is to kernel so always 0
dest.nl_groups = 0;

size = strlen(buf);
//if (! (size == 11 || size == 12) )
//	printf("ENter a Valid Input\n");
//else{
//setting first byte of buf for command
newbuf = (char *)calloc(strlen(buf) + 2, sizeof(char));
temp = newbuf + 1;
strcpy(temp, buf);
newbuf[0] = '1';

nhl = (struct nlmsghdr *)malloc(NLMSG_LENGTH(MAX_SIZE));
memset(nhl, 0, NLMSG_LENGTH(MAX_SIZE));

nhl->nlmsg_len = NLMSG_SPACE(MAX_SIZE);
nhl->nlmsg_flags = 0;
nhl->nlmsg_pid = getpid();
strcpy(NLMSG_DATA(nhl), newbuf);

iov.iov_base = nhl;
iov.iov_len = nhl->nlmsg_len;
/*
msg.msg_name = (void *)&dest;
msg.msg_namelen = sizeof(dest_addr);
msg.msg_iov = &iov;
msg.msg_iovlen = 1;
*/

sendmsg(sockfd, &msg, 0);
recvmsg(sockfd, &msg, 0);

printf("\n%s\n", NLMSG_DATA(nhl));

free(newbuf);
//}

}
//####################################################################################
//Get Offset Code
void w25q_get_offset(int sockfd){
struct sockaddr_nl dest;
struct nlmsghdr *nhl = NULL;
struct iovec iov;
struct msghdr msg = { &dest, sizeof(dest), &iov, 1, NULL, 0, 0};
char *newbuf;

memset(&dest, 0, sizeof(dest));
dest.nl_family = AF_NETLINK;
dest.nl_pid = 0;			//message is to kernel so always 0
dest.nl_groups = 0;

//setting first byte of buf for command
newbuf = (char *)calloc(100, sizeof(char));
newbuf[0] = '2';

nhl = (struct nlmsghdr *)malloc(NLMSG_LENGTH(MAX_SIZE));
memset(nhl, 0, NLMSG_LENGTH(MAX_SIZE));

nhl->nlmsg_len = NLMSG_SPACE(MAX_SIZE);
nhl->nlmsg_flags = 0;
nhl->nlmsg_pid = getpid();
strcpy(NLMSG_DATA(nhl), newbuf);

iov.iov_base = nhl;
iov.iov_len = nhl->nlmsg_len;

sendmsg(sockfd, &msg, 0);
recvmsg(sockfd, &msg, 0);

printf("\n%s\n",  NLMSG_DATA(nhl));

free(newbuf);
}
//####################################################################################
//Read Data from Device
void w25q_read_dev(int sockfd){
struct sockaddr_nl dest;
struct nlmsghdr *nhl = NULL;
struct iovec iov;
struct msghdr msg = { &dest, sizeof(dest), &iov, 1, NULL, 0, 0};
char *newbuf;

memset(&dest, 0, sizeof(dest));
dest.nl_family = AF_NETLINK;
dest.nl_pid = 0;			//message is to kernel so always 0
dest.nl_groups = 0;

//setting first byte of buf for command
newbuf = (char *)calloc(257, sizeof(char));
newbuf[0] = '3';

nhl = (struct nlmsghdr *)malloc(NLMSG_LENGTH(MAX_SIZE));
memset(nhl, 0, NLMSG_LENGTH(MAX_SIZE));

nhl->nlmsg_len = NLMSG_SPACE(MAX_SIZE);
nhl->nlmsg_flags = 0;
nhl->nlmsg_pid = getpid();
strcpy(NLMSG_DATA(nhl), newbuf);

iov.iov_base = nhl;
iov.iov_len = nhl->nlmsg_len;

sendmsg(sockfd, &msg, 0);
recvmsg(sockfd, &msg, 0);

printf("\n%s\n",  NLMSG_DATA(nhl));

free(newbuf);
}
//####################################################################################
//Write Data to Device
void w25q_write_dev(int sockfd, char buf[]){
struct sockaddr_nl dest;
struct nlmsghdr *nhl = NULL;
struct iovec iov;
struct msghdr msg = { &dest, sizeof(dest), &iov, 1, NULL, 0, 0};
char *newbuf, *temp;
int size;

memset(&dest, 0, sizeof(dest));
dest.nl_family = AF_NETLINK;
dest.nl_pid = 0;			//message is to kernel so always 0
dest.nl_groups = 0;

size = strlen(buf);
newbuf = (char *)calloc(strlen(buf) + 2, sizeof(char));
temp = newbuf + 1;
strcpy(temp, buf);
newbuf[0] = '4';

nhl = (struct nlmsghdr *)malloc(NLMSG_LENGTH(MAX_SIZE));
memset(nhl, 0, NLMSG_LENGTH(MAX_SIZE));

nhl->nlmsg_len = NLMSG_SPACE(MAX_SIZE);
nhl->nlmsg_flags = 0;
nhl->nlmsg_pid = getpid();
strcpy(NLMSG_DATA(nhl), newbuf);

iov.iov_base = nhl;
iov.iov_len = nhl->nlmsg_len;

sendmsg(sockfd, &msg, 0);
recvmsg(sockfd, &msg, 0);

printf("\n%s\n", NLMSG_DATA(nhl));

free(newbuf);

}
//####################################################################################


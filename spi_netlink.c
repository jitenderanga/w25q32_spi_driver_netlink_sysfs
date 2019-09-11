#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include"lib_spi.h"

void main(){
int fd, opt;
char offset[30];
char buf[200]={0};
char erase[30];

fd = netlink_w25q_init();
if (fd == -1)
	return ;

printf("Device Socket Successfully Created\n");

printf("W25Q32F Device Driver through Netlink\n");
while(1){

	printf("Select Operation to perform on device:\n");
	printf("1)Set Offset\t2)Get Offset\n3)Read from Device\t4)Write To Device\n5)Erase Device\n6)Exit\n\n");
	scanf("%d", &opt);

	switch(opt){
		case 1:
			printf("ENter the Offset(hex Format)\n");
			getchar();
			printf("block:sector:page<=><0x0-0x40>:<0x0-0xf>:<0x0-0xf>\n");
			fgets(offset, sizeof(offset), stdin);
			w25q_set_offset(fd, offset);
			break;
		case 2:
			w25q_get_offset(fd);
			break;
		case 3:
			w25q_read_dev(fd);
			break;
		case 4:
			printf("ENter the Data you want to write\n");
			getchar();
			fgets(buf, sizeof(buf), stdin);
			w25q_write_dev(fd, buf);
			break;
		case 5:
			printf("Enter Sector to Erase:\n");
			getchar();
			printf("block:sector<=><0x0-0x40>:<0x0-0xf>");
			fgets(erase, sizeof(erase), stdin);
			printf("Sector Erased\n");
			break;
		case 6:
			netlink_w25q_dest(fd);
			exit(0);
		default:
			printf("Wrong option Selected\n\n");
	}
}


}

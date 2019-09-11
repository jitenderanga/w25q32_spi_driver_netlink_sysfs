#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/spi/spi.h>
#include<linux/device.h>
#include<linux/slab.h>
#include<linux/delay.h>
#include<linux/jiffies.h>
#include<linux/spi/eeprom.h>
#include<linux/property.h>
#include<linux/sched.h>
#include"spi.h"
#include<net/sock.h>
#include<linux/netlink.h>
#include<linux/skbuff.h>
#include<linux/string.h>

#define SUCCESS 0;

#define NETLINK_TEST 23

#define MAX_SIZE 1024
static struct sock *nl_sock = NULL;

struct w25_struct *info=NULL;			//declaring a NuLL pointer that will contain info about device

static ssize_t w25_flash_read(struct w25_struct *ptr, char *buf, unsigned offset, size_t count){
        struct spi_transfer t[2];
        struct spi_message m;
        ssize_t status;
        u8 cp[5];
        pr_info(" Reading data from block:sector:page <=> 0x%x:0x%x:0x%x \n",ptr->block, ptr->sector, ptr->page);
        cp[0] = (u8)READ_DATA;
        cp[1] = offset >> 16;
        cp[2] = offset >> 8;
        cp[3] = offset >> 0;
        spi_message_init(&m);
        memset(t, 0, sizeof(t));
        t[0].tx_buf = cp;
        t[0].len = ptr->addr_width/8 + 1;
        spi_message_add_tail(&t[0], &m);

        memset(buf, '\0', IO_LIMIT);
        t[1].rx_buf = buf;
        t[1].len = count;
        spi_message_add_tail(&t[1], &m);

        status = spi_sync(ptr->spi, &m);
        if (status)
                pr_err("read %zd bytes at %d --> %d\n",
                        count, offset, (int) status);

        return status ? status : count;



}

static ssize_t w25_flash_write(struct w25_struct *ptr, const char *buf, loff_t off, size_t count){
        unsigned long   timeout, retries;
        ssize_t         sr, status = 0;
        u8              *temp;
        char            cp[2];

	//checking for write limit
        if (count > IO_LIMIT) {
                pr_err("Data exceeds IO_LIMIT: Write Operation Terminated\n");
                return -EFBIG;
        }

        pr_info(" Writing data to block:sector:page <=> 0x%x:0x%x:0x%x \n",ptr->block, ptr->sector, ptr->page);

	//allocating memory for writing 
        temp = kmalloc(count + ptr->addr_width/8 + 1, GFP_KERNEL);
        if (!temp)
                return -ENOMEM;

	//enabling the write latch before write operation
        cp[0] = (u8)ENABLE_WR;
        status = spi_write(ptr->spi, cp, 1);
        if (status < 0) {
                pr_info("Write Enable --> %d\n", (int) status);
                return status;
        }

	//setting up info as the device and api format
        temp[0] = (u8)WRITE_DATA;
        temp[1] = off >> 16;
        temp[2] = off >> 8;
        temp[3] = off >> 0;
        memcpy(temp+4, buf, count);

	//performing write operation on spi device
        status = spi_write(ptr->spi, temp, count+4);
        if (status<0) {
                kfree(temp);	//freeing memory if write failed 
                return status;
        }

	//setting up the timeout time and converting 'ms into jiffies'
        timeout = jiffies + msecs_to_jiffies(TIMEOUT);
        retries = 0;

        /*Check for write in progress status*/
        do {
		//check for status registed
                sr = spi_w8r8(ptr->spi, RD_STREG);
		//if status register bit is high then write is in progress
                if (sr & SR_CHECK) {
                        msleep(1);	//sleep for 1ms if write in progress
                        continue;
                }
                if (!(sr & SR_CHECK))	//write operation is over
                        break;
        } while (retries++ < 3 || time_before_eq(jiffies, timeout));
	//retries for 3 times max or 25ms

	//freeing up the allocated memory
        kfree(temp);

        return count;
}


static ssize_t w25_spi_read(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
        size_t count = IO_LIMIT;

        return w25_flash_read(info, buf, (int)info->offset, count);
}

static ssize_t w25_spi_write(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count){

	return w25_flash_write(info, buf, info->offset, count);
}

//routine passing offset info to user
static ssize_t spi_get_offset(struct kobject *kobj, struct kobj_attribute *attr, char *user_buf){
	//writing offset info into the user buffer
        return sprintf(user_buf, "block:sector:page\t0x%x:0x%x:0x%x\n",info->block, info->sector, info->page);
}

//routine for setting offset for device
static ssize_t spi_set_offset(struct kobject *kobj, struct kobj_attribute *attr, const char *user_buf, size_t count){
        long result;
        int i,status;
        char temp[10];
        int start=0,end=0,k=0;
        char buff[30];

	//if count is less than greater than value invalid size no need to proceed
        pr_info("Setting Offset: %s count: %d", user_buf, count);
        if (!(count == 13 || count == 12))
                return -EFAULT;

        memset(buff,'\0',sizeof(buff));
        strcpy(buff,user_buf);

        pr_info("buff=%s\n",buff);

	//getting block,sector, and page info
        for (i=0;buff[i]!='\0';i++){

        if (buff[i]==':' ||buff[i+1]=='\0'){
        end=i;
        if (buff[i+1]=='\0')
                end+=1;
        memset(temp,'\0',sizeof(temp));
        strncpy(temp,buff+start,i);
        temp[end-start]='\0';
        start=i+1;
        status=kstrtol(temp,16,&result);
                if (status<0)
                {
                pr_info("not valid\n");
                return 0;
                }

                if (k==0){
                info->block=result;
                k++;
                }
                else if (k==1){
                info->sector=result;
                k++;
                }

                else
                info->page=result;
        }
        }

	//performing check the info is valid or not
        if (info->block > 0x3f || info->sector > 0xf || info->page > 0xf) {
                pr_info("\tOffset must be in following format and range\n");
                pr_info("Blk:sec:page<=><0x0-0x40>:<0x0-0xf>:<0x0-0xf>\n");
                return -EFAULT;
        }
	//setting offset in the info struct of device 
        info->offset = (info->block * 0x10000) + (info->sector * 0x1000) +
                      (info->page * 0x100);

        pr_info(" block:sector:page <=> 0x%x:0x%x:0x%x \n",
                 info->block, info->sector, info->page);


	return count;
}


//sector erase func call
static ssize_t erase_spi(struct kobject *kobj, struct kobj_attribute *attr, const char *user_buf, size_t count){
        long result;
        int i,status;
        unsigned block=0,sector=0;
        char temp[10];
        int start=0,end=0,k=0;
        char buff[30];
	unsigned long offset, timeout, retries;
        ssize_t status1, sr;



        pr_info("Formating this sector: %s\n",user_buf);

        memset(buff,'\0',sizeof(buff));
        strcpy(buff,user_buf);

        pr_info("buff=%s\n",buff);

	//getting the block and sector info 

        for (i=0;buff[i]!='\0';i++){

        if (buff[i]==':' ||buff[i+1]=='\0'){
        end=i;
        if (buff[i+1]=='\0')
                end+=1;
        memset(temp,'\0',sizeof(temp));
        strncpy(temp,buff+start,i);
        temp[end-start]='\0';           
        start=i+1;
        status=kstrtol(temp,16,&result);
                if (status<0)
                {
                pr_info("not valid\n");
                return 0;
                }

                if (k==0){
                block=result;
                k++;
                }
                else if (k==1){
                sector=result;
                k++;
                }
		/*
                else
                page=result;*/
        }
        }
	
	//making sure the offset is vaild or not
	if (block > 0x3f || sector > 0xf){
	pr_info("Offset not vaild plz check\n");
        pr_info("Block:sector <=> <0x0-0x3f>:<0x0-0xf>\n");

	return -EAGAIN;
	}        

	//setting offset to the value which is to be formated/erased
	offset = (block*0x10000) + (sector*0x1000);

        memset(temp, '\0', sizeof(temp));
	
	//enabling the write latch before write operation
        temp[0] = ENABLE_WR;

	//using spi_write api to write data into the device
        status1 = spi_write(info->spi, temp, 1);
        if (status1 < 0)
                pr_info("Write Enable --> %d\n", (int) status1);

	//setting the cmd=SEC_ERASE for erasing the device
        temp[0] = (u8)SEC_ERASE;
        temp[1] = offset >> 16;		//device read offset from MSB to LSB 
        temp[2] = offset >> 8;		//setting offset as per device 
        temp[3] = offset >> 0;		

	//writing data into the device using spi_write api
	status1=spi_write(info->spi, temp, 4);
	if (status1 < 0)
		pr_info("Failed to Erase Disk--> %d\n",(int)status1);

	//setting timeout time in jiffies
        timeout = jiffies + msecs_to_jiffies(TIMEOUT);		//using <jiffies.h> func for conversion 
        retries = 0;
        /*Check for write in progress status*/
        do {
		//read status register 
                sr = spi_w8r8(info->spi, RD_STREG);	//we get 8 bytes
                if (sr & SR_CHECK) {	//if Status reg(u8) is high then this condtion is true and write is in progress
                        msleep(1);	//go to sleep for 1 ms
                        continue;
                }
                if (!(sr & SR_CHECK))	//device has finished writing data
                        break;
        } while (retries++ < 3 || time_before_eq(jiffies, timeout));	//loop run for 3 tries or wait 25ms to pass

	pr_info("block=0x%x:sector=0x%x Erased\n",block,sector);

	return count;
}


//setting up attributes for the kobject instance in sysfs with their respective read and write calls 
static struct kobj_attribute flash_rw = __ATTR(w25q32, 0660, w25_spi_read, w25_spi_write);

static struct kobj_attribute flash_offset = __ATTR(offset, 0660, spi_get_offset, spi_set_offset);

static struct kobj_attribute flash_erase = __ATTR(erase, 0220, NULL, erase_spi);

//initilaizing the attibutes for the files in sysfs...info like 'name' and 'mode' (meta data of file)
static struct attribute *attrs[] = {
	&flash_rw.attr,
	&flash_offset.attr,
	&flash_erase.attr,
	NULL,
};

//grouping all the attributes
static struct attribute_group attr_group = {
	.attrs = attrs,
};


void netlink_test(struct sk_buff *skb){
struct nlmsghdr *nhl;
int pid, res, ret, wsize;
struct sk_buff *skb_out;
int msg_size;
char msg[257] = "Invalid Input";
char *buf, opt, temp[10], *read_buf;
long result;
int i,status;
int start=0, end=0, k=0, flag=0;

pr_info("\n%s invoked\n", __func__);

nhl = (struct nlmsghdr *)skb->data;
pid = nhl->nlmsg_pid;
buf = (char *)nlmsg_data(nhl);
opt = buf[0];
buf = buf + 1;

switch(opt){
	case '1':
	
        pr_info("offset=%s\n",buf);

        //getting block,sector, and page info
        for (i=0;buf[i]!='\0';i++){

        if (buf[i]==':' ||buf[i+1]=='\0'){
        end=i;
        if (buf[i+1]=='\0')
                end+=1;
        memset(temp,'\0',sizeof(temp));
        strncpy(temp,buf+start, i);
        temp[end-start]='\0';
        start=i+1;
        status=kstrtol(temp,16,&result);
                if (status<0)
                {
		st:
                pr_info("not valid\n");
		flag = 1;
		goto out;
                }

                if (k==0){
                info->block=result;
                k++;
                }
                else if (k==1){
                info->sector=result;
			status = kstrtol(buf+start, 16, &result);
			if (status < 0)
				goto st;
			info->page = result;
			goto out;
                k++;
                }

                else
                info->page=result;
        }
        }

out:
	
	if (flag == 0)		
        	if (info->block > 0x3f || info->sector > 0xf || info->page > 0xf)
			flag = 1;

	if (flag == 0){
	info->offset = (info->block * 0x10000) + (info->sector * 0x1000) + (info->page * 0x100);
		sprintf(msg, "Offset Successfully Set");
	        pr_info(" block:sector:page <=> 0x%x:0x%x:0x%x \n", info->block, info->sector, info->page);
	}
		
		goto send;	

		break;

	case '2':
		sprintf(msg, "block:sector:page\t0x%x:0x%x:0x%x\n",info->block, info->sector, info->page);
		pr_info("set offset=%s\n", msg);

		break;

	case '3':
		read_buf = (char *)kzalloc(257, GFP_KERNEL);
	        w25_flash_read(info, read_buf, (int)info->offset, IO_LIMIT);
		sprintf(msg, read_buf);
		kfree(read_buf);
		break;

	case '4':
		pr_info("Data to be Written: %s\n", buf);
		wsize = strlen(buf);
		ret = w25_flash_write(info, buf, info->offset, wsize);
		if (ret < 0)
			sprintf(msg, "Failed To Write to Device\n");
		else
			sprintf(msg, "Data Successfully Written\n");
		break;

	case '5':
		pr_info("Not implemented Yet\n");
		break;

	default:
		pr_info("Wrong Option\n");	

}

send:

msg_size = strlen(msg);

skb_out = nlmsg_new(msg_size, 0);
if (!skb_out){
	pr_err("Failed to Allocate new skb");
	return;

}

nhl = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
NETLINK_CB(skb_out).dst_group = 0;
strncpy(nlmsg_data(nhl), msg, msg_size);

res = nlmsg_unicast(nl_sock, skb_out, pid);
if (res)
	pr_err("Err while Sending Msg\n");

//pr_info("Message Sent=%d\n%s\n", msg_size, msg);
}

static void get_info(struct device *dev){
	
	strcpy(info->name,"w25q32");

	//getting all the required property from the device

	if (device_property_read_u32(dev, "size", &info->size)<0)
		pr_info("Error: Failed to retrive device size info\n");

	if (device_property_read_u32(dev, "pagesize", &info->page_size)<0)
		pr_info("Error: Failed to retrive device pagesize info\n");

	if (device_property_read_u32(dev, "address-width", &info->addr_width)<0)
		pr_info("Error: Failed to retriver deice addr-width info\n");

}

//spi probe func..contain driver specfic initilaziation code 
static int w25_probe(struct spi_device *spi){
	ssize_t id;
	struct netlink_kernel_cfg cfg = { .input = netlink_test, };	
        int ret;

	pr_info("Device Inintailation routine invoked\n");
	//allocating memory for structure which will hold the data
	info=(struct w25_struct *)devm_kzalloc(&spi->dev,sizeof(struct w25_struct), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	//getting info from device tree
	get_info(&spi->dev);
	pr_info("Got the DT property\n");

	info->spi=spi;

	//save the driver info into device structure  &spi->dev->driver_data=info
	spi_set_drvdata(spi, info);

	//read 8bit from spi
	id=spi_w8r8(spi,READ_ID);
	if (id == MANU_ID )		//match for id of the device
		pr_info("Device Manufacture ID Verifed \n");
	else {
		pr_info("Failed to verify Manufacture id\n");
		pr_info("Check the device config or connections\n");
		return id;
	}

	//add kobject instance in sysfs..with name spi_w25q32
	info->kobject = kobject_create_and_add("w25q32", NULL);
	if (!info->kobject){
		pr_info("failed to create a kobject instance in sysfs\n");
		return -EBUSY;
	}

	//create a attribute group in sysfs  
	ret=sysfs_create_group(info->kobject, &attr_group);
	if (ret)
		kobject_put(info->kobject);

	//create a socket that can be used to access driver
	nl_sock = netlink_kernel_create(&init_net, NETLINK_TEST, &cfg);
	if (!nl_sock){
		pr_err("Error Creating Socket\n");
		return -1;
	}

	//printing info of driver in dmesg buffer
	pr_info("Driver registered with sysfs and device ready to use\n");
	pr_info("Size of w25q32 Flash		:%d\n", info->size);
	pr_info("Page_size of w25q32 		:%d\n", info->page_size);
	pr_info("address width			:%d\n", info->addr_width);
	
	return 0;
}

//spi remove func
static int w25_remove(struct spi_device *spi){

	devm_kfree(&spi->dev,info);
	kobject_put(info->kobject);
	netlink_kernel_release(nl_sock);
	pr_info("Device Unregistered success\n");

	return 0;
}


//adding device info to device table...used to match device with driver
static struct of_device_id spi_of_match[] = {
	{ .compatible= "winbond, w25q32" },
	{ }
};

//spi driver struct for containing driver infomation
static struct spi_driver spi_w25_driver = {
	.driver = {
		.name = "w25q32",
		.of_match_table=spi_of_match,
	},
	.probe=w25_probe,		//got invoked when device with compatible device found
	.remove=w25_remove,		
};

//spi module init and exit func (invoked when module is inserted or removed
module_spi_driver(spi_w25_driver);

MODULE_AUTHOR("jitenderanga@gmail.com");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Driver for spi(w25q32)");

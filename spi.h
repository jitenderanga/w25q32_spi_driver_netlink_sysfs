#define READ_ID		0x9F	/*read Manufacturer Id*/
#define ENABLE_WR	0x06	/*Enable Write latch */
#define DISABLE_WR	0x04	/*Reset the Write Latch*/
#define RD_STREG	0x05	/* Read status register*/
#define WR_STREG	0x01	/* Write status register*/
#define READ_DATA	0x03	/* Read data */
#define WRITE_DATA	0x02	/* Write data */

#define SR_CHECK	0x01	/*write in progress or no */
#define SR_WREN		0x02	/*write  enable (latched) */

#define MANU_ID		0xEF	/* Manufacturer Id */
#define SEC_ERASE	0x20	/*Erase Sector */
#define MAX_ADDR	3
#define TIMEOUT		25
#define IO_LIMIT	256 	/*bytes*/


struct w25_struct {
char 			name[15];
struct spi_device	*spi;
struct kobject		*kobject;
long int		offset;
unsigned		size;
unsigned		page_size;
unsigned		addr_width;
unsigned		block;
unsigned		sector;
unsigned 		page;
};


#include "stdio.h"
#include "unistd.h"
#include "sys/type.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
/*********************************************************************

	./ledtest /dev/led 0   // close led
	./ledtest /dev/led 1   // open led

**********************************************************************/

#define	LED_OFF	0
#define	LED_ON	1


int main(int argc, char *argv[])
{
	int fd;
	int retvalue;
	unsigned char databuf[1];

	if(argc <3){
		printf("Error Usage!\r\n");
		return -1;
	}

	filename = argv[1];

	fd = open(filename, O_RDWR);
	if(fd < 0){
		printf("file %s open failed!\r\n", argv[1]);
		return -1;
	}

	databuf[0] = atoi(argv[2]);
	retvalue = write(fd, databuf, sizeof(databuf));
	if(retvalue < 0){
		printf("LED Control Failed!\r\n");
		close(fd);
		return -1;
	}

	retvalue = close(fd);
	if(retvalue < 0){
		printf("file %s close failed!\r\n", argv[1]);
		return -1;
	}
	return 0;
	
}


#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "sting.h"

static char usrdata[] = {"usr data ====!"}

int main(int argc, char *argv[])
{
	int fd;
	int retvalue;
	char *filename;
	char readbuf[100];
	char writebuf[100];

	if(argc != 3){
		printf("Error Usage!\r\n");
		printf("./chrdevbaseApp /dev/chrdevbase 1\r\n");
		return -1;
	}
	filename = argv[1];

	fd = open(filename, O_RDWR);
	if(fd < 0){
		printf("Can't open file %s\r\n",filename);
		return -1;
	}

	if(atoi(argv[2] == 1){
		retvalue = read(fd, readbuf, 50);
		if(retvalue < 0){
			printf("read file %s failed!\r\n",filename);
		}else{
			printf("read data:%s\r\n",readbuf);
		}
	}

	if(atoi(argv[2] == 2){
		memcpy(writebuf, usrdata, sizeof(usrdata));
		retvalue = write(fd, writebuf,50);
		if(retvalue < 0){
			printf("write file %s failed!\r\n", filename);
		}
	}

	retvalue = close(fd);
	if(retvalue < 0){
		printf("Can't close file %s\r\n",filename);
		return -1;
	}

	return 0;
}

#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "linux/ioctl.h"
/***************************************************************
Copyright ? ALIENTEK Co., Ltd. 1998-2029. All rights reserved.
�ļ���		: imx6uirqApp.c
����	  	: ���ҿ�
�汾	   	: V1.0
����	   	: ��ʱ������Ӧ�ó���
����	   	: ��
ʹ�÷���	��./imx6uirqApp /dev/imx6uirq �򿪲���App
��̳ 	   	: www.openedv.com
��־	   	: ����V1.0 2019/7/26 ���ҿ�����
***************************************************************/

/*
 * @description		: main������
 * @param - argc 	: argv����Ԫ�ظ���
 * @param - argv 	: �������
 * @return 			: 0 �ɹ�;���� ʧ��
 */
int main(int argc, char *argv[])
{
	int fd;
	int ret = 0;
	char *filename;
	unsigned char data;
	
	if (argc != 2) {
		printf("Error Usage!\r\n");
		return -1;
	}

	filename = argv[1];
	fd = open(filename, O_RDWR);
	if (fd < 0) {
		printf("Can't open file %s\r\n", filename);
		return -1;
	}

	while (1) {
		ret = read(fd, &data, sizeof(data));
		if (ret < 0) {  /* ���ݶ�ȡ���������Ч */
			
		} else {		/* ���ݶ�ȡ��ȷ */
			if (data)	/* ��ȡ������ */
				printf("key value = %#X\r\n", data);
		}
	}
	close(fd);
	return ret;
}

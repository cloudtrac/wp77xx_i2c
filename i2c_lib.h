// Created January 2021

#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>

#ifndef I2C_WP77XX__
#define I2C_WP77XX__

#define	bmi160_addr 0x68
#define bmi160_cmd 0x7E
#define bmi160_cmd_soft_restart 0x11

#define i2c_buffer_size 32

// i2c stuff

int i2c_fd = -1;
bool i2c_state = false;

typedef struct I2C_DEV{
	unsigned char buf[i2c_buffer_size];
	short address;
}i2cdev;

typedef struct Accel{
	int x;
	int y;
	int z;
}Accel;

typedef struct Gyro{
	int x;
	int y;
	int z;
}Gyro;

bool i2c_init(){

	if (i2c_state == false){
		i2c_fd = open("/dev/i2c-4",O_RDWR);

		if(i2c_fd < 0){
			LE_CRIT( "i2c init: could not access i2c file descriptor");
			i2c_state = false;
			return i2c_state;
		}

		i2c_state = true;
		return i2c_state;
	
	}else{
		LE_INFO("i2c init: bus already initalized");
		return true;
	}
}

bool i2c_write(short addr, short reg, short num, ...){ 

	va_list data;
	va_start(data, num);
	
	size_t size = num+1;
	unsigned char *send_buffer = malloc(size);

	if(send_buffer == NULL){
		LE_ERROR("i2c_write: Could not allocate memory");
		return false;
	}
	
	send_buffer[0] = reg;
	for(int i = 0; i<num; i++){
		send_buffer[i+1] = (unsigned char)va_arg(data, int);
	}

	va_end(data);

	struct i2c_rdwr_ioctl_data burst;
	struct i2c_msg i2c_message;
	
	burst.nmsgs 		= 1;
	burst.msgs 			= &i2c_message;
	
	i2c_message.addr 	= addr;
	i2c_message.buf 	= send_buffer;
	i2c_message.len 	= num+1;
	i2c_message.flags	= 0;

	int result = ioctl(i2c_fd, I2C_RDWR, &burst); //actually send the data

	free(send_buffer);

	if(result < num){ 	//check for the correct number of send bytes
		LE_ERROR("i2c_write: data was not sent properly, %d bytes sent. %s",result,strerror(errno));
		return false;
	}else{
		return true;
	}
}

bool i2c_read(short addr, unsigned char reg, short num, unsigned char *return_buffer){
	
	if(num > i2c_buffer_size){
		LE_ERROR("i2c_read: Buffer size too small for the read size. Icrease buffer size in header file");
		return false;
	}

	return_buffer[0] = reg;

	struct i2c_rdwr_ioctl_data burst;
	struct i2c_msg i2c_message[2];

	burst.nmsgs 			= 2;
	burst.msgs 				= i2c_message;

	i2c_message[0].addr 	= addr;
	i2c_message[0].buf 		= return_buffer;
	i2c_message[0].len 		= 1;
	i2c_message[0].flags	= 0;

	i2c_message[1].addr 	= addr;
	i2c_message[1].buf 		= return_buffer;
	i2c_message[1].len 		= num;
	i2c_message[1].flags	= I2C_M_RD;

	int result = ioctl(i2c_fd, I2C_RDWR, &burst); //actually send the data

	if(result < 0){
		LE_ERROR("i2c_read: data was not read properly. %s",strerror(errno));
		return false;
	}else{
		return true;
	}
}

#endif

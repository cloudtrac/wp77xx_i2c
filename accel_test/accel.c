#include "legato.h"
#include "interfaces.h"
#include "i2c_lib.h"

i2cdev imu;

Accel accel;
Gyro gyro;

COMPONENT_INIT{

	i2c_init();

	//////////
  
  // The i2c_write funciton requires at least four arguments
  // Argument 1 is the 7 lower bits of the i2c address
  // Argument 2 is the address of the register you want to write to
  // Argument 3 is the number of bytes you want to send
  // Arguments >= 4 are the bytes you want to send. The function is known as a variadic 
  //function and you can pass as many bytes as you want here, but you must tell the function 
  //how many you want to send via argument 3.
  
  // The i2c_read function is almost identical- it requires exactly four arguments though.
  // Argument 1 is the 7 lower bits of the i2c address
  // Argument 2 is the address of the register you want to read from
  // Argument 3 is the number of bytes you want to read
  // Argument 4 is the pointer (address) to place the bytes. 
  
  // When initalizing the i2cdev "imu" it creates 32 bytes worth of space for the read function. 
  // If you need more room just bump that number up. 32 was arbitrarily chosen.
  
  // The program below communicates with the mangoh red's on board BMI160 and creates a csv file 
  //with ~20s worth of data recorded at ~20Hz.
  
	if(i2c_write(bmi160_addr, bmi160_cmd, 1, 0x11)){
		LE_INFO("BMI160 soft restart successful");
	}else{
		LE_INFO("BMI160 soft restart failed");
	}

	if(i2c_write(bmi160_addr, 0x41, 1, 0x0C)){
		LE_INFO("Accelerometer limit set to 16g");
	}else{
		LE_INFO("Acceleromter limit not set");
	}

	if(i2c_write(bmi160_addr, 0x43, 1, 0x02)){
		LE_INFO("Gyro limit set to 500 degrees per second");
	}else{
		LE_INFO("Gyro limit not set");
	}

	if(i2c_write(bmi160_addr, 0x69, 1, 0x7D)){ //not sure that this is effective
		LE_INFO("Calibrating accel and gyro for 250ms");
		usleep(250000);
	}else{
		LE_INFO("Calibration not performed");
	}

	if(i2c_read(bmi160_addr, 0x20, 2, imu.buf)){
		LE_INFO("temp data: 0x%02x,0x%02x", imu.buf[0],imu.buf[1]);

		int temp_raw = imu.buf[0] + (imu.buf[1] << 8);
		float temp = (temp_raw * 0.002) + 23;

		LE_INFO("temp: %f degrees celcius", temp);
	}

	////////////////////////////////////////////

	accel.x = 0;
	accel.y = 0;
	accel.z = 0;

	gyro.x = 0;
	gyro.y = 0;
	gyro.z = 0;

	char title[] = "A.x,A.y,A.z,G.x,G.y,G.z\r\n";
	int fd = open("/home/root/imu_log.csv", O_RDWR | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	write(fd, title, sizeof(title));

	for(int l = 0; l < 400; l++){

		i2c_read(bmi160_addr, 0x00, 24, imu.buf);

		accel.x = imu.buf[0x12] + (imu.buf[0x13] << 8);
		accel.y = imu.buf[0x14] + (imu.buf[0x15] << 8);
		accel.z = imu.buf[0x16] + (imu.buf[0x17] << 8);

		gyro.x = imu.buf[0x0C] + (imu.buf[0x0D] << 8);
		gyro.y = imu.buf[0x0E] + (imu.buf[0x0F] << 8);
		gyro.z = imu.buf[0x10] + (imu.buf[0x11] << 8);

		LE_INFO("Accel X:%hi\tY:%hi\tZ:%hi\t\tGyro X:%hi\tY:%hi\tZ:%hi", accel.x, accel.y, accel.z, gyro.x, gyro.y, gyro.z);
		
		char len = snprintf(NULL, 0, "%hi,%hi,%hi,%hi,%hi,%hi\r\n", accel.x, accel.y, accel.z, gyro.x, gyro.y, gyro.z);
		char buffer[len];
		snprintf(buffer, len, "%hi,%hi,%hi,%hi,%hi,%hi\r\n", accel.x, accel.y, accel.z, gyro.x, gyro.y, gyro.z);
		write(fd, buffer, sizeof(buffer));

		usleep(50000);

	}
	close(fd);
	close(i2c_fd);
	exit(0);

}

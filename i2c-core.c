#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
//#include <sys/stat.h>
//#include <sys/time.h>
#include <sys/ioctl.h>

#include "i2c-core.h"

void delay_us(int utime)
{
#if 0
	usleep(utime);
#elif 0
	volatile long int i, j;
	for(j = 0; j <6000; )
			j += 1;
#else
	volatile long long i, j;
	for(i = 0; i < utime; i++)
		for(j = 0; j < 35; )
			j += 1;
#endif
}

void delay_ms(int mtime)
{
#if 1
	int i;
	for(i = 0; i < 1000; i++)
		delay_us(mtime);
#else
	int i, j;
	for(i = 0; i < utime*8; i++)
		for(j = 0; j < 60*47; j++)
			;
#endif

}

char gp_gpio_open_flag[24*8] = {0};

#if 0

int gpio_export(int gpio_index)
{
	if( !gp_gpio_open_flag[gpio_index] )
	{
		char path_buf[64] = {0};
		snprintf( path_buf, 64, "/sys/class/gpio/gpio%d", (unsigned int)gpio_index );
		if(0 != access( path_buf, F_OK)){
			FILE* pGpio = fopen("/sys/class/gpio/export", "w");
			if( pGpio ){
				char ibuf[16] = {0};
				snprintf( ibuf, 16, "%d", gpio_index );
				fwrite( ibuf, 1, strlen(ibuf), pGpio );
				fclose( pGpio );
				usleep(10);
				if(0 == access( path_buf, F_OK)){
					gp_gpio_open_flag[gpio_index] = 1;
				}else{
					KPRINT("error gpio path exist %s\n", path_buf );
				}
			}else{
				KPRINT("error open path /sys/class/gpio/export\n" );
			}
		}
	}
	return 0;
}


/*	
 *	Set the direction of the single bit for the specific path 
 * 	dirbit=1 means output
 * 	dirbit=0 means input
 */
int gpio_dirsetbit(int gpio_index, unsigned int dirbit)
{
	int ret = -1;
	char path_buf[64] = {0};
	FILE* pGpio = NULL;
	gpio_export(gpio_index);
	snprintf( path_buf, 64, "/sys/class/gpio/gpio%d/direction", (unsigned int)gpio_index );
	pGpio = fopen(path_buf, "w");
	if( pGpio ){
		if( dirbit ){
			char ibuf[8] = {"out"};
			fwrite( ibuf, 1, 3, pGpio );
			fclose( pGpio );
		}else{
			char ixbuf[8] = {"in"};
			fwrite( ixbuf, 1, 2, pGpio );
			fclose( pGpio );
		}
	}else{		
		KPRINT("error open path %s\n", path_buf );
		return -1;
	}
	return 0;  
}


/*
 *	Bit write the value from a specific bit of a gpio path
 */
int gpio_writebit(int gpio_index, unsigned int bitvalue)
{
	char path_buf[64] = {0};
	FILE* pGpio = NULL;
	gpio_export(gpio_index);
	snprintf( path_buf, 64, "/sys/class/gpio/gpio%d/value", (unsigned int)gpio_index );
	pGpio = fopen(path_buf, "w");
	if( pGpio ){
		char ibuf[8] = {0};
		snprintf( ibuf, 8, "%d", bitvalue ? 1 : 0 );
		fwrite( ibuf, 1, 1, pGpio );
		fclose( pGpio );
	}else{
		KPRINT("error open path %s\n", path_buf );
		return -1;
	}
	return 0;
}


/*
 *	Bit read the value from a specific bit of a gpio path
 */
int gpio_readbit(int gpio_index, unsigned int * readbit)
{
	unsigned int readvalue;
	FILE* pGpio = NULL;
	char path_buf[64] = {0};
	gpio_export(gpio_index);
	snprintf( path_buf, 64, "/sys/class/gpio/gpio%d/value", (unsigned int)gpio_index );
	pGpio = fopen(path_buf, "r");
	if( pGpio ){
		char rbuf[1] = {0};
		int rlenx = 0;
		rlenx = fread( rbuf, 1, 1, pGpio );
		fclose( pGpio );
		if( 1 == rlenx  ){
			if( '1' == rbuf[0] ){
				readvalue = 1;
			}else{
				readvalue = 0;
			}
		}else{
			KPRINT("error read buf %s\n", path_buf );
			return -1;
		}
	}else{
		KPRINT("error open path %s\n", path_buf );
		return -1;
	}
	usleep(5);
	*readbit = readvalue;
	return 0;
}

#else

static int gpio_export(int gpio_index)
{	
	int fd = -1;
	//printf("set export \n");
	if( !gp_gpio_open_flag[gpio_index] )
	{
		char path_buf[64] = {0};
		snprintf( path_buf, 64, "/sys/class/gpio/gpio%d", (unsigned int)gpio_index );
		if(0 != access( path_buf, F_OK)){
			fd = open("/sys/class/gpio/export", O_WRONLY |O_NOCTTY | O_NONBLOCK);
			if( fd > 0 ){
				char ibuf[16] = {0};
				snprintf( ibuf, 16, "%d", gpio_index );
				write( fd, ibuf, strlen(ibuf));
				close( fd );
				//usleep(5);
				delay_us(5);
				if(0 == access( path_buf, F_OK)){
					gp_gpio_open_flag[gpio_index] = 1;
				}else{
					KPRINT("error gpio path exist %s\n", path_buf );
				}
			}else{
				KPRINT("error open path /sys/class/gpio/export\n" );
			}
		}
	}
	return 0;
}


/*	
 *	Set the direction of the single bit for the specific path 
 * 	dirbit=1 means output
 * 	dirbit=0 means input
 */
static int gpio_dirsetbit(int gpio_index, unsigned int dirbit)
{
	int ret = -1;
	int fd = -1;
	char path_buf[64] = {0};
	//printf("set dir \n");
	gpio_export(gpio_index);
	snprintf( path_buf, 64, "/sys/class/gpio/gpio%d/direction", (unsigned int)gpio_index );
	fd= open(path_buf, O_RDWR |O_NOCTTY | O_NONBLOCK);
	if( fd > 0 ){
		if( dirbit ){
			char ibuf[8] = {"out"};
			write( fd, ibuf, strlen(ibuf) );
			close( fd );
		}else{
			char ixbuf[8] = {"in"};
			write( fd, ixbuf, strlen(ixbuf) );
			close( fd );
		}
	}else{		
		KPRINT("error open path %s\n", path_buf );
		return -1;
	}
	return 0;  
}


/*
 *	Bit write the value from a specific bit of a gpio path
 */
static int gpio_writebit(int gpio_index, unsigned int bitvalue)
{	
	int fd = -1;
	char path_buf[64] = {0};
	//printf("set write\n");
	gpio_export(gpio_index);
	snprintf( path_buf, 64, "/sys/class/gpio/gpio%d/value", (unsigned int)gpio_index );
	fd = open(path_buf, O_RDWR |O_NOCTTY | O_NONBLOCK);
	if( fd > 0 ){
		char ibuf[8] = {0};
		snprintf( ibuf, 8, "%d", bitvalue ? 1 : 0 );
		write(fd, ibuf, strlen(ibuf));
		close( fd );
	}else{
		KPRINT("error open path %s\n", path_buf );
		return -1;
	}
	return 0;
}


/*
 *	Bit read the value from a specific bit of a gpio path
 */
static int gpio_readbit(int gpio_index, unsigned int * readbit)
{
	int fd = -1;
	unsigned int readvalue;
	char path_buf[64] = {0};
	//printf("set read\n");
	gpio_export(gpio_index);
	snprintf( path_buf, 64, "/sys/class/gpio/gpio%d/value", (unsigned int)gpio_index );
	fd  = open(path_buf, O_RDWR |O_NOCTTY | O_NONBLOCK);
	if( fd > 0 ){
		char rbuf[1] = {0};
		int rlenx = 0;
		rlenx = read( fd, rbuf, 1);
		close( fd );
		if( 1 == rlenx  ){
			if( '1' == rbuf[0] ){
				readvalue = 1;
			}else{
				readvalue = 0;
			}
		}else{
			KPRINT("error read buf %s\n", path_buf );
			return -1;
		}
	}else{
		KPRINT("error open path %s\n", path_buf );
		return -1;
	}
	delay_us(2);
	//usleep(2);
	*readbit = readvalue;
	return 0;
}

#endif


int i2c_gpio_init(void)
{
	int ret;
	
	KPRINT( "version - 201506041126" );
	KPRINT( "+->TD35xx\n" );
	
#if 0
	struct timeval tvx={0};
	struct timeval tvy={0};
	unsigned long long ullx = 0;
	unsigned long long ully = 0;
	
	gettimeofday(&tvx, 0);
	delay_us(1);
	gettimeofday(&tvy, 0);
	
	ullx = tvx.tv_sec;
	ullx *= 1000;
	ullx *= 1000;
	ullx += tvx.tv_usec;

	ully = tvy.tv_sec;
	ully *= 1000;
	ully *= 1000;
	ully += tvy.tv_usec;
	
	KPRINT("cm_Delay = %llu\n", (ully -ullx) );
#endif

#if 0
	ret = system( "himm 0x1f0010e4 0x1400" );//set gpio0_1
	if ( -1 == ret )
	{
		KPRINT("error himm 0x1f0010e4 0x1400\n");
	}
	ret = system( "himm 0x1f0010e8 0x1400" );//set gpio0_2
	if ( -1 == ret )
	{
		KPRINT("error himm 0x1f0010e8 0x1400\n");
	}
#else
	ret = system( "himm 0x1f0000dc 0x1500" );
	if ( -1 == ret )
	{
		KPRINT("error himm 0x1f0000dc 0x1500\n");
	}
	ret = system( "himm 0x1f0000e0 0x1400" );
	if ( -1 == ret )
	{
		KPRINT("error himm 0x1f0000dc 0x1400\n");
	}
#endif

	KPRINT("ATSHA204A CryptoMemory Device Driver v1.0.0: %s %s\n", __DATE__, __TIME__);

	return 0;
}

#define GPIO_NUB(gpio_grupo, gpio_bits) 	(gpio_grupo * 8 + gpio_bits)
#if 0
static unsigned int ENCRY_GPIO_SDA = GPIO_NUB(0, 2);	//GPIO0_2
static unsigned int ENCRY_GPIO_SCL = GPIO_NUB(0, 1);	//GPIO0_1
#else 
static unsigned int ENCRY_GPIO_SDA = GPIO_NUB(13, 2);	//GPIO13_2
static unsigned int ENCRY_GPIO_SCL = GPIO_NUB(13, 3);	//GPIO13_3
#endif

#if 1
#define I2C_CLK_OUT     	gpio_dirsetbit(ENCRY_GPIO_SCL, 1) 
#define I2C_CLK_H  		gpio_writebit(ENCRY_GPIO_SCL, 1) 
#define I2C_CLK_L     		gpio_writebit(ENCRY_GPIO_SCL, 0) 

#define I2C_SDA_OUT    	gpio_dirsetbit(ENCRY_GPIO_SDA, 1) 
#define I2C_SDA_IN    		gpio_dirsetbit(ENCRY_GPIO_SDA, 0)
#define I2C_SDA_H     		gpio_writebit(ENCRY_GPIO_SDA, 1) 
#define I2C_SDA_L     		gpio_writebit(ENCRY_GPIO_SDA, 0) 
#else
void clk_out(void){gpio_dirsetbit(ENCRY_GPIO_SCL, 1);};
void clk_h(void){gpio_writebit(ENCRY_GPIO_SCL, 1);};
void clk_l(void){gpio_writebit(ENCRY_GPIO_SCL, 0);};
void sda_out(void){gpio_dirsetbit(ENCRY_GPIO_SDA, 1);};
void sda_in(void){gpio_dirsetbit(ENCRY_GPIO_SDA, 0);};
void sda_h(void){gpio_writebit(ENCRY_GPIO_SDA, 1);};
void sda_l(void){gpio_writebit(ENCRY_GPIO_SDA, 0);};
void I2C_CLK_OUT = clk_out();
void I2C_CLK_H = clk_h();
void I2C_CLK_L = clk_l();

void *(I2C_SDA_OUT)(void) = &sda_out();
void I2C_SDA_IN = sda_in();
void I2C_SDA_H = sda_h();
void I2C_SDA_L = sda_l();

#endif

// 1/2 Clock Cycle transition to HIGH
static void i2c_clk_high(void)
{
	delay_us(2);
	I2C_CLK_H;
	delay_us(4);
}

// 1/2 Clock Cycle transition to LOW
static void i2c_clk_low(void)
{
	delay_us(4);
	I2C_CLK_L;
	delay_us(2);
}

// Do one full clock cycle
// Changed 1/19/05 to eliminate one level of return stack requirements
static void i2c_clk_clock(void)
{
	delay_us(2);
	I2C_CLK_L;
	delay_us(4);
	I2C_CLK_H;
	delay_us(2);
}

// Send a start sequence
// Modified 7-21-04 to correctly set SDA to be an output
static void i2c_start(void)
{
	I2C_SDA_OUT;		// Data line must be an output to send a start sequence
	delay_us(10);
	i2c_clk_low();
	I2C_SDA_H;
	delay_us(8);
	i2c_clk_high();
	delay_us(8);
	I2C_SDA_L;
	delay_us(8);
	i2c_clk_low();
	delay_us(8);
}

// Send a stop sequence
// Modified 7-21-04 to correctly set SDA to be an output
static void i2c_stop(void)
{
	I2C_SDA_OUT;                         // Data line must be an output to send a stop sequence
	delay_us(1020);
	i2c_clk_low();
	I2C_SDA_L;
	delay_us(8);
	i2c_clk_high();
	delay_us(8);
	I2C_SDA_H;
	delay_us(8);
}



// Do a number of clock cycles
static void i2c_clk_clock_cycles(uint8_t ucCount)
{
	uint8_t i;

	for (i = 0; i < ucCount; ++i)
		i2c_clk_clock();
}


static void i2c_wait_clock(uint8_t loop)
{
	uint8_t i, j;

	I2C_SDA_L;
	for(j=0; j<loop; j++){
		i2c_start();
		for(i = 0; i<15; i++)
			i2c_clk_clock();
		i2c_stop();
	}
}




// Send a ACK or NAK or to the device
static void back_ack_nak(uint8_t ucAck)
{
	I2C_SDA_OUT;                         // Data line must be an output to send an ACK
	delay_us(10);
	i2c_clk_low();
	if (ucAck) 
		I2C_SDA_L;               // Low on data line indicates an ACK
	else       
		I2C_SDA_H;               // High on data line indicates an NACK
	delay_us(10);
	i2c_clk_high();
	delay_us(10);
	i2c_clk_low();
}


// Write a byte
// Returns 0 if write successed, 1 if write fails failure
// Modified 7-21-04 to correctly control SDA
static uint8_t i2c_write(uint8_t ucData)
{
	uint8_t i;
	unsigned int value;

	I2C_SDA_OUT;						// Set data line to be an output
	delay_us(10);
	for(i=0; i<8; i++){				// Send 8 bits of data
		i2c_clk_low();

		if (ucData & 0x80)
			I2C_SDA_H;
		else
			I2C_SDA_L;

		i2c_clk_high();
		ucData = ucData<<1;
	}
	i2c_clk_low();

	// wait for the ack
	I2C_SDA_IN;                      // Set data line to be an input
	delay_us(10);

	i2c_clk_high();
	while(i>1) {                   	// loop waiting for ack (loop above left i == 8)
		delay_us(10);

		gpio_readbit(ENCRY_GPIO_SDA, &value);
		//KPRINT("read bit %d,%d\n", i, value);

		if (value == 1)
			i--;
		else
			i = 0;
	}
	i2c_clk_low();

	I2C_SDA_OUT;                     // Set data line to be an output
	delay_us(20);
	
	return i;
}

// Read a byte from device, MSB
// Modified 7-21-04 to correctly control SDA
static uint8_t i2c_read(void)
{
	uint8_t i;
	unsigned int value;
	uint8_t rByte = 0;

	I2C_SDA_IN;                          // Set data line to be an input
	I2C_SDA_H;
	delay_us(10);
	for(i=0x80; i; i=i>>1){
		i2c_clk_clock();
		gpio_readbit(ENCRY_GPIO_SDA, &value);

		if (value == 1)
			rByte |= i;

		i2c_clk_low();
	}
	I2C_SDA_OUT;                         // Set data line to be an output
	delay_us(10);
	
	return rByte;
}



static uint8_t i2c_receive_data(uint8_t ucLen, uint8_t* pucRecBuf)
{
	int i;

	for(i = 0; i < (ucLen-1); i++){
		pucRecBuf[i] = i2c_read();
		back_ack_nak(TRUE);
	}
	pucRecBuf[i] = i2c_read();
	back_ack_nak(FALSE);
	i2c_stop();
	return SUCCESS;
}

// Send a byte
static uint8_t i2c_send_byte(uint8_t cmd)
{
	uint8_t i;

	i = CM_START_TRIES;
	
	while (i) {
		if (i2c_write(cmd) == 0) 
			break;
		if (--i == 0) 
			return FAIL_CMDSTART;
	}

	return SUCCESS;
}

static uint8_t i2c_send_data(uint8_t ucLen, uint8_t* pucSendBuf)
{
	int i;
	for(i = 0; i< ucLen; i++) {
		if (i2c_send_byte(pucSendBuf[i]) == 1)
			return FAIL_WRDATA;
	}

	return SUCCESS;
}



/***************************************************************************************
���ܣ�sha204-wakeup
˵����sha204-I2Cģʽ���ѡ�SDA��High��Low,���ͱ���>60us������һ�ֽ�0x00��SDA���ָߵ�ƽ>3MS
***************************************************************************************/
static void sha204p_wakeup(void)
{  
	i2c_start(); 		      				//��I2Cͨ�ţ�SDA����
	delay_us(70);						//SDA���ֵ͵�ƽ>60us
	i2c_send_byte(0x00);     			//��һ�ֽ�����
	i2c_stop();			  			//��I2Cͨ�ţ�SDA����
	delay_ms(50);						//SDA���ֵ͵�ƽ>3MS
}

/***************************************************************************************
���ܣ�sha204-send_command
˵����sha204-I2Cģʽ�����ݰ�
***************************************************************************************/
uint8_t sha204p_send_command(uint8_t count, uint8_t *command)
{   	
	uint8_t result=0;                        			//��I2Cͨ��     
	i2c_start();
	while(i2c_send_byte(0xc8));	       		//sha204-д��ַ
	while(i2c_send_byte(0x03));	       		//sha204-����-��������ͨ�ŷ�ʽ
	while(i2c_send_data(count,command));		//���ݰ�����
	i2c_stop();
	return result;
}

/***************************************************************************************
���ܣ�sha204-read
˵����sha204-I2Cģʽ�����ݰ�
***************************************************************************************/
uint8_t sha204p_receive_response(uint8_t size, uint8_t *response)
{
	uint8_t count=0;  
	uint8_t result=0;

	i2c_start();							//��I2Cͨ��
	i2c_send_byte(0xc9);		   			//sha204-����ַ
	response[0]=i2c_read();	   			//����һ���ֽ�-���ݳ���
	back_ack_nak(TRUE);					//��Ӧ��
	count = response[0];					//�ж����ݳ����Ƿ����Ҫ��
	if ((count < size) || (count > size)) {
		i2c_stop();
		result=1;
	}else{
		i2c_receive_data(count - 1, &response[1]); 	//����ʣ������	 	
	}
	
	return result;
}												   

/***************************************************************************************
���ܣ�sha204-wakeup
˵����OUT-*response Ϊ4�ֽڻ�����Ӧֵ
***************************************************************************************/
uint8_t sha204c_wakeup(uint8_t *response)
{
	sha204p_wakeup();						       	//����
	sha204p_receive_response(4, response);		   	//��4�ֽ���Ӧֵһ��
	return 0;
}

/***************************************************************************************
���ܣ�sha204-sleep
˵����
***************************************************************************************/
uint8_t sha204p_sleep(void)
{
	i2c_start();					      	//��I2Cͨ��
	i2c_send_byte(0xc8);                	//sha204-д��ַ
	i2c_send_byte(0x01);				//sha204-����-˯��
	i2c_stop();						//��I2Cͨ��
	return 0;
}		
	
// Power On Chip
// Returns 0 (SUCCESS) if no error
void ic_power_on(void)
{
	I2C_CLK_OUT;		//SCL
	I2C_SDA_OUT	;		//SDA

	delay_us(1);
	I2C_CLK_L;
	delay_us(10);
	I2C_SDA_H;

	delay_ms(2);
	// Chip should now be in sync mode and ready to operate
}

// Shut down secure memory
void ic_power_off(void)
{
	delay_us(1);
	I2C_CLK_L;
	delay_us(6);
}




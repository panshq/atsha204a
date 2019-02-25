#ifndef _I2C_CORE_H_
#define _I2C_CORE_H_


#ifndef FALSE
#define FALSE 	(0)
#define TRUE 	(!FALSE)
#endif

//#define DEBUG			//该宏用于控制打印信息
#ifdef DEBUG
	#define KPRINT(fmt, args...) printf(fmt, ##args)
#else
	#define KPRINT(fmt, args...)
#endif

typedef unsigned char  uint8_t;

// Return Code Defination
#define SUCCESS       (0)
#define FAILED        (1)
#define FAIL_CMDSTART (2)
#define FAIL_CMDSEND  (3)
#define FAIL_WRDATA   (4)
#define FAIL_RDDATA   (5)

#define CM_START_TRIES 	10

void delay_us(int utime);
void delay_ms(int mtime);
int i2c_gpio_init(void);
uint8_t sha204p_send_command(uint8_t count, uint8_t *command);
uint8_t sha204p_receive_response(uint8_t size, uint8_t *response);
uint8_t sha204c_wakeup(uint8_t *response);
uint8_t sha204p_sleep(void);
void ic_power_on(void);
void ic_power_off(void);


#endif

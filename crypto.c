#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#include "i2c-core.h"
#include "sha204-core.h"
#include "crypto.h"

//#define CALC_TIME

void authentication_on()
{
	i2c_gpio_init();
	//printf("i2c gpio init succ !\n");
	
	ic_power_on();
	//printf("ic power on \n");
}

void authentication_off()
{
	ic_power_off();
}

int get_otp_data(uint8_t* buf)
{
	sha204_read_otp(buf);

}

int get_cfg_data(uint8_t* buf)
{
	sha204_read_cfg(buf);

}

int get_solt_data(uint8_t *secret_key, uint8_t *challenge, uint8_t read_key_id, uint8_t read_solt_id, uint8_t* buf)
{
	sha204_read_solt(secret_key, challenge, read_key_id, read_solt_id, buf);

}

 inline int authentication_main(uint8_t *secret_key, uint8_t *challenge, uint8_t solt_id, int mode)
{
#if 0
	int i = 0, j;
	int ret, num, cont = 0, fail = 0;
		
	i2c_gpio_init();
	printf("i2c gpio init succ !\n");
	
	ic_power_on();
	printf("ic power on \n");

#if 1		
	struct timeval tvx={0};
	struct timeval tvy={0};
	unsigned long long ullx = 0;
	unsigned long long ully = 0;
	
	gettimeofday(&tvx, 0);	
#endif

	num = 2;
	while(num--)	
		for(i = 0; i <= 0xf; i++){
			if(i < 9){
				secret_key[31] = '1' + i;
				secret_key[30] = '0';
			}else{
				secret_key[30] = '1';
				secret_key[31] = '0' + i%9;
			}

			ret = sha204_main_handle(secret_key, challenge, i, 1);
			if(ret){
				cont++;
				//printf(" fail = %d ; cont = %d\n", fail, cont);
			}else
				fail++;
			
		}
#if 1
	gettimeofday(&tvy, 0);
	
	ullx = tvx.tv_sec;
	ullx *= 1000;
	ullx *= 1000;
	ullx += tvx.tv_usec;

	ully = tvy.tv_sec;
	ully *= 1000;
	ully *= 1000;
	ully += tvy.tv_usec;
		
	printf("run 32 time = %llu\n", (ully -ullx) );
#endif	
	printf(" fail = %d ; cont = %d\n", fail, cont);

	ic_power_off();
	
	return 0;
#else

#ifdef CALC_TIME	
	struct timeval tvx={0};
	struct timeval tvy={0};
	unsigned long long ullx = 0;
	unsigned long long ully = 0;
	
	gettimeofday(&tvx, 0);	
#endif

	while(sha204_main_handle(secret_key, challenge, solt_id, mode)){
#ifdef CALC_TIME
		gettimeofday(&tvy, 0);
		
		ullx = tvx.tv_sec;
		ullx *= 1000;
		ullx *= 1000;
		ullx += tvx.tv_usec;

		ully = tvy.tv_sec;
		ully *= 1000;
		ully *= 1000;
		ully += tvy.tv_usec;
			
		printf("check one solt time = %llu\n", (ully -ullx) );
#endif
		
		return 1;
	}

#ifdef CALC_TIME
		gettimeofday(&tvy, 0);
		
		ullx = tvx.tv_sec;
		ullx *= 1000;
		ullx *= 1000;
		ullx += tvx.tv_usec;

		ully = tvy.tv_sec;
		ully *= 1000;
		ully *= 1000;
		ully += tvy.tv_usec;
			
		printf("check one solt time = %llu\n", (ully -ullx) );
#endif

	return 0;
#endif
}


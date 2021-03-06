#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#include "i2c-core.h"
#include "sha204-core.h"
#include "qst_authen.h"

//#define CALC_TIME

void qst_authen_on()
{
	i2c_gpio_init();
	//printf("i2c gpio init succ !\n");
	
	ic_power_on();
	//printf("ic power on \n");
}

void qst_authen_off()
{
	ic_power_off();
}

int get_cfg_data(uint8_t* buf)
{
	sha204_read_cfg(buf);

}

int qst_authen_get_version(IC_INFO *info)
{	
	char buf[64];
	char *ch;
	
	memset(buf, 0x0, sizeof(buf));
	sha204_read_otp(buf);
	//printf("otp:%s \n", buf);//Flag:www.1000video.com.cn-Time:2019.03.01-Version:00.01.000.0000

	memmove(info->id, buf+5, 20);
	memmove(info->time, buf+31, 10);
	memmove(info->version, buf+50, 14);

	return 0;
}

int qst_authen_get_option(IC_OPT *opt)
{
	char buf[64];
	char tmp[5];
	uint8_t read_key[32]={0x53, 0x75, 0x5A, 0x68, 0x6F, 0x75, 0x51, 0x69, 
						0x61, 0x6E, 0x53, 0x68, 0x69, 0x54, 0x6F, 0x6E, 
						0x67, 0x52, 0x65, 0x61, 0x64, 0x4F, 0x70, 0x74, 
						0x69, 0x6F, 0x6E, 0x53, 0x6F, 0x6C, 0x74, 0x30};
	uint8_t challenge[32]={0};
	
	memset(buf, 0x0, sizeof(buf));
	memset(tmp, 0x0, sizeof(tmp));
	memset(challenge, 0x0, sizeof(challenge));
	sha204_read_solt(read_key, challenge, 0x0f, 0x00, buf);

	//printf("otp:%s \n", buf);//otp:Option:0000-0000
	
	strncpy(tmp, buf+7, 4);
	opt->master_opt = strtol(tmp, NULL, 16);
	
	strncpy(tmp, buf+12, 4);
	opt->minor_opt= strtol(tmp, NULL, 16);
	
	return 0;
}

 inline int qst_authen_verify(uint8_t *secret_key, uint8_t *challenge, uint8_t solt_id, int mode)
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


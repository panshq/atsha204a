#include<stdio.h>
#include<string.h>
#include"crypto.h"

unsigned char secret_key_1[32] =  
{
	0x77, 0x77, 0x77, 0x2E, 0x31, 0x30, 0x30, 0x30,
	0x76, 0x69, 0x64, 0x65, 0x2E, 0x63, 0x6F, 0x6D,
	0x2E, 0x63, 0x6E, 0x2D, 0x32, 0x30, 0x31, 0x39,
	0x30, 0x32, 0x31, 0x32, 0x2D, 0x4E, 0x31, 0x36
};

unsigned char challenge_date[32] =
{                                              
	0x31, 0xAE, 0x02, 0xE3, 0xB1, 0xC2, 0x74, 0xCF,
	0x5E, 0xA7, 0x08, 0x40, 0x85, 0x31, 0x30, 0x5D, 
	0x9E, 0xF1, 0x31, 0x99, 0x79, 0x0B, 0x0B, 0x5E, 
	0x0D, 0x7A, 0x2F, 0xB0, 0x68, 0x1A, 0xCC, 0x2E
	
};

int main()
{
	int i;
	int ret = 0;
	unsigned char buf[88];
	
	authentication_on();

#if 1
	memset(buf, 0x0, sizeof(buf));
	get_cfg_data(buf);
	printf("\nread cfg :");
	for(i = 0; i < 88; i++)
		printf("%02X ", buf[i]);
	printf("\n");
#endif 

#if 1
	memset(buf, 0x0, sizeof(buf));
	get_otp_data(buf);
	printf("\nread otp :");
	for(i = 0; i < 64; i++)
		printf("%02X ", buf[i]);
	printf("\n");

#endif

#if 1
	memset(buf, 0x0, sizeof(buf));
	get_solt_data(secret_key_1, challenge_date, 0x0f, 2, buf);
	printf("\nread solt :");
	for(i = 0; i < 32; i++)
		printf("%02X ", buf[i]);
	printf("\n");
	
#endif

	while(authentication_main(secret_key_1, challenge_date, 0x0f, 1)){
		authentication_off();
		printf("check succ \n");	
		return 0;
	}
	
	authentication_off();
	printf("check fail \n");
	return -1;
}

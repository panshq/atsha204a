#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include "sha204-core.h"
#include "i2c-core.h"

volatile int cont = 20;

static unsigned int SHFR(unsigned int x,uint8_t n)
	{return (x>>=n);}
static unsigned int ROTR(unsigned int x,uint8_t n)
	{return (((x >> n) | (x << (32 - n))));}
static unsigned int CH(unsigned int x,unsigned int y,unsigned int z)
	{return(((x & y) ^ (~x & z)));}
static unsigned int MAJ(unsigned int x,unsigned int y,unsigned int z)
	{return(((x & y) ^ (x & z) ^ (y & z)));}
static unsigned int SHA256_F1(unsigned int x)
	{return((ROTR(x,  2) ^ ROTR(x, 13) ^ ROTR(x, 22)));}
static unsigned int SHA256_F2(unsigned int x)
	{return((ROTR(x,  6) ^ ROTR(x, 11) ^ ROTR(x, 25)));}
static unsigned int SHA256_F3(unsigned int x)
	{return((ROTR(x,  7) ^ ROTR(x, 18) ^ SHFR(x,  3)));}
static unsigned int SHA256_F4(unsigned int x)
	{return((ROTR(x, 17) ^ ROTR(x, 19) ^ SHFR(x, 10)));}


static void sha256_Subroutine(uint8_t *Input,unsigned int *Output)
{
	unsigned int w[64];
	unsigned int wv[8];
	unsigned int t1, t2;
	uint8_t j;
	unsigned int sha256_k[64] ={
		0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
		 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
		 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
		 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
		 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
		 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
		 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
		 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
		 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
		 0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
		 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
		 0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
		 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
		 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
		 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
	 };

	for (j = 0; j < 16; j++) {
		w[j]=  ((unsigned int)Input[4*j+3])+
		    (((unsigned int)Input[4*j+2])<<8)+
		    (((unsigned int)Input[4*j+1])<<16)+
		    (((unsigned int)Input[4*j])  << 24);    
	}

	for (j = 16; j < 64; j++) {
		w[j]= SHA256_F4(w[j-2]) + w[j-7]+ SHA256_F3(w[j - 15]) + w[j-16];
	}

	for (j = 0; j < 8; j++) {
		wv[j] = Output[j];
	}

	for (j = 0; j < 64; j++) {
		t1 = wv[7] + SHA256_F2(wv[4]) + CH(wv[4], wv[5], wv[6])+ sha256_k[j] + w[j];
		t2 = SHA256_F1(wv[0]) + MAJ(wv[0], wv[1], wv[2]);
		wv[7] = wv[6];
		wv[6] = wv[5];
		wv[5] = wv[4];
		wv[4] = wv[3] + t1;
		wv[3] = wv[2];
		wv[2] = wv[1];
		wv[1] = wv[0];
		wv[0] = t1 + t2;
	}

	for (j = 0; j < 8; j++) {
		Output[j] += wv[j];
	}
}


static void sha256_handle(uint8_t *Input,uint8_t *Output)
{
	unsigned int sha256_h[8];
	uint8_t sha256_block[64];
	uint8_t i=0;

	sha256_h[0] = 0x6a09e667;
	sha256_h[1] = 0xbb67ae85;
	sha256_h[2] = 0x3c6ef372; 
	sha256_h[3] = 0xa54ff53a;
	sha256_h[4] = 0x510e527f; 
	sha256_h[5] = 0x9b05688c;
	sha256_h[6] = 0x1f83d9ab; 
	sha256_h[7] = 0x5be0cd19;

	memmove(sha256_block, Input, 64);

	sha256_Subroutine(sha256_block,sha256_h); 

	memmove(sha256_block, Input+64, 24);
	memset(sha256_block+24, 0x0, 40);
		
	sha256_block[24]=0x80;
	sha256_block[60]=0x00;
	sha256_block[61]=0x00;
	sha256_block[62]=0x02;
	sha256_block[63]=0xc0;

	sha256_Subroutine(sha256_block,sha256_h);

	for (i = 0 ; i < 8; i++) {
		Output[4*i]  =((uint8_t)(sha256_h[i] >> 24));
		Output[4*i+1]=((uint8_t)(sha256_h[i] >> 16));
		Output[4*i+2]=((uint8_t)(sha256_h[i] >>  8));
		Output[4*i+3]=((uint8_t)(sha256_h[i]));
	} 
}


static uint8_t sha204_calculate_mac(uint8_t *key,uint8_t *challenge,uint8_t solt_id,uint8_t *mac_result)
{
	uint8_t i;
	uint8_t p_temp[88];

	memset(p_temp, 0x0, sizeof(p_temp));
	memmove(p_temp, key, 32);
	memmove(p_temp+32, challenge, 32);

	p_temp[64]=0x08;		       //opcode
	p_temp[65]=0x00;		       //mode
	p_temp[66]=solt_id;	       //solt_id:0x00-0x0f
	p_temp[67]=0x00;		       //0x00

	for(i=0;i<11;i++){	       	//OPT[0:10] --opcode=0x08 : 0x00
		p_temp[i+68]=0x00;
	}
			 
	p_temp[79] = 0XEE;                   //SN[8]
#if 0
	p_temp[80] = 0XC2;
	p_temp[81] = 0X9E;
	p_temp[82] = 0XFC;
	p_temp[83] = 0X7F;
#else	
	for(i=0;i<4;i++){		     //SN[4:7] --opcode=0x08:0x00
		p_temp[i+80]=0x00;
	}
#endif

	p_temp[84] = 0x01;		    //SN[0]
	p_temp[85] = 0x23;		    //SN[1]
#if 0
	p_temp[86] = 0x9C;		    //SN[2]	 --opcode=0x08:0x00
	p_temp[87] = 0x93;		    //SN[3]	 --opcode=0x08:0x00
#else
	p_temp[86] = 0x0;		    //SN[2]	 --opcode=0x08:0x00
	p_temp[87] = 0x0;		    //SN[3]	 --opcode=0x08:0x00
#endif

	sha256_handle(p_temp,mac_result);

	return 0;
}

static void sha204c_calculate_crc(uint8_t length, uint8_t *date, uint8_t *crc)
{
	uint8_t counter;
	uint16_t crc_register = 0;
	uint16_t polynom = 0x8005;

	uint8_t shift_register;
	uint8_t data_bit, crc_bit;

	for (counter = 0; counter < length; counter++) {
		for (shift_register = 0x01; shift_register > 0x00; shift_register <<= 1) {
	 		data_bit = (date[counter] & shift_register) ? 1 : 0;
	 		crc_bit = crc_register >> 15;
	 		crc_register <<= 1;
			 if (data_bit != crc_bit)
				crc_register ^= polynom;
		}		
	}
	crc[0] = (uint8_t) (crc_register & 0x00FF);
	crc[1] = (uint8_t) (crc_register >> 8);
} 



static void sha256_handle_nonce(uint8_t *Input,uint8_t *Output)
{
	unsigned int sha256_h[8];
	uint8_t sha256_block[64];
	uint8_t i=0;

	sha256_h[0] = 0x6a09e667;
	sha256_h[1] = 0xbb67ae85;
	sha256_h[2] = 0x3c6ef372;
	sha256_h[3] = 0xa54ff53a;
	sha256_h[4] = 0x510e527f;
	sha256_h[5] = 0x9b05688c;
	sha256_h[6] = 0x1f83d9ab;
	sha256_h[7] = 0x5be0cd19;

	memmove(sha256_block, Input, 55);
	memset(sha256_block+55, 0x0, 9);

	sha256_block[55]=0x80;

	sha256_block[60]=0x00;
	sha256_block[61]=0x00;
	sha256_block[62]=0x01;
	sha256_block[63]=0xb8;

	sha256_Subroutine(sha256_block,sha256_h);

	for (i = 0 ; i < 8; i++){
		Output[4*i]  =((uint8_t)(sha256_h[i] >> 24));
		Output[4*i+1]=((uint8_t)(sha256_h[i] >> 16));
		Output[4*i+2]=((uint8_t)(sha256_h[i] >>  8));
		Output[4*i+3]=((uint8_t)(sha256_h[i]));
	}
}

static inline void sha256_handle_gendig(uint8_t *Input,uint8_t *Output)
{
	unsigned int sha256_h[8];
	uint8_t sha256_block[64];
	uint8_t i=0;

	sha256_h[0] = 0x6a09e667;
	sha256_h[1] = 0xbb67ae85;
	sha256_h[2] = 0x3c6ef372;
	sha256_h[3] = 0xa54ff53a;
	sha256_h[4] = 0x510e527f;
	sha256_h[5] = 0x9b05688c;
	sha256_h[6] = 0x1f83d9ab;
	sha256_h[7] = 0x5be0cd19;

	memmove(sha256_block, Input, 64);

	sha256_Subroutine(sha256_block,sha256_h);

	memmove(sha256_block, Input+64, 32);
	memset(sha256_block+32, 0x0, 32);
	sha256_block[32]=0x80;

	sha256_block[60]=0x00;
	sha256_block[61]=0x00;
	sha256_block[62]=0x03;
	sha256_block[63]=0x00;

	sha256_Subroutine(sha256_block,sha256_h);

	for (i = 0 ; i < 8; i++){
		Output[4*i]  =((uint8_t)(sha256_h[i] >> 24));
		Output[4*i+1]=((uint8_t)(sha256_h[i] >> 16));
		Output[4*i+2]=((uint8_t)(sha256_h[i] >>  8));
		Output[4*i+3]=((uint8_t)(sha256_h[i]));
	}
}


static inline uint8_t sha204h_nonce(struct sha204h_nonce_in_out param)
{
	// Local Variables
	uint8_t temporary[55];
	uint8_t *p_temp;

	p_temp = temporary;
		
	memcpy(p_temp, param.rand_out, 32);
	p_temp += 32;
		
	memcpy(p_temp, param.num_in, 20);
	p_temp += 20;
		
	*p_temp++ = 0x16;
	*p_temp++ = param.mode;
	*p_temp++ = 0x00;
		
	sha256_handle_nonce(temporary,param.temp_key->value);
		
	param.temp_key->source_flag = 0;
	param.temp_key->key_id = 0;
	param.temp_key->gen_data = 0;
	param.temp_key->check_flag = 0;
	param.temp_key->valid = 1;
	
	return 0x00;
}

static uint8_t sha204h_gen_dig(struct sha204h_gen_dig_in_out param)
{
	// Local Variables
	uint8_t temporary[96];
	uint8_t i;
	uint8_t *p_temp;

	p_temp = temporary;

	memcpy(p_temp, param.stored_value, 32);
	p_temp += 32;
	
	*p_temp++ = 0x15;
	
	*p_temp++ = param.zone;
	
	*p_temp++ = param.key_id & 0xFF;
	*p_temp++ = (param.key_id >> 8) & 0xFF;
	
	*p_temp++ =0xee;
	*p_temp++ =0x01;
	*p_temp++ =0x23;

	memset(p_temp, 0x0, 25);
	p_temp += 25;
	
	memcpy(p_temp, param.temp_key->value, 32);
	
	sha256_handle_gendig(temporary,param.temp_key->value);

	param.temp_key->valid = 1;
	
	if ((param.zone ==0x02) && (param.key_id <= 15)){
		param.temp_key->gen_data = 1;
		param.temp_key->key_id = (param.key_id & 0xF);    // mask lower 4-bit only
	}else{
		param.temp_key->gen_data = 0;
		param.temp_key->key_id = 0;
	 }
	return 0x00;
}


static uint8_t sha204h_decrypt(struct sha204h_decrypt_in_out param)
{
	uint8_t i;
	// Decrypt by XOR-ing Data with the TempKey
	for (i = 0; i < 32; i++){
		param.data[i] ^= param.temp_key->value[i];
	}
	// Update TempKey fields
	param.temp_key->valid = 0;
	
	return 0x00;
}

static uint8_t sha204m_execute_random( uint8_t *random_buf)
{
	uint8_t i=0;
	uint8_t p_buffer[2];
	uint8_t tx_buffer[39];

	tx_buffer[0]=07;	          //len
	tx_buffer[1]=0x1B;		  //opcode
	tx_buffer[2]=0x00;		  //mode
	tx_buffer[3]=0x00;		  //zero bit0
	tx_buffer[4]=0x00;		  //zero bit1
	
	sha204c_calculate_crc(5,tx_buffer, p_buffer);
	tx_buffer[5]=p_buffer[0];
	tx_buffer[6]=p_buffer[1];

	#if 0
	printf("send:");
	for(i = 0; i < 7; i++)
		printf("%02X ", tx_buffer[i]);
	printf("\n");
	#endif	
	sha204p_send_command(7,tx_buffer);

	delay_ms(25);
	i = cont;
	while(sha204p_receive_response(35, random_buf) && i)
		i--;

	#if 0
	printf("random:");
	for(i = 0; i < 35; i++)
		printf("%02X ", random_buf[i]);
	printf("\n");
	#endif
	
	if(i > 0)
		return 0;
	else 
		return -1;
}


static uint8_t sha204m_execute_nonce( uint8_t *nonce_data ,uint8_t *rx_buffer)
{
	uint8_t i=0;
	uint8_t p_buffer[2];
	uint8_t tx_buffer[39];

	tx_buffer[0]=27;	          //len
	tx_buffer[1]=0x16;		  //opcode
	tx_buffer[2]=0x01;		  //mode
	tx_buffer[3]=0x00;		  //sol_id(0x00-0x0f)
	tx_buffer[4]=0x00;		  //NA

	memmove(tx_buffer+5, nonce_data, 20);
	
	sha204c_calculate_crc(25,tx_buffer, p_buffer);	
	tx_buffer[25]=p_buffer[0];
	tx_buffer[26]=p_buffer[1];

	sha204p_send_command(27,tx_buffer);

	delay_ms(25);
	i = cont;
	while(sha204p_receive_response(35, rx_buffer) && i)
		i--;

	if(i > 0)
		return 0;
	else 
		return -1;
}


static uint8_t sha204m_execute_gendig(uint8_t key_id,uint8_t *response_buffer)
{	
	int i;
	uint8_t p_buffer[2];
	uint8_t tx_buffer[39];
	
	tx_buffer[0]=7;	                	//len
	tx_buffer[1]=0x15;		            //opcode---gendig
	tx_buffer[2]=0x02;		            //mode---config
	tx_buffer[3]=key_id;                //address_L
	tx_buffer[4]=0x00;                  //address_H
	
	sha204c_calculate_crc(5,tx_buffer, p_buffer);
	tx_buffer[5]=p_buffer[0];
	tx_buffer[6]=p_buffer[1];

	sha204p_send_command(7,tx_buffer);

	delay_ms(25);
	i = cont;
	while(sha204p_receive_response(4, response_buffer) && i)
		i--;

	if(i > 0)
		return 0;
	else 
		return -1;
}


static uint8_t sha204m_execute_write_config(uint16_t wrire_address , uint8_t *write_data,uint8_t *response_order)
{
	uint8_t i=0;
	uint8_t p_buffer[2];
	uint8_t tx_buffer[39];
	
	tx_buffer[0]=11;	                		//len
	tx_buffer[1]=0x12;		            		//opcode---write
	tx_buffer[2]=0x00;		            		//mode---config
	tx_buffer[3]=wrire_address & 0xFF;  //address_L
	tx_buffer[4]=wrire_address >> 8;    //address_H
	
	for(i=0;i<4;i++){
		tx_buffer[5+i]=write_data[i];
	}
	
	sha204c_calculate_crc(9,tx_buffer, p_buffer);
	tx_buffer[9]=p_buffer[0];
	tx_buffer[10]=p_buffer[1];

	sha204p_send_command(11,tx_buffer);

	delay_ms(25);

	while(sha204p_receive_response(4, response_order));
	
	return 0;	
}


static uint8_t sha204m_execute_write_data(uint16_t wrire_address , uint8_t *write_data,uint8_t *response_order)
{
	uint8_t i=0;
	uint8_t p_buffer[2];
	uint8_t tx_buffer[39];
	
	tx_buffer[0]=39;	                		 //len
	tx_buffer[1]=0x12;		            		//opcode---write
	tx_buffer[2]=0x82;		            		//mode---32bit-data
	tx_buffer[3]=wrire_address & 0xFF;  //address_L
	tx_buffer[4]=wrire_address >> 8;    //address_H
	
	for(i=0;i<32;i++){
		tx_buffer[5+i]=write_data[i];
	}
	
	sha204c_calculate_crc(37,tx_buffer, p_buffer);	
	tx_buffer[37]=p_buffer[0];
	tx_buffer[38]=p_buffer[1];

	sha204p_send_command(39,tx_buffer);

	delay_ms(25);

	while(sha204p_receive_response(4,response_order));
	
	return 0;
}


static uint8_t sha204m_execute_write_otp(uint16_t wrire_address , uint8_t *write_data,uint8_t *response_order)
{
	uint8_t i=0;
	uint8_t p_buffer[2];
	uint8_t tx_buffer[39];
	
	tx_buffer[0]=39;	                	//len
	tx_buffer[1]=0x12;		            //opcode---write
	tx_buffer[2]=0x81;		            //mode---OTP
	tx_buffer[3]=wrire_address & 0xFF;  //address_L
	tx_buffer[4]=wrire_address >> 8;    //address_H
	
	for(i=0;i<32;i++){
		tx_buffer[5+i]=write_data[i];
	}
	
	sha204c_calculate_crc(37,tx_buffer, p_buffer);	
	tx_buffer[37]=p_buffer[0];
	tx_buffer[38]=p_buffer[1];

	sha204p_send_command(39,tx_buffer);

	delay_ms(25);

	while(sha204p_receive_response(4, response_order));
	
	return 0;
}

static uint8_t sha204m_execute_read_otp(uint16_t read_address, uint8_t *response_order)
{
	uint8_t i=0;
	uint8_t p_buffer[2];
	uint8_t tx_buffer[7];
	
	tx_buffer[0]=7;	                	//len
	tx_buffer[1]=0x02;		            //opcode---write
	tx_buffer[2]=0x81;		            //mode---OTP
	tx_buffer[3]=read_address & 0xFF;  //address_L
	tx_buffer[4]=read_address >> 8;    //address_H

	sha204c_calculate_crc(5,tx_buffer, p_buffer);	
	tx_buffer[5]=p_buffer[0];
	tx_buffer[6]=p_buffer[1];

	#if 0
		printf("send:");
		for(i = 0; i < 7; i++)
			printf(" %02X", tx_buffer[i]);
		printf("\n");
	#endif
	
	sha204p_send_command(7,tx_buffer);

	delay_ms(25);

	i = cont;
	while(sha204p_receive_response(35, response_order) && i)
		i--;
	
	#if 0
		printf("recv:");
		for(i = 0; i < 35; i++)
			printf(" %02X", response_order[i]);
		printf("\n");
	#endif

	if(i > 0)
		return 0;
	else 
		return -1;
	
}

static uint8_t sha204m_execute_read_config_unit(uint8_t read_mode , uint16_t read_address,uint8_t *response_order)
{	
	int i;
	uint8_t p_buffer[2];
	uint8_t tx_buffer[7];
	
	tx_buffer[0]=7;	                			//len
	tx_buffer[1]=0x02;		            			//opcode---read
	tx_buffer[2]=read_mode;		    		//mode---
	tx_buffer[3]=(read_address & 0xFF);  		//address_L
	tx_buffer[4]=(read_address >> 8);    		//address_H
	
	sha204c_calculate_crc(5,tx_buffer, p_buffer);
	tx_buffer[5]=p_buffer[0];
	tx_buffer[6]=p_buffer[1];

	#if 0
	printf("send:");
	for(i = 0; i < 7; i++)
		printf(" %02X", tx_buffer[i]);
	printf("\n");
	#endif
	sha204p_send_command(7,tx_buffer);
	
	//waiting ic runing
	delay_ms(25);

	//printf("recv:");
	if(read_mode) {
		i = cont;
		while(sha204p_receive_response(35, response_order) && i)
			i--;
		#if 0
		for(i = 0; i < 35; i++)
			printf(" %02X", response_order[i]);
		printf("\n");
		#endif
	}else {
		i = cont;
		while(sha204p_receive_response(7, response_order) && i)
			i--;
		#if 0
		for(i = 0; i < 7; i++)
			printf(" %02X", response_order[i]);
		printf("\n");
		#endif
	}
	
	if(i > 0)
		return 0;
	else 
		return -1;
}


static int sha204m_execute_read_data_unit(uint8_t read_mode , uint16_t read_address, uint8_t *response_order)
{	
	int i;
	uint8_t p_buffer[2];
	uint8_t tx_buffer[39];
	
	tx_buffer[0]=7;	                    	//len
	tx_buffer[1]=0x02;		            		//opcode---read
	tx_buffer[2]=read_mode;		    	//mode---
	tx_buffer[3]=read_address & 0xFF;  //address_L
	tx_buffer[4]=read_address >> 8;    //address_H
	
	sha204c_calculate_crc(5,tx_buffer, p_buffer);
	tx_buffer[5]=p_buffer[0];
	tx_buffer[6]=p_buffer[1];
	
	#if 0
	printf("send:");
	for(i = 0; i < 7; i++)
		printf(" %02X", tx_buffer[i]);
	printf("\n");
	#endif
	
	sha204p_send_command(7,tx_buffer);

	delay_ms(25);
	i = cont;
	if(read_mode&0x80) {
		while(sha204p_receive_response(35, response_order) && i)
			i--;
	#if 0
		printf("recv:");
		for(i = 0; i < 35; i++)
			printf(" %02X", response_order[i]);
		printf("\n");
	#endif
	
	}else{           
		while(sha204p_receive_response(7, response_order) && i)
			i--;
	#if 0
		printf("recv:");
		for(i = 0; i < 7; i++)
			printf(" %02X", response_order[i]);
		printf("\n");
	#endif
	
	}
	if(i > 0)
		return 0;
	else
		return -1;
}


static int sha204m_execute_lock_config(uint8_t *response_order)
{
	uint8_t p_buffer[2];
	uint8_t tx_buffer[39];
	
	tx_buffer[0]=7;	                   		//len
	tx_buffer[1]=0x17;		            		//opcode--LOCK_CONFIG
	tx_buffer[2]=0x80;		           		//mode---
	tx_buffer[3]=0x00;                  		//address_L
	tx_buffer[4]=0x00;                  		//address_H
	
	sha204c_calculate_crc(5,tx_buffer, p_buffer);
	tx_buffer[5]=p_buffer[0];
	tx_buffer[6]=p_buffer[1];

	sha204p_send_command(7,tx_buffer);

	delay_ms(25);

   	while(sha204p_receive_response(7, response_order));
	
	return 0;
}


static int sha204m_execute_lock_data_otp(uint8_t *response_order)
{
	uint8_t p_buffer[2];
	uint8_t tx_buffer[39];
	
	tx_buffer[0]=7;	                    //len
	tx_buffer[1]=0x17;		            	//opcode--LOCK_CONFIG
	tx_buffer[2]=0x81;		            	//mode---
	tx_buffer[3]=0x00;                  	//address_L
	tx_buffer[4]=0x00;                  	//address_H
	
	sha204c_calculate_crc(5,tx_buffer, p_buffer);
	tx_buffer[5]=p_buffer[0];
	tx_buffer[6]=p_buffer[1];
	
	sha204p_send_command(7,tx_buffer);

	delay_ms(25);

    	while(sha204p_receive_response(7, response_order));
	
	return 0;

}


static uint8_t sha204m_read_config(uint8_t *response_order)
{	
	int ret;
	uint8_t buf[35];
	
	memset(buf, 0x0, sizeof(buf));
	ret |= sha204m_execute_read_config_unit(0x80, 0x0000, buf);
	memmove(response_order, buf+ 1, 32);

	memset(buf, 0x0, sizeof(buf));
	ret |= sha204m_execute_read_config_unit(0x80, 0x0008, buf);
	memmove(response_order+32, buf+ 1, 32);
	
	memset(buf, 0x0, sizeof(buf));
	ret |= sha204m_execute_read_config_unit(0x00, 0x0010, buf);
	memmove(response_order+64, buf+ 1, 4);

	memset(buf, 0x0, sizeof(buf));
	ret |= sha204m_execute_read_config_unit(0x00, 0x0011, buf);
	memmove(response_order+68, buf+ 1, 4);

	memset(buf, 0x0, sizeof(buf));
	ret |= sha204m_execute_read_config_unit(0x00, 0x0012, buf);
	memmove(response_order+72, buf+ 1, 4);

	memset(buf, 0x0, sizeof(buf));
	ret |= sha204m_execute_read_config_unit(0x00, 0x0013,buf);
	memmove(response_order+76, buf+ 1, 4);

	memset(buf, 0x0, sizeof(buf));
	ret |= sha204m_execute_read_config_unit(0x00, 0x0014, buf);
	memmove(response_order+80, buf+ 1, 4);
	
	memset(buf, 0x0, sizeof(buf));
	ret |= sha204m_execute_read_config_unit(0x00, 0x0015, buf);
	memmove(response_order+84, buf+ 1, 4);
	
	return ret;
}


static uint8_t atsha204_enc_read(uint16_t slot_to_read,  uint16_t key_id, uint8_t* secret_key, uint8_t* NumIn,uint8_t dec_data[32])
{
	static uint8_t status =0;
	uint8_t response_buffer[35];

	memset(response_buffer, 0x0, sizeof(response_buffer));
	status |= sha204m_execute_nonce(NumIn,response_buffer);
	nonce_param.mode =0x01;
	nonce_param.num_in = NumIn;
	nonce_param.rand_out = &response_buffer[1];	   //返回32位随机数
	nonce_param.temp_key = &tempkey;               //SHA256计算结果
	status |= sha204h_nonce(nonce_param);

	status |= sha204m_execute_gendig(key_id, response_buffer);
	gendig_param.zone = 0x02;
	gendig_param.key_id = key_id;
	gendig_param.stored_value =secret_key;      //密钥
	gendig_param.temp_key = &tempkey;	        //SHA256进出，结果
	status |= sha204h_gen_dig(gendig_param);

	status |=sha204m_execute_read_data_unit(0x82 , slot_to_read,response_buffer);
	
	memmove(dec_data, response_buffer+1, 32);
	dec_param.data = dec_data;
	dec_param.temp_key = &tempkey;

	status |= sha204h_decrypt(dec_param);	  //解密明文

#if 0
	printf("data:");
	for(int i = 0; i < 32; i++)
		printf("%02X ", dec_param.data[i]);
	printf("\n");
#endif

	return status;
}



/***************************************************************************************
fun		:	sha204-mac
explain	:	IN-sol_id :0-16
		   	IN-*random 32bit
		  	 OUT-*rx_buffer return mac
***************************************************************************************/
static int sha204m_execute_mac(uint8_t sol_id,uint8_t *random, uint8_t *rx_buffer)
{  
	uint8_t i=0;
	uint8_t p_buffer[2];
	uint8_t tx_buffer[39];
	
	tx_buffer[0]=39;	          	//len
	tx_buffer[1]=0x08;		  	//opcode
	tx_buffer[2]=0x00;		  	//mode
	tx_buffer[3]=sol_id;		//sol_id(0x00-0x0f)
	tx_buffer[4]=0x00;		  	//NA

	memmove(tx_buffer+5, random, 32);

	sha204c_calculate_crc(37,tx_buffer, p_buffer);
	tx_buffer[37]=p_buffer[0];	
	tx_buffer[38]=p_buffer[1];	

	sha204p_send_command(39,tx_buffer);

	//waiting 2.5ms
	delay_ms(25);
	i = cont;
	while(sha204p_receive_response(35, rx_buffer) && i){
		i--;
	}

	if(i > 0)
		return 0;
	else
		return -1;
}


int sha204_read_cfg(uint8_t *response_order)
{	
	int ret;
	uint8_t response_mac[35];
	memset(response_mac, 0x0, sizeof(response_mac));
	sha204c_wakeup(response_mac);

	ret = sha204m_read_config(response_order);

	sha204p_sleep();
	
	return ret;
}

int sha204_read_otp(uint8_t *response_order)
{	
	int ret;
	uint8_t buf[35];
	uint8_t response_mac[35];
	memset(response_mac, 0x0, sizeof(response_mac));
	sha204c_wakeup(response_mac);

	memset(buf, 0x0, 35);
	ret  |= sha204m_execute_read_otp(0x0000, buf);
	memmove(response_order, buf+1, 32);
	memset(buf, 0x0, 35);
	ret  |= sha204m_execute_read_otp(0x0008, buf);
	memmove(response_order+32, buf+1, 32);

	sha204p_sleep();
	
	return ret;
}

int sha204_read_solt(uint8_t *secret_key, uint8_t *challenge, uint8_t solt_id, uint8_t read_id, uint8_t *solt_buf)
{	
	int ret;
	uint8_t buf[35];
	uint8_t response_mac[35];

	memset(response_mac, 0x0, sizeof(response_mac));
	sha204c_wakeup(response_mac);
	
	ret = atsha204_enc_read(((read_id << 3)&0xFFFF), solt_id, secret_key, challenge, solt_buf);

	sha204p_sleep();
	
	return ret;
}



/***************************************************************************************
fun		: 	sha204-main
explain	:	IN-*secret_key ,
		   	IN-*challenge is 32bit random
		   	IN-solt_id 
		   	return : 1 succ, 0 fail
***************************************************************************************/
int sha204_main_handle(uint8_t *secret_key, uint8_t *challenge, uint8_t solt_id, int mode)
{	
	uint8_t config_read_buf[88];
	uint8_t response_mac[35];
	uint8_t sha256_result[32];
	uint8_t random_buf[35];
	uint8_t challenge_tmp[32];
	uint8_t i=0,result=0, ret;

	memset(response_mac, 0x0, sizeof(response_mac));
	memset(sha256_result, 0x0, sizeof(sha256_result));
	memset(random_buf, 0x0, sizeof(response_mac));
	memset(challenge_tmp, 0x0, sizeof(challenge_tmp));

	//printf("wake up IC\n");
	sha204c_wakeup(response_mac);

	if(mode){
		//printf("get random\n");
		ret |= sha204m_execute_random(random_buf);

		//printf("run nonce\n");
		ret |= sha204m_execute_nonce(random_buf+1,response_mac);

		memmove(challenge_tmp, response_mac+1, 32);
		memset(response_mac, 0x0, sizeof(response_mac));
	}else{
		memmove(challenge_tmp, challenge, 32);
		memset(response_mac, 0x0, sizeof(response_mac));
	}

	//printf("calculate mac\n");
	ret |= sha204_calculate_mac(secret_key,challenge_tmp,solt_id,sha256_result);  

	//printf("run mac\n");
	ret |= sha204m_execute_mac(solt_id,challenge_tmp,response_mac);
	
	//printf("sleep IC\n");
	sha204p_sleep();

	while(strncmp(sha256_result, response_mac+1, 32) == 0){
		return 1 ;  	//ok
	}

#if 0
		printf("solt_id:%d, mode:%d\n", solt_id, mode);
		printf("key:");
		for(i=0;i<32;i++) {
			printf(" %02X", secret_key[i]);
		}
		printf("\nrandom:");
		for(i=0;i<32;i++) {
			printf(" %02X", challenge_tmp[i]);
		}
		printf("\nmac:");
		for(i=0;i<32;i++) {
			printf(" %02X", sha256_result[i]);
		}
		printf("\nresult:");
		for(i=1;i<= 32; i++) {
			printf(" %02X", response_mac[i]);
		}
		printf("\n");
#endif

	return 0;		//fail	
}


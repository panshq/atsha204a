#ifndef _SHA204A_CORE_H_
#define _SHA204A_CORE_H_

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;


/***************************结构体申明*************************************/
struct sha204h_temp_key
{
	uint8_t value[32];
	uint8_t key_id :4;
	uint8_t source_flag :1;
	uint8_t gen_data :1;
	uint8_t check_flag :1;
	uint8_t valid :1;
};

struct sha204h_nonce_in_out
{
	uint8_t mode;
	uint8_t *num_in;
	uint8_t *rand_out;
	struct sha204h_temp_key *temp_key;
};

struct sha204h_gen_dig_in_out
{
	uint8_t zone;
	uint16_t key_id;
	uint8_t *stored_value;
	struct sha204h_temp_key *temp_key;
};

struct sha204h_decrypt_in_out
{
	uint8_t *data;
	struct sha204h_temp_key *temp_key;
};

struct sha204h_nonce_in_out nonce_param;		//!< Parameter for nonce helper function
struct sha204h_gen_dig_in_out gendig_param;	//!< Parameter for gendig helper function
struct sha204h_temp_key tempkey;				//!< Tempkey parameter for nonce and mac helper function
struct sha204h_decrypt_in_out dec_param;		//!< Parameter for decrypt helper function

int sha204_read_cfg(uint8_t *response_order);
int sha204_read_otp(uint8_t *response_order);
int sha204_read_solt(uint8_t *secret_key, uint8_t *challenge, uint8_t solt_id, uint8_t read_id, uint8_t *solt_buf);
int sha204_main_handle(uint8_t *secret_key, uint8_t *challenge, uint8_t solt_id, int mode);

#endif

#ifndef _CRYPTO_H_
#define _CRYPTO_H_

void authentication_on();
void authentication_off();
void get_cfg_data(unsigned char *buf);
void get_otp_data(unsigned char *buf);
void get_solt_data(unsigned char *secret_key, unsigned char *challenge, unsigned char solt_id, unsigned char read_solt_id,unsigned char* buf);
int authentication_main(unsigned char *secret_key, unsigned char *challenge, unsigned char solt_id, int mode);

#endif
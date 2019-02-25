#ifndef _CRYPTO_H_
#define _CRYPTO_H_

void authentication_on();
void authentication_off();
int authentication_main(unsigned char *secret_key, unsigned char *challenge, unsigned char solt_id, int mode);

#endif
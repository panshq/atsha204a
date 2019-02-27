#ifndef _QST_AUTHEN_H_
#define _QST_AUTHEN_H_

typedef struct ic_info{
	char id[21];
	char time[11];
	char version[15];
}IC_INFO;

typedef struct option{
	unsigned int master_opt;
	unsigned int minor_opt;	
}IC_OPT;

/*Function: enable IC*/
void qst_authen_on();
/*Function: sleep IC*/
void qst_authen_off();
/*Function: get version information from chip*/
int qst_authen_get_version(IC_INFO *info);
/*Function: get function selection information from chip*/
int qst_authen_get_option(IC_OPT *opt);
/*Function: according to the input parameters to verify*/
int qst_authen_verify(unsigned char *secret_key, unsigned char *challenge, unsigned char solt_id, int mode);

#endif
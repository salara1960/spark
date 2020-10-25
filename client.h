#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "functions.h"

#ifdef SET_CLI_OFD


#define max_url_len 256
#define def_srv_url "f1test.taxcom.ru:7778"
//#define def_srv_url "193.0.214.11:7778"
//#define def_srv_url "localhost:9192"


//---------------------------------------------------------------------------------------------------

typedef struct {
    char *url;
    uint8_t *data;
    int datalen;
} s_ses;

//---------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------

extern void closeTCP(int *esoc);
extern int openTCP(const char *eurl);
extern void *cli_nitka(void *arg);

#endif

#endif
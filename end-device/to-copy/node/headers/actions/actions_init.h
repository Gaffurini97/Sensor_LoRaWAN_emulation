#ifndef ACTIONS_INIT_H
#define ACTIONS_INIT_H

#include "actions_common.h"
#include <time.h>
#include <stdlib.h>
#include <limits.h>

#define APP_S_KEY_LEN 16
#define NWK_S_KEY_LEN 16


typedef struct JoinAcceptData_t{
    LoRaMacMessageJoinAccept_t *msg;
    uint8_t *nwkSKey; 
    uint8_t *appSKey;
    bool is_ok;
} JoinAcceptData_t;

/* -----------JOIN REQUEST--------------- */
LoRaMacMessageJoinRequest_t *generateJoinRequest(char*,char*,char*,uint16_t);
void free_join_request(LoRaMacMessageJoinRequest_t *);

/* -----------JOIN ACCEPT----------- */
JoinAcceptData_t *generateJoinAccept(char*,uint16_t, uint8_t*,uint8_t);
bool IsDataOk(JoinAcceptData_t*);
void free_join_accept(JoinAcceptData_t*);

/* -----------UTILITY----------- */
uint16_t generateNonce();

#endif
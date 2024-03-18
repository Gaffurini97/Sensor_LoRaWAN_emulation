#ifndef LORA_NODE_COMMUNICATION_H
#define LORA_NODE_COMMUNICATION_H

#include <stdio.h>
#include "commons/node_gw_common.h"
#include "commons/packet_parsings.h"
#include "actions/actions_init.h"
#include "actions/actions.h"
#include "../loramac/utilities.h"
#include "enmqtt.h"
#include "commons/parson.h"
#include <sys/time.h>

typedef struct node_info{
    int communication_type; /* FROM FILE */
    char *node_name; /* FROM FILE*/
    char *devEUI_c; /* FROM FILE*/
    char *joinEUI_c; /* FROM FILE*/
    char *appKey_c; /* FROM FILE*/
    uint16_t lastDevNonce;/* FROM FILE*/
    uint8_t *appSKey;
    uint8_t *nwkSKey;
    uint32_t devAddr;
    uint32_t sequenceCounter;
    JoinAcceptData_t *j_accept_data;
} LoRaNodeInfo;

static pthread_cond_t quit_cond = PTHREAD_COND_INITIALIZER;

void set_connected();
JoinAcceptData_t *send_join_request_await_accept(uint8_t*, uint8_t, LoRaNodeInfo *);
bool send_uplink_no_ack(uint8_t*,uint8_t);
struct lgw_pkt_rx_s generate_rx_packet(uint8_t*,uint8_t);

char *concatStrAndDevAddr(char* st,uint32_t dev_addr);
char *concatStrAndDevAddrOpt(char* st,uint32_t dev_addr,bool st_before_address);
char *concatStrs(char* st,char* st2);


bool send_broadcast_alert(char* str);
bool send_is_ready_alert();
bool send_info_message(char* message);
#endif
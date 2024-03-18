#ifndef GATEWAY_STACKS_H
#define GATEWAY_STACKS_H

#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "commons/node_gw_common.h"

#define PKT_STACK_SIZE      256
#define TX_METADATA_NB      16
#define RX_METADATA_NB      16

typedef enum{
    STACK_FULL = 1,
    STACK_EMPTY = -1,
    STACK_OK = 0
} StackStatus;

static struct lgw_pkt_rx_s *stack_r;
static int rsp_r; //Received stack pointer
static int sp_r; //Stack pointer

static pthread_mutex_t mtx_r = PTHREAD_MUTEX_INITIALIZER;

int initialize_stacks();
int get_remaining_slots_read(int sp, int rsp, int size);
int get_remaining_slots_push(int sp, int rsp, int size);
StackStatus push_r(struct lgw_pkt_rx_s);
StackStatus pop_r(struct lgw_pkt_rx_s *);

#endif
#ifndef GATEWAY_COMMUNICATION_H
#define GATEWAY_COMMUNICATION_H

#include "gateway_stacks.h"
#include "gwmqtt.h"
#include "commons/packet_parsings.h"

#define BUFFER_OUT_SIZE 1024
#define BUFFER_IN_SIZE 1024

int lgw_start(void);
int lgw_stop(void);
int lgw_send(struct lgw_pkt_tx_s);
int lgw_receive(uint8_t, struct lgw_pkt_rx_s *);

#endif
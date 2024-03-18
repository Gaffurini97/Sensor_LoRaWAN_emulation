#ifndef PACKET_PARSINGS_H
#define PACKET_PARSINGS_H
#include "node_gw_common.h"
#include "parson.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "base64.h"

char* ngw_rx_to_json(struct lgw_pkt_rx_s *obj);
struct lgw_pkt_rx_s *ngw_json_to_rx(char* json);

char* ngw_tx_to_json(struct lgw_pkt_tx_s *obj);
struct lgw_pkt_tx_s *ngw_json_to_tx(char* json);
#endif
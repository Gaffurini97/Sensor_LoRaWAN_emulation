#ifndef LORA_NODE_H
#define LORA_NODE_H

#include "actions/actions_init.h"
#include "actions/actions.h"
#include "commons/parson.h"
#include "lora_node_communication.h"

#define DEV_EUI_C "devEUI"
#define JOIN_EUI_C "joinEUI"
#define APP_KEY_C "app_key"
#define NODE_NAME_C "node_name"
#define LAST_DEV_NONCE_C "lastdevNonce"
#define RETRY_JOIN_REQUEST_COUNT_C "retry_joinRequest_count"

#define CONFIGURATION_FILE_PATH "/cfg-data/config.json"

bool executeJoin(LoRaNodeInfo*);
bool save_last_devNonce(const char*,uint16_t);
#endif
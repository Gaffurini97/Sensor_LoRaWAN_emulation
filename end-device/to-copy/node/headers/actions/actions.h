#ifndef ACTIONS_H
#define ACTIONS_H

#include "actions_common.h"

typedef struct response{
    uint8_t *payload_decrypted;
    char* payload_hex;
    LoRaMacMessageData_t *dwLink;
} Response;

Response *deserialize_downlink(uint8_t*,uint8_t*,uint8_t*,uint8_t);
LoRaMacMessageData_t *serialize_uplink(uint32_t,uint32_t,uint8_t*,uint8_t*,uint8_t*,uint8_t);

LoRaMacMessageData_t *serialize_hex_uplink(uint32_t devAddr, uint32_t sequenceCounter,
            uint8_t *nwkSKey,uint8_t *appSKey,
            char *hex_frame, uint8_t frame_size);
#endif
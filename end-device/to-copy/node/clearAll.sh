#!/bin/sh
rm loramac/LoRaMacCrypto.o
rm loramac/LoRaMacParser.o
rm loramac/LoRaMacSerializer.o
rm loramac/utilities.o

rm loramac/crypto/aes.o
rm loramac/crypto/cmac.o

rm src/lora_node_communication.o
rm src/lora_node.o
rm src/enmqtt.o
rm src/initializer.o

rm src/commons/base64.o
rm src/commons/hash_map_manager.o
rm src/commons/parson.o
rm src/commons/packet_parsings.o

rm src/routes/routes_manag.o

rm src/actions/actions_init.o
rm src/actions/actions.o

rm execnode
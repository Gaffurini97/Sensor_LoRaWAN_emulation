#!/bin/sh
rm loramac/LoRaMacCrypto.o
rm loramac/LoRaMacParser.o
rm loramac/LoRaMacSerializer.o
rm loramac/utilities.o

rm loramac/crypto/aes.o
rm loramac/crypto/cmac.o

rm src/basic_pkt_fwd.o
rm src/gateway_communication.o
rm src/gateway_stacks.o
rm src/gwmqtt.o

rm src/commons/base64.o
rm src/commons/hash_map_manager.o
rm src/commons/parson.o
rm src/commons/packet_parsings.o

rm src/routes/routes_manag.o

rm execgateway

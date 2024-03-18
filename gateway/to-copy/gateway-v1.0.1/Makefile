compile_all:
	gcc -c loramac/LoRaMacCrypto.c -o loramac/LoRaMacCrypto.o
	gcc -c loramac/LoRaMacParser.c -o loramac/LoRaMacParser.o
	gcc -c loramac/LoRaMacSerializer.c -o loramac/LoRaMacSerializer.o
	gcc -c loramac/utilities.c -o loramac/utilities.o

	gcc -c loramac/crypto/aes.c -o loramac/crypto/aes.o
	gcc -c loramac/crypto/cmac.c -o loramac/crypto/cmac.o

	gcc -c -Iheaders src/basic_pkt_fwd.c -o src/basic_pkt_fwd.o
	gcc -c -Iheaders src/gateway_communication.c -o src/gateway_communication.o
	gcc -c -Iheaders src/gateway_stacks.c -o src/gateway_stacks.o
	gcc -c -Iheaders src/gwmqtt.c -o src/gwmqtt.o

	gcc -c -Iheaders/commons src/commons/base64.c -o src/commons/base64.o
	gcc -c -Iheaders/commons src/commons/hash_map_manager.c -o src/commons/hash_map_manager.o
	gcc -c -Iheaders/commons src/commons/parson.c -o src/commons/parson.o
	gcc -c -Iheaders/commons src/commons/packet_parsings.c -o src/commons/packet_parsings.o

	gcc -c -Iheaders/routes src/routes/routes_manag.c -o src/routes/routes_manag.o

	gcc src/routes/routes_manag.o src/commons/hash_map_manager.o src/gwmqtt.o src/basic_pkt_fwd.o src/gateway_communication.o src/gateway_stacks.o src/commons/parson.o src/commons/base64.o src/commons/packet_parsings.o loramac/LoRaMacCrypto.o loramac/LoRaMacParser.o loramac/LoRaMacSerializer.o loramac/utilities.o loramac/crypto/aes.o loramac/crypto/cmac.o -lpthread -lmosquitto -o execgateway

compile_all:
	gcc -c loramac/LoRaMacCrypto.c -o loramac/LoRaMacCrypto.o
	gcc -c loramac/LoRaMacParser.c -o loramac/LoRaMacParser.o
	gcc -c loramac/LoRaMacSerializer.c -o loramac/LoRaMacSerializer.o
	gcc -c loramac/utilities.c -o loramac/utilities.o

	gcc -c loramac/crypto/aes.c -o loramac/crypto/aes.o
	gcc -c loramac/crypto/cmac.c -o loramac/crypto/cmac.o

	gcc -c -Iheaders src/lora_node_communication.c -o src/lora_node_communication.o
	gcc -c -Iheaders src/lora_node.c -o src/lora_node.o
	gcc -c -Iheaders src/enmqtt.c -o src/enmqtt.o
	gcc -c -Iheaders src/initializer.c -o src/initializer.o

	gcc -c -Iheaders/commons src/commons/base64.c -o src/commons/base64.o
	gcc -c -Iheaders/commons src/commons/hash_map_manager.c -o src/commons/hash_map_manager.o
	gcc -c -Iheaders/commons src/commons/parson.c -o src/commons/parson.o
	gcc -c -Iheaders/commons src/commons/packet_parsings.c -o src/commons/packet_parsings.o

	gcc -c -Iheaders/routes src/routes/routes_manag.c -o src/routes/routes_manag.o

	gcc -c -Iheaders/actions src/actions/actions_init.c -o src/actions/actions_init.o
	gcc -c -Iheaders/actions src/actions/actions.c -o src/actions/actions.o

	gcc src/initializer.o src/enmqtt.o src/lora_node.o ./src/lora_node_communication.o ./src/routes/routes_manag.o ./src/commons/parson.o ./src/commons/hash_map_manager.o ./src/commons/base64.o ./src/commons/packet_parsings.o ./src/actions/actions_init.o ./src/actions/actions.o ./loramac/LoRaMacCrypto.o ./loramac/LoRaMacParser.o ./loramac/LoRaMacSerializer.o ./loramac/utilities.o ./loramac/crypto/aes.o ./loramac/crypto/cmac.o -lpthread -lmosquitto -o execnode

#include "../../headers/actions/actions_init.h"

/*
                            OTAA PROCEDURE v1.0.x
*/

bool is_rand_initialized = false;

/* -----------JOIN REQUEST--------------- */

//LORAWAN-1.0.4 spec row 1383//
LoRaMacMessageJoinRequest_t *generateJoinRequest(char *devEUI_c, char *joinEUI_c,
                                                char *appKey_c, uint16_t noncee){
    uint16_t devNonce = noncee;
    
    //devEUI	
	uint8_t devEUI[LORAMAC_DEV_EUI_FIELD_SIZE];
	hex2bin(devEUI_c,16,devEUI);
	
	//appEUI
	uint8_t joinEUI[LORAMAC_JOIN_EUI_FIELD_SIZE];
	hex2bin(joinEUI_c,16,joinEUI);
	
	//appkey
	uint8_t appKey[16];	
	hex2bin(appKey_c,32,appKey);
	
	//struttura	
	LoRaMacMessageJoinRequest_t *join_request = malloc(sizeof(LoRaMacMessageJoinRequest_t));
    join_request->Buffer = malloc(sizeof(uint8_t)*LORAMAC_JOIN_REQ_MSG_SIZE);
    join_request->BufSize = LORAMAC_JOIN_REQ_MSG_SIZE; 
    join_request->MHDR.Value = 0;
    memcpy1(join_request->JoinEUI,joinEUI,LORAMAC_JOIN_EUI_FIELD_SIZE);
    memcpy1(join_request->DevEUI,devEUI,LORAMAC_DEV_EUI_FIELD_SIZE);
	       
    join_request->DevNonce = devNonce;
    
    //serializzo        
    LoRaMacSerializerJoinRequest(join_request);       
    //mic calc e lo aggiungo
    LoRaMacJoinComputeMic(join_request->Buffer, ( LORAMAC_JOIN_REQ_MSG_SIZE - LORAMAC_MIC_FIELD_SIZE ),
        appKey, &join_request->MIC);
    //riserializzo
    LoRaMacSerializerJoinRequest(join_request);
	
    printf("Join-Request Buffer\n");
    for (int i = 0; i < LORAMAC_JOIN_REQ_MSG_SIZE; i++) {
		printf("%02X", join_request->Buffer[i]);
    }
    printf("\n");

    return join_request;
}

void free_join_request(LoRaMacMessageJoinRequest_t *join_request){
    free(join_request->Buffer);
    free(join_request);
}

/* -----------JOIN ACCEPT----------- */

JoinAcceptData_t *generateJoinAccept(char *appKey_c, uint16_t devNonce,
                                    uint8_t *frame, uint8_t size){
    //Inizializzo il dato da ritornare
	JoinAcceptData_t * data = malloc(sizeof(JoinAcceptData_t));
    data->is_ok = false;
    data->appSKey = malloc(sizeof(uint8_t)*APP_S_KEY_LEN);
    data->nwkSKey = malloc(sizeof(uint8_t)*NWK_S_KEY_LEN);
	uint32_t devAddr;
        	
	//appkey
	uint8_t appKey[16];	
	hex2bin(appKey_c,32,appKey);
	
	uint8_t decBuffer[size];
    LoRaMacMessageJoinAccept_t *join_accept = malloc(sizeof(LoRaMacMessageJoinAccept_t)); //struttura dati del pacchetto join accept
    //data->msg lo punto al dato sopra
    data->msg = join_accept;
    decBuffer[0] = frame[0]; //salvo l'mhdr
    LoRaMacJoinDecrypt(&frame[1], size-1, appKey, &decBuffer[1]); //decripto 
	    
    //riempio la struttura
    join_accept->Buffer = decBuffer;           
    join_accept->BufSize = size;
    LoRaMacParserJoinAccept(join_accept);
    //verifico il mic
    uint32_t frame_mic;
    LoRaMacJoinComputeMic(join_accept->Buffer, size - LORAMAC_MIC_FIELD_SIZE, appKey, &frame_mic);
    
    if(frame_mic == join_accept->MIC) {
        //creo le chiavi sessione
        uint8_t appNonce[6];
        for (int i = 0; i < 3;i++){
            appNonce[i] = join_accept->JoinNonce[i];
        }
        for (int i = 3; i < 6;i++){
            appNonce[i] = join_accept->NetID[i-3];
        }
        LoRaMacJoinComputeSKeys(appKey, appNonce, devNonce , data->nwkSKey, data->appSKey );
        
        //estraggo il devAddress
        devAddr = join_accept->DevAddr;
        printf("%02X",devAddr);
        data->is_ok = true;

        for(int r=0;r<16;r++){
            printf("%02X", data->nwkSKey[r]);
        }
        for(int r=0;r<16;r++){
            printf("%02X", data->appSKey[r]);
        }
    }
    return data;
}

bool IsDataOk(JoinAcceptData_t *data){
    return data->is_ok;
}

void free_join_accept(JoinAcceptData_t *data){
    free(data->appSKey);
    free(data->nwkSKey);
    free(data->msg);
    free(data);
}

/* -----------UTILITY----------- */
uint16_t generateNonce(){
    if(!is_rand_initialized){
        srand((unsigned)time(NULL));
        is_rand_initialized = true;
    }
    return (uint16_t)(rand() % USHRT_MAX);
}


/* TODO OTAA PROCEDURE v1.1.x */

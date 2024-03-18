#include "../../headers/actions/actions.h"

Response *deserialize_downlink(uint8_t *nwkSKey,uint8_t *appSKey, uint8_t *frame, uint8_t size){
    Response *resp = malloc(sizeof(Response));

    resp->dwLink = malloc(sizeof(LoRaMacMessageData_t));//struttura dati del pacchetto downlink

    uint8_t frmPayload_buff[size - LORAMAC_FRAME_PAYLOAD_MIN_SIZE];
    
    //riempio la struttura
    resp->dwLink->Buffer = frame;           
    resp->dwLink->BufSize = size;
    resp->dwLink->FRMPayload = frmPayload_buff;
    LoRaMacParserData(resp->dwLink);
    
    //verifico il mic
    uint32_t frame_mic;
    LoRaMacComputeMic(resp->dwLink->Buffer, size - LORAMAC_MIC_FIELD_SIZE,
            nwkSKey, resp->dwLink->FHDR.DevAddr, 0x01, resp->dwLink->FHDR.FCnt, &frame_mic);
    
    
    //decripto il payload
    if(frame_mic == resp->dwLink->MIC) {
        resp->payload_decrypted = malloc(resp->dwLink->FRMPayloadSize);
        
        LoRaMacPayloadDecrypt(resp->dwLink->FRMPayload, resp->dwLink->FRMPayloadSize,
            appSKey, resp->dwLink->FHDR.DevAddr, 0x01 ,resp->dwLink->FHDR.FCnt, resp->payload_decrypted);

        //populate hex
        resp->payload_hex = bin2hex(resp->payload_decrypted,resp->dwLink->FRMPayloadSize);
        
        //Okke return response
        return resp;
    } else {
        return NULL;
    }
}

LoRaMacMessageData_t *serialize_uplink(uint32_t devAddr, uint32_t sequenceCounter,
            uint8_t *nwkSKey,uint8_t *appSKey,
            uint8_t *payload, uint8_t payload_size){
    LoRaMacMessageData_t *upLink = malloc(sizeof(LoRaMacMessageData_t));
    uint8_t buffer[LORAMAC_FRAME_PAYLOAD_MIN_SIZE + payload_size + LORAMAC_F_PORT_FIELD_SIZE ];
    uint8_t payload_buffer[payload_size];
    upLink->Buffer = buffer;
    upLink->BufSize = LORAMAC_FRAME_PAYLOAD_MIN_SIZE + payload_size+ LORAMAC_F_PORT_FIELD_SIZE ;
    upLink->FRMPayload = payload_buffer;

    //MHDR
    upLink->MHDR.Value = 0x00;  
    upLink->MHDR.Bits.MType = 2;     //unconfirmed data up
    //FHDR       
    upLink->FHDR.DevAddr = devAddr;
    upLink->FHDR.FCtrl.Value = 0x00; //
    upLink->FHDR.FCnt = (uint16_t)(sequenceCounter & 0x0000FFFF);

    // fport 
    upLink->FPort = 0x01; //lo pongo pari a 1 , non presente nei parametri 
    
    //cripto il frmpayload e lo aggiungo nella struttura assieme al size
    LoRaMacPayloadEncrypt( payload, payload_size , appSKey, devAddr, 0x00, sequenceCounter, upLink->FRMPayload );
    upLink->FRMPayloadSize = payload_size;
    
    //serializzo
    int t = LoRaMacSerializerData(upLink);
    //calcolo il mic
    LoRaMacComputeMic(upLink->Buffer, (LORAMAC_FRAME_PAYLOAD_MIN_SIZE + payload_size + LORAMAC_F_PORT_FIELD_SIZE  - LORAMAC_MIC_FIELD_SIZE), nwkSKey, devAddr,0x00,sequenceCounter, &upLink->MIC);
    
    //riserializzo
    int f = LoRaMacSerializerData(upLink);
           
    for (int r = 0; r < upLink->BufSize; r++) {
        printf("%02X", upLink->Buffer[r]);
    }
    
    return upLink;
}

LoRaMacMessageData_t *serialize_hex_uplink(uint32_t devAddr, uint32_t sequenceCounter,
            uint8_t *nwkSKey,uint8_t *appSKey,
            char *hex_frame, uint8_t frame_size){
    uint8_t *frame = malloc(frame_size);//Sono bytes quindi va bene così
	hex2bin(hex_frame,frame_size*2,frame);
    return serialize_uplink(devAddr, sequenceCounter,nwkSKey,appSKey,frame,frame_size);
}
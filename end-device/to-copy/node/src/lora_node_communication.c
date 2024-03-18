#include "../headers/lora_node_communication.h"

LoRaNodeInfo *node_ref;

pthread_mutex_t connected_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t connected_cond = PTHREAD_COND_INITIALIZER;
bool lorawan_connected = false;

pthread_mutex_t msg_received_mutex = PTHREAD_MUTEX_INITIALIZER;
JoinAcceptData_t *obj = NULL;


pthread_cond_t join_accept_arrived_cond = PTHREAD_COND_INITIALIZER;


void send_measures(struct timeval *msg_arrived, char* category){
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_dotset_string(root_object, "msg.type", "MEASURES");
    json_object_dotset_string(root_object, "msg.ref.type", "EN");
    json_object_dotset_string(root_object, "msg.ref.id", node_ref->devEUI_c);
    json_object_dotset_string(root_object, "msg.category", category);
    json_object_dotset_number(root_object, "msg.timestamp.value", msg_arrived->tv_sec);
    json_object_dotset_number(root_object, "msg.timestamp.us", msg_arrived->tv_usec);
    char* serialized_string = json_serialize_to_string(root_value);
    enmqtt_publish(serialized_string,"/measures", 0);
    json_free_serialized_string(serialized_string);
    json_value_free(root_value);
}

void set_connected(){
    //without lock for avoiding loop
    lorawan_connected = true;
    pthread_cond_signal(&connected_cond);
}

void free_message(char* topic, char* payload){
    if(topic != NULL)free(topic);
    if(payload != NULL)free(payload);
}

/* topic and payload are already copied*/
void on_message_received(char* topic, void* payload, int length){
    //Prendo l'istante in cui è arrivato il messaggio
    struct timeval msg_arrived;
    gettimeofday(&msg_arrived, NULL);
    pthread_mutex_lock(&msg_received_mutex);
    char* temp_topic;char* final_topic;
    printf("%s \n",topic);fflush(stdout);
    if(!lorawan_connected){
        temp_topic = routes_getTopicFromAction(ACTION_RADIO_RECEIVE);
        final_topic = concatStrs(temp_topic, node_ref->devEUI_c);

        //obj == NULL means radio wanna join acept
        if(strcmp(topic, final_topic) == 0 && obj == NULL){
            printf("FOUND DOWNLINK RADIO FOR JOIN\n");fflush(stdout);
            if(final_topic!=NULL)free(final_topic);
            char* payload_ch = (char*)payload;
            uint8_t payload[256];
            int t1size = b64_to_bin(payload_ch, strlen(payload_ch), payload, sizeof payload);

            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object = json_value_get_object(root_value);
            json_object_dotset_string(root_object, "msg.type", "INFO");
            json_object_dotset_string(root_object, "msg.ref.type", "EN");
            json_object_dotset_string(root_object, "msg.category", "join-accept_received");
            json_object_dotset_number(root_object, "msg.timestamp.value", msg_arrived.tv_sec);
            json_object_dotset_number(root_object, "msg.timestamp.us", msg_arrived.tv_usec);
            char* serialized_string = json_serialize_to_string(root_value);
            enmqtt_publish(serialized_string,"/DbALT", 0);
            json_free_serialized_string(serialized_string);
            json_value_free(root_value);

            JoinAcceptData_t *jacpt_d = generateJoinAccept(node_ref->appKey_c,
                node_ref->lastDevNonce, payload, t1size);
            //Uso lo stesso dev nonce di prima senza modificarlo per creare il join accept
            if(IsDataOk(jacpt_d)){
                obj = jacpt_d;
                pthread_cond_signal(&join_accept_arrived_cond);
            }
            pthread_mutex_unlock(&msg_received_mutex);
            return;
        }

        char* action = routes_getTopicFromAction(topic);
        if(action == NULL){
            printf("ERROR\n");
            return;
        }
        if(strcmp(action, ACTION_JOIN_ACCEPT) == 0 && obj == NULL) {
            printf("FOUND JOIN ACCEPT\n");fflush(stdout);
            printf("Json: %s\n", (char*)payload);fflush(stdout);
            /*Join Accept è da gw a end node quindi sarà in json il contenuto.*/
            struct lgw_pkt_tx_s* temp_obj = ngw_json_to_tx((char*)payload);

            
            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object = json_value_get_object(root_value);
            json_object_dotset_string(root_object, "msg.type", "INFO");
            json_object_dotset_string(root_object, "msg.ref.type", "EN");
            json_object_dotset_string(root_object, "msg.category", "join-accept_received");
            json_object_dotset_number(root_object, "msg.timestamp.value", msg_arrived.tv_sec);
            json_object_dotset_number(root_object, "msg.timestamp.us", msg_arrived.tv_usec);
            char* serialized_string = json_serialize_to_string(root_value);
            enmqtt_publish(serialized_string,"/DbALT", 0);
            json_free_serialized_string(serialized_string);
            json_value_free(root_value);

            /*Successivamente devo prendere il payload*/
            int payload_size = sizeof(temp_obj->payload)/sizeof(uint8_t);
            uint8_t *cpy_payload = malloc(payload_size);
            memcpy(cpy_payload, temp_obj->payload, payload_size);
            int tempobj_size = temp_obj->size;
            //Passo il payload copiato
            JoinAcceptData_t *jacpt_d = generateJoinAccept(node_ref->appKey_c,
                node_ref->lastDevNonce, cpy_payload, tempobj_size);
            //Uso lo stesso dev nonce di prima senza modificarlo per creare il join accept
            //Libero le risorse del temp_obj
            if(temp_obj != NULL)
                free(temp_obj);
            if(IsDataOk(jacpt_d)){
                obj = jacpt_d;
                pthread_cond_signal(&join_accept_arrived_cond);
            }
            pthread_mutex_unlock(&msg_received_mutex);
            return;
        }
        //Else ignore
        free_message(topic, payload);
    }else{
        //Altrimenti FORWARDING OR COLLECTING
        temp_topic = routes_getTopicFromAction(ACTION_RADIO_RECEIVE);
        final_topic = concatStrs(temp_topic, node_ref->devEUI_c);
        //obj == NULL means radio wanna join acept
        if(strcmp(topic, final_topic) == 0){
            printf("FOUND DOWNLINK RADIO\n");fflush(stdout);
            if(final_topic!=NULL)free(final_topic);
            char* payload_ch = (char*)payload;
            printf("Payload char is %s\n",payload_ch);
            temp_topic = routes_getTopicFromAction(DISSEMINATOR_ACTION_DOWNLINK_NO_ACK);
            final_topic = concatStrs(temp_topic, node_ref->devEUI_c);
            if(strcmp(payload_ch,"no_downlink")==0){
                send_measures(&msg_arrived,"ENR_d_ok_nodata");
                struct timeval tstmp_downlink_send;
                gettimeofday(&tstmp_downlink_send, NULL);
                enmqtt_publish("ok", final_topic, 0);//Uplink sent - NO downlink on queue
                enmqtt_publish("ok,no-downlink\n","/DbALT", 0);
                send_measures(&tstmp_downlink_send,"ENR_d_beforesend");
                printf("published OK - no downlink\n");fflush(stdout);
                free_message(topic, payload);
                if(final_topic != NULL)free(final_topic);
                pthread_mutex_unlock(&msg_received_mutex);
                return;
            }else if(strcmp(payload_ch,"error")==0){
                send_measures(&msg_arrived,"ENR_d_error");
                struct timeval tstmp_downlink_send;
                gettimeofday(&tstmp_downlink_send, NULL);
                enmqtt_publish("error", final_topic, 0);//Uplink sent - NO downlink on queue
                enmqtt_publish("error,no-downlink\n","/DbALT", 0);
                send_measures(&tstmp_downlink_send,"ENR_d_beforesend");
                printf("published ERROR - no downlink\n");fflush(stdout);
                free_message(topic, payload);
                if(final_topic != NULL)free(final_topic);
                pthread_mutex_unlock(&msg_received_mutex);
                return;
            }
            printf("%s payload\n", payload_ch);fflush(stdout);
            uint8_t payload[256];
            int t1size = b64_to_bin(payload_ch, strlen(payload_ch), payload, sizeof payload);
            Response *response = deserialize_downlink(
                node_ref->nwkSKey,node_ref->appSKey,
                payload, t1size);
            if(response == NULL){
                send_measures(&msg_arrived,"ENR_d_error");
                struct timeval tstmp_downlink_send;
                gettimeofday(&tstmp_downlink_send, NULL);
                enmqtt_publish("error", final_topic, 0);//Uplink sent - NO downlink on queue
                enmqtt_publish("error,no-downlink\n","/DbALT", 0);
                send_measures(&tstmp_downlink_send,"ENR_d_beforesend");
                printf("published ERROR - no downlink\n");fflush(stdout);
                //free_message(topic, payload);
                if(final_topic != NULL)free(final_topic);
                pthread_mutex_unlock(&msg_received_mutex);
                return;
            }
            send_measures(&msg_arrived,"ENR_d_ok_data");
            //Ed infine forwarding
            temp_topic = routes_getTopicFromAction(DISSEMINATOR_ACTION_DOWNLINK_NO_ACK);
            final_topic = concatStrs(temp_topic, node_ref->devEUI_c);
            /* Il payload in uscita è esadecimale !*/

            struct timeval tstmp_downlink_send;
            gettimeofday(&tstmp_downlink_send, NULL);

            enmqtt_publish(response->payload_hex, final_topic, 0);
            enmqtt_publish(response->payload_hex,"/DbALT", 0);

            send_measures(&tstmp_downlink_send,"ENR_d_beforesend");

            //Free resources
            if(response->payload_hex != NULL)free(response->payload_hex);
            if(response->payload_decrypted != NULL)free(response->payload_decrypted);
            if(response->dwLink != NULL)free(response->dwLink);
            if(response != NULL)free(response);
            if(topic != NULL)free(topic);
            if(final_topic != NULL)free(final_topic);
            //free_message(topic, payload);
            //---
            pthread_mutex_unlock(&msg_received_mutex);
            return;
        }

        final_topic = routes_getTopicFromAction(ACTION_DOWNLINK_NO_ACK);
        if(strcmp(topic, final_topic) == 0){
            send_measures(&msg_arrived,"EN_d_received");
            printf("FOUND DOWNLINK\n");fflush(stdout);
            //Se downlink specifico, costruisco il topic
            temp_topic = routes_getTopicFromAction(DISSEMINATOR_ACTION_DOWNLINK_NO_ACK);
            final_topic = concatStrs(temp_topic, node_ref->devEUI_c);
            printf("Json: %s\n", (char*)payload);fflush(stdout);
            //Prima di deserializzare, ricevo un JSON dal GW
            struct lgw_pkt_tx_s* temp_obj = ngw_json_to_tx((char*)payload);

            /*Successivamente devo prendere il payload*/
            int payload_size = sizeof(temp_obj->payload)/sizeof(uint8_t);
            uint8_t *cpy_payload = malloc(payload_size);
            memcpy(cpy_payload, temp_obj->payload, payload_size);
            int tempobj_size = temp_obj->size;
            //Libero le risorse del temp_obj
            if(temp_obj != NULL)free(temp_obj);
            //Passo il payload copiato
            //Ora effettuo la deserializzazione del payload del temp_obj
            //NB: deserialize downlink converte pure in hex
            Response *response = deserialize_downlink(node_ref->nwkSKey,node_ref->appSKey,
                cpy_payload, tempobj_size);
            if(response == NULL){
                if(final_topic!=NULL)free(final_topic);
                if(cpy_payload!=NULL)free(cpy_payload);
                struct timeval tstmp_downlink_send;
                gettimeofday(&tstmp_downlink_send, NULL);
                enmqtt_publish("error", final_topic, 0);//Uplink sent - NO downlink on queue
                enmqtt_publish("error,no-downlink\n","/DbALT", 0);
                send_measures(&tstmp_downlink_send,"EN_d_beforesend");
                printf("published ERROR - no downlink\n");fflush(stdout);
                if(payload != NULL)free(payload);
                pthread_mutex_unlock(&msg_received_mutex);
                return;
            }
            //Ed infine forwarding
            /* Il payload in uscita è esadecimale !*/

            struct timeval tstmp_downlink_send;
            gettimeofday(&tstmp_downlink_send, NULL);

            enmqtt_publish(response->payload_hex, final_topic, 0);
            enmqtt_publish(response->payload_hex,"/DbALT", 0);
            
            send_measures(&tstmp_downlink_send,"EN_d_beforesend");

            //Free resources
            if(response->payload_hex != NULL)free(response->payload_hex);
            if(response->payload_decrypted != NULL)free(response->payload_decrypted);
            //Buffer = cpy_payload
            if(response->dwLink->Buffer != NULL)free(response->dwLink->Buffer);
            if(response->dwLink != NULL)free(response->dwLink);
            if(response != NULL)free(response);
            free_message(topic, payload);
            if(final_topic!=NULL)free(final_topic);
            //---
            pthread_mutex_unlock(&msg_received_mutex);
            return;
        }

        temp_topic = routes_getTopicFromAction(DISSEMINATOR_ACTION_UPLINK_NO_ACK);
        final_topic = concatStrs(temp_topic, node_ref->devEUI_c);
        if(strcmp(topic, final_topic) == 0){
            if(node_ref->communication_type == 1){
                send_measures(&msg_arrived,"ENR_u_received");
            }else if(node_ref->communication_type == 0){
                send_measures(&msg_arrived,"EN_u_received");
            }else{
                send_measures(&msg_arrived,"enSDR_u_received");
            }
            printf("FOUND UPLINK TO SEND\n");fflush(stdout);
            if(final_topic!=NULL)free(final_topic);
            /* Il messaggio arriva da esterno in esadecimale, deve essere convertito
            da hex a bytes array. Successivamente costruisco l'uplink e lo invio.*/
            LoRaMacMessageData_t *data_to_send = serialize_hex_uplink(
                node_ref->devAddr, node_ref->sequenceCounter++,
                node_ref->nwkSKey, node_ref->appSKey, (char*)payload, length/2);
            free_message(topic, payload);//Payload free cause hex2bin
            send_uplink_no_ack(data_to_send->Buffer,data_to_send->BufSize);
            if(data_to_send != NULL)free(data_to_send);
            printf("Message succesfully sent\n");
        }
        //Ignora
        pthread_mutex_unlock(&msg_received_mutex);
    }
}

JoinAcceptData_t *send_join_request_await_accept(uint8_t *buffer, uint8_t size, LoRaNodeInfo *paramnode_ref){
    obj = NULL;
    node_ref = paramnode_ref;
    struct lgw_pkt_rx_s rxpkt = generate_rx_packet(buffer,size);
    if(node_ref->communication_type == 1){
        printf("Communication type hybrid\n");
        char* json = ngw_rx_to_json(&rxpkt);

        printf("Messaggio End-Node - Broker: %s\n",json);
        fflush(stdout);
        int new_size = 256*2;
        char payload_str[new_size];
        int new_char_size = bin_to_b64(buffer, size, payload_str, new_size);
        char payload_to_send[new_char_size];
        memcpy(payload_to_send, payload_str,new_char_size+1);

        char* temp_topic = routes_getTopicFromAction(ACTION_RADIO_RECEIVE);
        char* final_topic_rec = concatStrs(temp_topic, node_ref->devEUI_c);
        temp_topic = routes_getTopicFromAction(ACTION_RADIO_SEND);
        char* final_topic_send = concatStrs(temp_topic, node_ref->devEUI_c);

        char *topics[] = {final_topic_rec};
        enmqtt_listen_topics(topics, 1, &on_message_received);
        
        struct timeval msg_arrived;
        gettimeofday(&msg_arrived, NULL);

        if(!enmqtt_publish(payload_to_send, final_topic_send, 0)){
            enmqtt_stop_listen();
            printf("ERROR\n");
            fflush(stdout);
            return NULL;
        }
        JSON_Value *root_value = json_value_init_object();
        JSON_Object *root_object = json_value_get_object(root_value);
        json_object_dotset_string(root_object, "msg.type", "INFO");
        json_object_dotset_string(root_object, "msg.ref.type", "EN");
        json_object_dotset_string(root_object, "msg.category", "join-request_sent");
        json_object_dotset_number(root_object, "msg.timestamp.value", msg_arrived.tv_sec);
        json_object_dotset_number(root_object, "msg.timestamp.us", msg_arrived.tv_usec);
        char* serialized_string = json_serialize_to_string(root_value);
        enmqtt_publish(serialized_string,"/DbALT", 0);
        json_free_serialized_string(serialized_string);
        json_value_free(root_value);
        printf("OK\n");fflush(stdout);
    }else if(node_ref->communication_type == 0){
        printf("Communication type virtual\n");
        char* json = ngw_rx_to_json(&rxpkt);

        printf("Messaggio End-Node - Broker: %s\n",json);
        fflush(stdout);
        char *actions[] = {ACTION_JOIN_ACCEPT};
        enmqtt_listen(actions, 1, &on_message_received);

        char str_to_print[300];

        struct timeval msg_arrived;
        gettimeofday(&msg_arrived, NULL);

        if(!enmqtt_publish_action(json, ACTION_JOIN_REQUEST, 0)){
            if(json != NULL)free(json);
            enmqtt_stop_listen();
            printf("ERROR\n");fflush(stdout);
            return NULL;
        }
        if(json != NULL)free(json);
        JSON_Value *root_value = json_value_init_object();
        JSON_Object *root_object = json_value_get_object(root_value);
        json_object_dotset_string(root_object, "msg.type", "INFO");
        json_object_dotset_string(root_object, "msg.ref.type", "EN");
        json_object_dotset_string(root_object, "msg.category", "join-request_sent");
        json_object_dotset_number(root_object, "msg.timestamp.value", msg_arrived.tv_sec);
        json_object_dotset_number(root_object, "msg.timestamp.us", msg_arrived.tv_usec);
        char* serialized_string = json_serialize_to_string(root_value);
        enmqtt_publish(serialized_string,"/DbALT", 0);
        json_free_serialized_string(serialized_string);
        json_value_free(root_value);
        printf("OK2\n");
        fflush(stdout);
    }
    else{
        printf("Communication type hybrid SDR\n");
        char* json = ngw_rx_to_json(&rxpkt);

        printf("Messaggio End-Node - Broker: %s\n",json);
        fflush(stdout);
        char *actions[] = {ACTION_JOIN_ACCEPT};
        enmqtt_listen(actions, 1, &on_message_received);

        char str_to_print[300];

        struct timeval msg_arrived;
        gettimeofday(&msg_arrived, NULL);

        if(!enmqtt_publish_action(json, ACTION_JOIN_REQUEST, 0)){
            if(json != NULL)free(json);
            enmqtt_stop_listen();
            printf("ERROR\n");fflush(stdout);
            return NULL;
        }
        if(json != NULL)free(json);
        JSON_Value *root_value = json_value_init_object();
        JSON_Object *root_object = json_value_get_object(root_value);
        json_object_dotset_string(root_object, "msg.type", "INFO");
        json_object_dotset_string(root_object, "msg.ref.type", "EN");
        json_object_dotset_string(root_object, "msg.category", "join-request_sent");
        json_object_dotset_number(root_object, "msg.timestamp.value", msg_arrived.tv_sec);
        json_object_dotset_number(root_object, "msg.timestamp.us", msg_arrived.tv_usec);
        char* serialized_string = json_serialize_to_string(root_value);
        enmqtt_publish(serialized_string,"/DbALT", 0);
        json_free_serialized_string(serialized_string);
        json_value_free(root_value);
    }

    pthread_mutex_t join_accept_arrived_mtx = PTHREAD_MUTEX_INITIALIZER;
    printf("Waiting Join Accept from server.\n");
    pthread_mutex_lock(&join_accept_arrived_mtx);
    pthread_cond_wait(&join_accept_arrived_cond,&join_accept_arrived_mtx);
    pthread_mutex_unlock(&join_accept_arrived_mtx);
    return obj;//Rimane inviariato per sempre
}

bool send_uplink_no_ack(uint8_t *buffer, uint8_t size){
    pthread_mutex_lock(&connected_mtx);
    if(!lorawan_connected){
        pthread_cond_wait(&connected_cond, &connected_mtx);
        pthread_mutex_unlock(&connected_mtx);
    }else{
        pthread_mutex_unlock(&connected_mtx);
    }
    if(node_ref->communication_type == 0){
        struct lgw_pkt_rx_s elem = generate_rx_packet(buffer,size);
        struct lgw_pkt_rx_s *ptr = &elem;
        char* jsontosend = ngw_rx_to_json(ptr);

        struct timeval time_send;
        gettimeofday(&time_send, NULL);

        bool result = enmqtt_publish_action(jsontosend, ACTION_UPLINK_NO_ACK, 0);
        if(jsontosend != NULL)free(jsontosend);
        
        send_measures(&time_send,"EN_u_beforesend");

        return result;
    }else if(node_ref->communication_type == 1){
        int new_size = 256*2;
        char payload_str[new_size];
        int new_char_size = bin_to_b64(buffer, size, payload_str, new_size);
        char payload_to_send[new_char_size];
        memcpy(payload_to_send, payload_str,new_char_size+1);

        char* temp_topic = routes_getTopicFromAction(ACTION_RADIO_SEND);
        char* final_topic_send = concatStrs(temp_topic, node_ref->devEUI_c);

        struct timeval time_send;
        gettimeofday(&time_send, NULL);

        bool result = enmqtt_publish(payload_to_send, final_topic_send, 0);
        
        send_measures(&time_send,"ENR_u_beforesend");

        return result;
    }
    else{
        int new_size = size;
        char payload_str[new_size];
        void *payload_str_ptr = payload_str;
        printf("Message to send is:\n");
        int i = new_size;
        int j = 0;
        while(i--){
            sprintf(payload_str_ptr, "%02X", buffer[j]);
            payload_str_ptr += 2;
            j++;
        }
        printf("%s\n",payload_str);

        char* final_topic_send = concatStrs("/SigGen/Uplink/", node_ref->devEUI_c);

        struct timeval time_send;
        gettimeofday(&time_send, NULL);

        bool result = enmqtt_publish(payload_str, final_topic_send, 0);
        
        send_measures(&time_send,"enSDR_u_beforesend");

        return result;
    }
}

bool send_is_ready_alert(){
    //Creo nwk e app keys
    int common_dim = 16*2+1;
    char *nwkskey_str = malloc(common_dim);
    char *ptr = nwkskey_str;
    for (int r = 0; r < 16; r++) {
        ptr += sprintf(ptr, "%02X", node_ref->nwkSKey[r]);
    }
    char *appskey_str = malloc(common_dim);
    ptr = appskey_str;    
    for (int r = 0; r < 16; r++) {
        ptr += sprintf(ptr, "%02X", node_ref->appSKey[r]);
    }



    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_number(root_object, "status", 1);
    json_object_set_string(root_object, "message", "Ready");
    json_object_set_number(root_object, "devAddr", node_ref->devAddr);
    json_object_set_string(root_object, "nwkSKey", nwkskey_str);
    json_object_set_string(root_object, "appSKey", appskey_str);
    char *serialized_string = json_serialize_to_string(root_value);
    bool result = enmqtt_publish_action(serialized_string, DISSEMINATOR_ACTION_BROADCAST_ALERTS, 2);
    free(nwkskey_str);
    free(appskey_str);
    json_free_serialized_string(serialized_string);
    json_value_free(root_value);


    
    return result;
}


bool send_info_message(char* message) {
    return enmqtt_publish_action(message, DISSEMINATOR_ACTION_MESSAGES, 1);
}

struct lgw_pkt_rx_s generate_rx_packet(uint8_t *buffer, uint8_t size){
    //Dati fittizi + dati corretti.
    struct lgw_pkt_rx_s data;
    data.freq_hz = 868.1;              /*!> central frequency of the IF chain */
    data.if_chain = 0;
    data.status = STAT_CRC_OK;                    /*!> status of the received packet */
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    data.count_us = (uint32_t)(currentTime.tv_sec * ((int)1e6) + currentTime.tv_usec);
    //data.count_us = time(NULL);         /*!> internal concentrator counter for timestamping, 1 microsecond resolution */
    data.rf_chain = 1;
    data.modulation = MOD_LORA;         /*!> modulation used by the packet */
    data.bandwidth = BW_125KHZ;            /*!> modulation bandwidth (LoRa only) */
    data.datarate = DR_LORA_SF9;               //E' un SF9 il test
    data.coderate = CR_LORA_4_5;        /* stesso link sopra generico fissato a 4/5 per LoRaWAN */
    data.rssi = -17;                     /*!> average packet RSSI in dB */
    data.snr = 9.5;                     /*!> average packet SNR, in dB (LoRa only) */
    data.snr_min = 9;                 /*!> minimum packet SNR, in dB (LoRa only) */
    data.snr_max = 10;                 /*!> maximum packet SNR, in dB (LoRa only) */
    data.crc = 0;                       /*!> CRC that was received in the payload *///TODO
    data.size = size;                   /*!> Payload size in Bytes + HEADER size in Bytes*/
    memcpy(data.payload, buffer, size);/*!> buffer containing the payload */
    return data;
}

char *concatStrs(char* st,char* st2){
    char *result = malloc(strlen(st) + strlen(st2) + 1);
    result[0]  = '\0';
    strncat(result, st, strlen(st));
    strncat(result, st2, strlen(st2));
    return result;
}
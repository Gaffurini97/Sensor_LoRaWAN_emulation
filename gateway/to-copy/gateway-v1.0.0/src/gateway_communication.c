#include "../headers/gateway_communication.h"

void on_message_received(char* topic, void* payload, int length){
    printf("GW Message received %s\n", topic);
    char* action = routes_getTopicFromAction(topic);
    if(strcmp(action, ACTION_JOIN_REQUEST) == 0
        || strcmp(action, ACTION_UPLINK_NO_ACK) == 0){
        struct lgw_pkt_rx_s *obhj = ngw_json_to_rx((char*)payload);
        push_r(*obhj);
    }
    //Else ignore
}

int lgw_start(void){
    if(initialize_stacks()!=0){
        return LGW_HAL_ERROR;
    }
    char *actions[] = {ACTION_JOIN_REQUEST, ACTION_UPLINK_NO_ACK};
    mqtt_listen(actions, 2, &on_message_received);
    printf("GW Listening\n");
    return LGW_HAL_SUCCESS;
}

int lgw_stop(void){
    mqtt_stop_listen();
    free(stack_r);
    return LGW_HAL_SUCCESS;
}


/* Max_pkt = max number of pkts that can be popped from stack. */
int lgw_receive(uint8_t max_pkt, struct lgw_pkt_rx_s *pkt_data){
    int status = STACK_OK;
    int i = 0;
    while(i < max_pkt && status == STACK_OK){
        status = pop_r(&pkt_data[i++]);
    }
    i--;
    //If it receives > 1 return ok, else return 0 ( = no data to read).
    if(i == 0) {
        return 0;
    }
    return 1;
}

int lgw_send(struct lgw_pkt_tx_s pkt_data){
    uint8_t first_byte = pkt_data.payload[0];
    /*
    11100000 = 224 = MASK
                0        1       2       3       4    5
    shift 5 => 11100000 1110000, 111000, 11100, 1110, 111
    */
    uint8_t FType = 0;
    uint8_t MASK = 224;
    FType = first_byte & MASK;
    FType = FType>>5;
    char* topic;
    if(FType == 1){//Join Accept
        printf("-Found Join Accept-\n");
        topic = routes_getTopicFromAction(ACTION_JOIN_ACCEPT);
    }else{//Downlink...
        printf("-Found Downlink-\n");
        topic = routes_getTopicFromAction(ACTION_DOWNLINK_NO_ACK);
    }
    char* json = ngw_tx_to_json(&pkt_data);
    printf("Messaggio Gateway - Broker: %s\n",json);
    if(!mqtt_publish(json, topic, 0)){
        free(json);
        return LGW_HAL_ERROR;
    }
    free(json);
    return LGW_HAL_SUCCESS;
}


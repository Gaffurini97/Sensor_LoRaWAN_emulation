#include "../headers/lora_node.h"
/*SEE https://www.thethingsnetwork.org/docs/lorawan/end-device-activation*/

bool save_last_devNonce(const char* conf_file,uint16_t last){
    /* load configuration file */
	if (access(conf_file, R_OK) == 0) {
		printf("INFO: found global configuration file config.json, parsing it\n");
	} else {
		printf("ERROR: [main] failed to find any configuration file named config.json\n");
		return false;
	}
	/* try to parse JSON */
	JSON_Value *root_val = json_parse_file_with_comments(conf_file);
	if (root_val == NULL) {
		printf("ERROR: %s is not a valid JSON file\n", conf_file);
		return false;
	}
	/* point to the gateway configuration object */
	JSON_Object *root_object = json_value_get_object(root_val);
	JSON_Object *conf_obj = json_object_get_object(root_object, "lorawan_info");
	if (conf_obj == NULL) {
		printf("INFO: %s does not contain a JSON object named lorawan_info\n", conf_file);
		return false;
	}
    json_object_remove(conf_obj,"last_dev_nonce");
    json_object_set_number(conf_obj,"last_dev_nonce",last);
    char* serialized_string = json_serialize_to_string_pretty(root_val);
    
    FILE *fptr = fopen(conf_file,"w");
    fprintf(fptr,"%s",serialized_string);
    fclose(fptr);

    json_value_free(root_val);
    json_free_serialized_string(serialized_string);
    return true;
}

bool executeJoin(LoRaNodeInfo *node_ref){
    printf("-------------START JOIN PROCEDURE ----------");
    //Creo il messaggio
    LoRaMacMessageJoinRequest_t *jreq_d = generateJoinRequest(node_ref->devEUI_c,
        node_ref->joinEUI_c,node_ref->appKey_c,node_ref->lastDevNonce);

    node_ref->lastDevNonce = jreq_d->DevNonce;
    //Lo invio al gateway e ricevo una risposta
    JoinAcceptData_t *jacpt_d = send_join_request_await_accept(
        jreq_d->Buffer,
        jreq_d->BufSize,
        node_ref);
    if(jacpt_d == NULL){
        printf("RUNTIME ERROR: something wrong with OTAA.\n");
        return false;
    }
    //Salvo i dati necessari nel riferimento
    node_ref->appSKey = jacpt_d->appSKey;
    node_ref->nwkSKey = jacpt_d->nwkSKey;
    node_ref->devAddr = jacpt_d->msg->DevAddr;
    node_ref->j_accept_data = jacpt_d;
    
    
    printf("------------ END JOIN PROCEDURE ----------\n");

    node_ref->lastDevNonce+=1;
    if(!save_last_devNonce(CONFIGURATION_FILE_PATH,node_ref->lastDevNonce)){
        return false;
    }

    printf("-------- CHANGING TOPICS PROCEDURE -------\n");

    /*
    Ora rimane in ascolto dei downlink e
    degli uplink da inviare specifici (inviati da esterno)
    */
    
    char *downlink_generic_topic = routes_getTopicFromAction(ACTION_DOWNLINK_NO_ACK);
    char *uplink_specific_topic = concatStrs(
        routes_getTopicFromAction(DISSEMINATOR_ACTION_UPLINK_NO_ACK),
        node_ref->devEUI_c
    );
    char *radio_downlink_specific_topic = concatStrs(
        routes_getTopicFromAction(ACTION_RADIO_RECEIVE),
        node_ref->devEUI_c
    );
    char *new_topics[] = {
        downlink_generic_topic,
        uplink_specific_topic,
        radio_downlink_specific_topic
    };
    int topics_count = 3;
    change_topics(new_topics, topics_count);
    printf("Now I'm listening to <<");
    for(int i = 0; i < topics_count; i++){
        printf("%s ", new_topics[i]);
    }
    printf(">>\n");

    printf("End-Node is Ready\n");
    //Invio un messaggio sul canale di alert che specifica che il nodo è stato inizializzato correttamente
    //ed è pronto a ricevere messaggi / "forwardare" messaggi
    
    //Addr ++ str
    set_connected();
    return send_is_ready_alert();
}
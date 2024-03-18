#include "../headers/initializer.h"

LoRaNodeInfo *node_info;

static int parse_configuration_file(const char *conf_file);
static int parse_actions_informations(JSON_Value *obj_value);
static int parse_mqtt_informations(JSON_Object *conf_obj);
static int parse_lorawan_info(JSON_Object *conf_obj);


int main(int argc, char *argv[]){
	
	if(parse_configuration_file(CONFIGURATION_FILE_PATH) == -1){
		return -1;
	}

    //Start OTAA
    printf("\n-----------OTAA-------------------------\n");
    if(executeJoin(node_info)){
        printf("OTAA Success\n");
    }else{
        printf("OTAA Failure\n");
	}
    pthread_mutex_t quit_mtx = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&quit_mtx);
	/*
	quit_cond è in lora_node_communication.h
	è per le implementazioni future, serve per effettuare un
	"safe quit" liberando tutte le risorse prima di uscire.
	*/
    pthread_cond_wait(&quit_cond, &quit_mtx);
    pthread_mutex_unlock(&quit_mtx);
    enmqtt_stop_listen();
    freeMQTT();
    routes_free();
    return 0;
}



static int parse_lorawan_info(JSON_Object *conf_obj){
    node_info = malloc(sizeof(LoRaNodeInfo));
    node_info->appSKey = NULL;
    node_info->nwkSKey = NULL;
    node_info->j_accept_data = NULL;
    node_info->devAddr = 0;
    node_info->sequenceCounter = 0;
    const char* devEUI_c = json_object_get_string(conf_obj, "devEUI");
	if (devEUI_c == NULL) {
		return -1;
	}
    node_info->devEUI_c = strdup(devEUI_c);
    const char* joinEUI_c = json_object_get_string(conf_obj, "joinEUI");
	if (joinEUI_c == NULL) {
		return -1;
	}
    node_info->joinEUI_c = strdup(joinEUI_c);
    const char* appKey_c = json_object_get_string(conf_obj, "appKey");
	if (appKey_c == NULL) {
		return -1;
	}
    node_info->appKey_c = strdup(appKey_c);
    const char* node_name = json_object_get_string(conf_obj, "nodeName");
	if (node_name == NULL) {
		return -1;
	}
    node_info->node_name = strdup(node_name);
    JSON_Value *last_dev_nonce = json_object_get_value(conf_obj, "last_dev_nonce");
	if (last_dev_nonce == NULL) {
		return -1;
	}
    node_info->lastDevNonce = (uint16_t)json_value_get_number(last_dev_nonce);
    return 0;
}
static int parse_mqtt_informations(JSON_Object *conf_obj){
	const char* mqtt_host_name = json_object_get_string(conf_obj, "host_name");
	if (mqtt_host_name == NULL) {
		return -1;
	}
	
	JSON_Value *val = json_object_get_value(conf_obj, "port");
	if (val == NULL) {
		return -1;
	}
	int serv_port_up = (int)json_value_get_number(val);

	const char* mqtt_client_id = json_object_get_string(conf_obj, "client_id");
	if (mqtt_client_id == NULL) {
		return -1;
	}

	const char* mqtt_username = json_object_get_string(conf_obj, "username");
	if (mqtt_username == NULL) {
		return -1;
	}

	const char* mqtt_password = json_object_get_string(conf_obj, "password");
	if (mqtt_password == NULL) {
		return -1;
	}
	initializeMQTT(
		strdup(mqtt_host_name), serv_port_up,
		strdup(mqtt_client_id), strdup(mqtt_username),
		strdup(mqtt_password));
	return 0;
}
static int parse_actions_informations(JSON_Value *obj_value){
	JSON_Array *actions_mapping = json_value_get_array(obj_value);
	JSON_Object *temp_obj;
	if(actions_mapping == NULL){
		printf("NO Actions mapping");
		fflush(stdout);
		return -1;
	}
	routes_Initialize();
	for(int i = 0; i < json_array_get_count(actions_mapping); i++){
		temp_obj = json_array_get_object(actions_mapping, i);
		if(temp_obj == NULL){
			return -1;
		}
		const char* temp_action = json_object_get_string(temp_obj, "action");
		if(temp_action == NULL){
			return -1;
		}
		const char* temp_topic = json_object_get_string(temp_obj, "topic");
		if(temp_topic == NULL){
			return -1;
		}
		routes_load_key_topic_map(strdup(temp_action), strdup(temp_topic));
		routes_load_key_topic_map(strdup(temp_topic), strdup(temp_action));
	}
	return 0;
}

static int parse_configuration_file(const char *conf_file){
	JSON_Object *conf_obj = NULL;
    JSON_Value *val = NULL;

	/* load configuration file */
	if (access(conf_file, R_OK) == 0) {
		printf("INFO: found global configuration file config.json, parsing it\n");
	} else {
		printf("ERROR: [main] failed to find any configuration file named config.json\n");
		return -1;
	}
	
	
	/* try to parse JSON */
	JSON_Value *root_val = json_parse_file_with_comments(conf_file);
	if (root_val == NULL) {
		printf("ERROR: %s is not a valid JSON file\n", conf_file);
		return -1;
	}
	
	/* point to the gateway configuration object */
	JSON_Object *root_object = json_value_get_object(root_val);

	conf_obj = json_object_get_object(root_object, "mqtt_communication");
	if (conf_obj == NULL) {
		printf("INFO: %s does not contain a JSON object named mqtt_communication\n", conf_file);
		return -1;
	} else {
		printf("INFO: %s does contain a JSON object named mqtt_communication, parsing end-node parameters\n", conf_file);
	}
    if(parse_mqtt_informations(conf_obj) == -1){
		return -1;
	}

    val = json_object_get_value(root_object, "actions_mapping");
	if (val == NULL) {
		printf("INFO: %s does not contain a JSON object named actions_mapping\n", conf_file);
		return -1;
	} else {
		printf("INFO: %s does contain a JSON object named actions_mapping, parsing end-node parameters\n", conf_file);
	}
	if(parse_actions_informations(val) == -1){
		return -1;
	}

    conf_obj = json_object_get_object(root_object, "lorawan_info");
	if (conf_obj == NULL) {
		printf("INFO: %s does not contain a JSON object named lorawan_info\n", conf_file);
		return -1;
	} else {
		printf("INFO: %s does contain a JSON object named lorawan_info, parsing end-node parameters\n", conf_file);
	}
    if(parse_lorawan_info(conf_obj) == -1){
		return -1;
	}

	JSON_Value *communication_type_jv = json_object_get_value(root_object, "communication_type");
	if (communication_type_jv == NULL) {
		return -1;
	}
	node_info->communication_type = (int)json_value_get_number(communication_type_jv);
	printf("Communication type %d\n",node_info->communication_type);
	fflush(stdout);
    JSON_Value *initial_sleep_in_sec = json_object_get_value(root_object, "initial_sleep_in_sec");
	if (initial_sleep_in_sec == NULL) {
		return -1;
	}
    int sec_to_wait = (int)json_value_get_number(initial_sleep_in_sec);

    fflush(stdout);
    if(sec_to_wait>0) usleep(sec_to_wait*1000000);
	fflush(stdout);
    //End info initialization
	return 0;
}

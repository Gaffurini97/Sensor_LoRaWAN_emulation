#include "../headers/enmqtt.h"


/* Private things */

void(*enon_message_received)(char* topic, void* payload, int length);

pthread_mutex_t enmosquitto_mtx = PTHREAD_MUTEX_INITIALIZER;
static bool eninitialized_and_connected = false;
static struct mosquitto *enmosq;

pthread_mutex_t enrun_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t enrun_cond = PTHREAD_COND_INITIALIZER;
static int run = 1;

bool changing_topics = false;

pthread_mutex_t entopics_mtx = PTHREAD_MUTEX_INITIALIZER;
static char **encurrent_topics;
static int encurrent_topics_length;

void enconnect_callback(struct mosquitto *mosq, void *obj, int result);
void enmessage_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message);
void enmy_disconnect_callback(struct mosquitto *mosq, void *obj, int rc);
void enmqtt_loop();
void encheck_initialized();
void eninitialize_and_connect_mqtt();

void enconnect_callback(struct mosquitto *mosq, void *obj, int result)
{
    printf("connect callback, rc = %d\n", result);
}
void enmessage_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
    printf("Receive message %s %s\n", message->topic, (char*)message->payload);
    int topic_size = strlen(message->topic);
    char* topic = malloc(topic_size+1);
    memset(topic, '\0', topic_size);
    strncpy(topic, message->topic, topic_size+1);

    int payload_size = message->payloadlen;
    char* payload = malloc(payload_size+1);
    memset(payload, '\0', payload_size);
    strncpy(payload, (char*)message->payload, payload_size+1);

    if(!changing_topics){
        (*enon_message_received)(topic, payload, payload_size);
    }
    fflush(stdout);
}
void enmy_disconnect_callback(struct mosquitto *mosq, void *obj, int rc)
{
    printf("Disconnected!\n");
    eninitialize_and_connect_mqtt();
    enmqtt_publish("RECONNECTION","/DbALT",0);
	eninitialized_and_connected = false;
}



void initializeMQTT(char* mqtt_host_name, int mqtt_port,
            char* mqtt_client_id, char* mqtt_username, char* mqtt_password){
    info = malloc(sizeof(ClientMQTTInfo));
    if(info == NULL){
        printf("INFO NULL \n");
        exit(-1);
    }
    info->mqtt_host_name = mqtt_host_name;
    info->mqtt_port = mqtt_port;
    info->mqtt_client_id = mqtt_client_id;
    info->mqtt_username = mqtt_username;
    info->mqtt_password = mqtt_password;

    encurrent_topics = NULL;
    encurrent_topics_length = 0;
}
void freeMQTT(){
    free(info);
}

void eninitialize_and_connect_mqtt(){

    //enmosq = mosquitto_new(info->mqtt_client_id, true, NULL);
    enmosq = mosquitto_new(info->mqtt_client_id, true, NULL);
    if(!enmosq){
        printf("Cant initiallize mosquitto library\n");
        pthread_mutex_unlock(&enmosquitto_mtx);
        exit(-1);
    }
	mosquitto_username_pw_set(enmosq, info->mqtt_username, info->mqtt_password);

	if(enmosq){
        mosquitto_connect_callback_set(enmosq, enconnect_callback);
        mosquitto_message_callback_set(enmosq, enmessage_callback);
        mosquitto_disconnect_callback_set(enmosq, enmy_disconnect_callback);
        //mosquitto connect
        int rc = mosquitto_connect(enmosq, info->mqtt_host_name, info->mqtt_port, 60);
        if(rc != MOSQ_ERR_SUCCESS){
            printf("Failed to connect with mosquitto\n");
            exit(-1);
        }
    }
}

void encheck_initialized(){
    pthread_mutex_lock(&enmosquitto_mtx);
    if(!eninitialized_and_connected){
        if(info == NULL){
            printf("You have not initialized MQTT before\n");
            pthread_mutex_unlock(&enmosquitto_mtx);
            exit(-1);
        }
        eninitialize_and_connect_mqtt();
        eninitialized_and_connected = true;
    }
    pthread_mutex_unlock(&enmosquitto_mtx);
}

/* Public things */
void enmqtt_listen(char **actions,int actions_len,
            void(*on_msg_rcvd)(char* topic, void* payload, int length))
{
    enon_message_received = on_msg_rcvd;
    run = 1;
    int rc = 0;
    if(encurrent_topics_length > 0 && encurrent_topics != NULL){
        for(int i = 0; i < encurrent_topics_length; i++){
            if(encurrent_topics[i] != NULL){
                free(encurrent_topics[i]);
            }
        }
        free(encurrent_topics);
    }
    encurrent_topics = malloc(sizeof(char*) * actions_len);
    pthread_mutex_lock(&entopics_mtx);
    for(int i = 0; i < actions_len; i++){
        encurrent_topics[i] = routes_getTopicFromAction(actions[i]);
    }
    encurrent_topics_length = actions_len;
    pthread_mutex_unlock(&entopics_mtx);

    encheck_initialized();

    //subscribe all topics
    subscribe_current_topics();
    
    pthread_t thid;
    pthread_create(&thid, NULL, (void*(*)(void*))&enmqtt_loop, NULL);
}

void enmqtt_listen_topics(char **topics,int topics_len,
            void(*on_msg_rcvd)(char* topic, void* payload, int length))
{
    enon_message_received = on_msg_rcvd;
    run = 1;
    int rc = 0;
    if(encurrent_topics_length > 0 && encurrent_topics != NULL){
        for(int i = 0; i < encurrent_topics_length; i++){
            if(encurrent_topics[i] != NULL){
                free(encurrent_topics[i]);
            }
        }
        free(encurrent_topics);
    }
    encurrent_topics = malloc(sizeof(char*) * topics_len);
    pthread_mutex_lock(&entopics_mtx);
    for(int i = 0; i < topics_len; i++){
        encurrent_topics[i] = strdup(topics[i]);
    }
    encurrent_topics_length = topics_len;
    pthread_mutex_unlock(&entopics_mtx);

    encheck_initialized();

    //subscribe all topics
    subscribe_current_topics();
    
    pthread_t thid;
    pthread_create(&thid, NULL, (void*(*)(void*))&enmqtt_loop, NULL);
}


void change_topics(char **new_topics, int new_topics_len){
    change_actORtopics(new_topics, new_topics_len, false);
}

void change_actions(char **new_actions, int new_actions_len){
    change_actORtopics(new_actions, new_actions_len, true);
}

void change_actORtopics(char **new_actions, int new_actions_len, bool is_action){
    changing_topics = true;
    printf("Unsubscribing current topics\n");
    unsubscribe_current_topics();
    printf("Change current topics\n");
    pthread_mutex_lock(&entopics_mtx);
    if(encurrent_topics_length > 0 && encurrent_topics != NULL){
        for(int i = 0; i < encurrent_topics_length; i++){
            if(encurrent_topics[i] != NULL){
                free(encurrent_topics[i]);
            }
        }
        free(encurrent_topics);
    }
    encurrent_topics = malloc(sizeof(char*) * new_actions_len);
    for(int i = 0; i < new_actions_len; i++){
        char *temp = new_actions[i];
        encurrent_topics[i] = (is_action?routes_getTopicFromAction(temp):temp);
    }
    encurrent_topics_length = new_actions_len;
    pthread_mutex_unlock(&entopics_mtx);
    printf("Subscribing current topics\n");
    subscribe_current_topics();
    changing_topics = false;
}

void subscribe_current_topics(){
    pthread_mutex_lock(&entopics_mtx);
    for(int i = 0; i < encurrent_topics_length; i++){
        int rc = mosquitto_subscribe(enmosq, NULL, encurrent_topics[i], 0);
        if(rc != MOSQ_ERR_SUCCESS){
            printf("Failed to SUBSCRIBE with mosquitto\n");
            exit(-1);
        }
        enmqtt_publish("TOPIC SOTTOSCRITTO:","/DbALT",0);
        enmqtt_publish(encurrent_topics[i],"/DbALT",0);
    }
    pthread_mutex_unlock(&entopics_mtx);
}

void unsubscribe_current_topics(){
    printf("Unsubscribing current");
    int *res;
    pthread_mutex_lock(&entopics_mtx);
    for(int i = 0; i < encurrent_topics_length; i++){
        mosquitto_unsubscribe(enmosq, res, encurrent_topics[i]);
        enmqtt_publish("TOPIC RIMOSSO DA:","/DbALT",0);
        enmqtt_publish(encurrent_topics[i],"/DbALT",0);
    }
    pthread_mutex_unlock(&entopics_mtx);
}

void ensleep_till_stop_listening(){
    pthread_mutex_t wait_mtx = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&wait_mtx);
    pthread_cond_wait(&enrun_cond, &wait_mtx);
    pthread_mutex_unlock(&wait_mtx);
}

void enmqtt_loop(){
    printf("starting loop\n");
    mosquitto_loop_start(enmosq);

    pthread_mutex_t wait_mtx = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&wait_mtx);
    pthread_cond_wait(&enrun_cond, &wait_mtx);
    pthread_mutex_unlock(&wait_mtx);

    mosquitto_loop_stop(enmosq, true);

    unsubscribe_current_topics();

    mosquitto_disconnect(enmosq);
    mosquitto_destroy(enmosq);
    pthread_exit((void*)0);
}

void enmqtt_stop_listen() {
    pthread_mutex_lock(&enrun_mtx);
    run = 0;
    pthread_cond_signal(&enrun_cond);
    pthread_mutex_unlock(&enrun_mtx);
}

bool enmqtt_publish_action(char* message, char* action, int qos){
    return enmqtt_publish(message, routes_getTopicFromAction(action), qos);
}


bool enmqtt_publish(char* message, char* topic, int qos)
{
    encheck_initialized();
    int rc;
    rc = mosquitto_publish(enmosq, NULL, topic, strlen(message), message, qos, false);
    if(rc){
        switch(rc){
            case MOSQ_ERR_INVAL:
                fprintf(stderr, "Error: Invalid input. Does your topic contain '+' or '#'?\n");
                break;
            case MOSQ_ERR_NOMEM:
                fprintf(stderr, "Error: Out of memory when trying to publish message.\n");
                break;
            case MOSQ_ERR_NO_CONN:
                fprintf(stderr, "Error: Client not connected when trying to publish.\n");
                break;
            case MOSQ_ERR_PROTOCOL:
                fprintf(stderr, "Error: Protocol error when communicating with broker.\n");
                break;
            case MOSQ_ERR_PAYLOAD_SIZE:
                fprintf(stderr, "Error: Message payload is too large.\n");
                break;
        }
    }
    if(rc == MOSQ_ERR_SUCCESS){
        return true;
    }
    return false;
}

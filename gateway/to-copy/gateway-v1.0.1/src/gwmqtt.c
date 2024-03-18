#include "../headers/gwmqtt.h"


/* Private things */

void(*gwon_message_received)(char* topic, void* payload, int lgwgth);

pthread_mutex_t gwmosquitto_mtx = PTHREAD_MUTEX_INITIALIZER;
static bool gwinitialized_and_connected = false;
static struct mosquitto *gwmosq;

pthread_mutex_t gwrun_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t gwrun_cond = PTHREAD_COND_INITIALIZER;
static int run = 1;

bool changing_topics = false;

pthread_mutex_t gwtopics_mtx = PTHREAD_MUTEX_INITIALIZER;
static char **gwcurrent_topics;
static int gwcurrent_topics_length;

void gwconnect_callback(struct mosquitto *mosq, void *obj, int result);
void gwmessage_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message);
void gwmy_disconnect_callback(struct mosquitto *mosq, void *obj, int rc);
void gwmqtt_loop();
void gwcheck_initialized();
void gwinitialize_and_connect_mqtt();

void gwconnect_callback(struct mosquitto *mosq, void *obj, int result)
{
    printf("connect callback, rc = %d\n", result);
}
void gwmessage_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
    printf("receive message(%s) : %s\n",message->topic, (char*)message->payload);
    if(!changing_topics)
        (*gwon_message_received)(message->topic, message->payload, message->payloadlen);
}
void gwmy_disconnect_callback(struct mosquitto *mosq, void *obj, int rc)
{
    printf("Disconnected!\n");
	gwinitialized_and_connected = false;
}



void initializeMQTT(char* mqtt_host_name, int mqtt_port,
            char* mqtt_client_id, char* mqtt_username, char* mqtt_password){
    printf("INITIALIZING MQTT\n");
    fflush(stdout);
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

    gwcurrent_topics = NULL;
    gwcurrent_topics_length = 0;
}
void freeMQTT(){
    free(info);
}

void gwinitialize_and_connect_mqtt(){
    gwmosq = mosquitto_new(info->mqtt_client_id, true, NULL);
    if(!gwmosq){
        printf("Cant initialize mosquitto library\n");
        pthread_mutex_unlock(&gwmosquitto_mtx);
        exit(-1);
    }
	mosquitto_username_pw_set(gwmosq, info->mqtt_username, info->mqtt_password);

	if(gwmosq){
        mosquitto_connect_callback_set(gwmosq, gwconnect_callback);
        mosquitto_message_callback_set(gwmosq, gwmessage_callback);
        mosquitto_disconnect_callback_set(gwmosq, gwmy_disconnect_callback);
        //mosquitto connect
        int rc = mosquitto_connect(gwmosq, info->mqtt_host_name, info->mqtt_port, 60);
        if(rc != MOSQ_ERR_SUCCESS){
            printf("Failed to connect with mosquitto\n");
            exit(-1);
        }
    }
}

void gwcheck_initialized(){
    pthread_mutex_lock(&gwmosquitto_mtx);
    if(!gwinitialized_and_connected){
        if(info == NULL){
            printf("You have not initialized MQTT before\n");
            pthread_mutex_unlock(&gwmosquitto_mtx);
            exit(-1);
        }
        gwinitialize_and_connect_mqtt();
        gwinitialized_and_connected = true;
    }
    pthread_mutex_unlock(&gwmosquitto_mtx);
}

/* Public things */
void mqtt_listen(char **actions,int actions_len,
            void(*on_msg_rcvd)(char* topic, void* payload, int length))
{
    gwon_message_received = on_msg_rcvd;
    run = 1;
    int rc = 0;
    if(gwcurrent_topics_length > 0 && gwcurrent_topics != NULL){
        for(int i = 0; i < gwcurrent_topics_length; i++){
            if(gwcurrent_topics[i] != NULL){
                free(gwcurrent_topics[i]);
            }
        }
        free(gwcurrent_topics);
    }
    gwcurrent_topics = malloc(sizeof(char*) * actions_len);
    pthread_mutex_lock(&gwtopics_mtx);
    for(int i = 0; i < actions_len; i++){
        gwcurrent_topics[i] = routes_getTopicFromAction(actions[i]);
    }
    gwcurrent_topics_length = actions_len;
    pthread_mutex_unlock(&gwtopics_mtx);

    gwcheck_initialized();

    //subscribe all topics
    subscribe_current_topics();
    
    pthread_t thid;
    pthread_create(&thid, NULL, (void*(*)(void*))&gwmqtt_loop, NULL);
    printf("Created thread\n");
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
    pthread_mutex_lock(&gwtopics_mtx);
    if(gwcurrent_topics_length > 0 && gwcurrent_topics != NULL){
        for(int i = 0; i < gwcurrent_topics_length; i++){
            if(gwcurrent_topics[i] != NULL){
                free(gwcurrent_topics[i]);
            }
        }
        free(gwcurrent_topics);
    }
    gwcurrent_topics = malloc(sizeof(char*) * new_actions_len);
    for(int i = 0; i < new_actions_len; i++){
        char *temp = new_actions[i];
        gwcurrent_topics[i] = (is_action?routes_getTopicFromAction(temp):temp);
    }
    gwcurrent_topics_length = new_actions_len;
    pthread_mutex_unlock(&gwtopics_mtx);
    printf("Subscribing current topics\n");
    subscribe_current_topics();
    changing_topics = false;
}

void subscribe_current_topics(){
    pthread_mutex_lock(&gwtopics_mtx);
    for(int i = 0; i < gwcurrent_topics_length; i++){
        int rc = mosquitto_subscribe(gwmosq, NULL, gwcurrent_topics[i], 0);
        if(rc != MOSQ_ERR_SUCCESS){
            printf("Failed to SUBSCRIBE with mosquitto\n");
            exit(-1);
        }
    }
    pthread_mutex_unlock(&gwtopics_mtx);
}

void unsubscribe_current_topics(){
    printf("Unsubscribing current");
    int *res;
    pthread_mutex_lock(&gwtopics_mtx);
    for(int i = 0; i < gwcurrent_topics_length; i++){
        printf("%s\n", gwcurrent_topics[i]);
        mosquitto_unsubscribe(gwmosq, res, gwcurrent_topics[i]);
    }
    pthread_mutex_unlock(&gwtopics_mtx);
}

void sleep_till_stop_listening(){
    pthread_mutex_t wait_mtx = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&wait_mtx);
    pthread_cond_wait(&gwrun_cond, &wait_mtx);
    pthread_mutex_unlock(&wait_mtx);
}

void gwmqtt_loop(){
    printf("starting loop\n");
    mosquitto_loop_start(gwmosq);

    pthread_mutex_t wait_mtx = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&wait_mtx);
    pthread_cond_wait(&gwrun_cond, &wait_mtx);
    pthread_mutex_unlock(&wait_mtx);

    mosquitto_loop_stop(gwmosq, true);

    unsubscribe_current_topics();

    mosquitto_disconnect(gwmosq);
    mosquitto_destroy(gwmosq);
    pthread_exit((void*)0);
}

void mqtt_stop_listen() {
    pthread_mutex_lock(&gwrun_mtx);
    run = 0;
    pthread_cond_signal(&gwrun_cond);
    pthread_mutex_unlock(&gwrun_mtx);
}


bool mqtt_publish_action(char* message, char* action, int qos){
    return mqtt_publish(message, routes_getTopicFromAction(action), qos);
}

bool mqtt_publish(char* message, char* topic, int qos)
{
    gwcheck_initialized();
    printf("Sending message...\n");
    int rc;
    printf("topic %s\n", topic);
    rc = mosquitto_publish(gwmosq, NULL, topic,
        strlen(message), message, qos, false);
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
        printf("Message sent\n");
        return true;
    }
    return false;
}

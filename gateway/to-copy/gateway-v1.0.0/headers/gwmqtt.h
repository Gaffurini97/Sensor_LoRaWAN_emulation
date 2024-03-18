#ifndef ENMQTT_H
#define ENMQTT_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mosquitto.h>
#include <signal.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "routes/routes_manag.h"

#define TOPICS_STR_MAX_LEN 100

typedef struct mqtt_info{
    char* mqtt_host_name;
    int mqtt_port;
    char* mqtt_client_id;
    char* mqtt_username;
    char* mqtt_password;
} ClientMQTTInfo;

static ClientMQTTInfo *info;

void initializeMQTT(char*,int,char*,char*,char*);
void freeMQTT();

void mqtt_listen(char**,int,void(*)(char*,void*,int));
void mqtt_stop_listen();
void sleep_till_stop_listening();
bool mqtt_publish(char*,char*,int);
bool mqtt_publish_action(char*,char*,int);
void change_actORtopics(char**,int,bool);
void change_actions(char**,int);
void change_topics(char**,int);
void subscribe_current_topics();
void unsubscribe_current_topics();
#endif
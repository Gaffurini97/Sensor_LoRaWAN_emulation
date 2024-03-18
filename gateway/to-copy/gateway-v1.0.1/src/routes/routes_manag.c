#include "../../headers/routes/routes_manag.h"


void routes_Initialize(){
    key_topic_map_max_size = 100;
    key_topic_map = malloc(sizeof(APMap*) * key_topic_map_max_size);
    for(int i = 0; i < key_topic_map_max_size; i++){
        key_topic_map[i] = NULL;
    }
    if(key_topic_map == NULL){
        printf("NULLO\n");
        exit(-1);
    }
}

void routes_load_key_topic_map(char *action, char* topic){
    map_addElement(action, topic, key_topic_map, key_topic_map_max_size);
}

char* routes_getTopicFromAction(char* action){
    APMap * elem = map_lookup(action, key_topic_map, key_topic_map_max_size);
    if(elem == NULL){
        printf("trovato niente\n");
        return NULL;
    }

    return elem->value;
}

void routes_free(){
    for(int i = 0; i < key_topic_map_max_size; i++){
        APMap *elem = key_topic_map[i];
        if(elem != NULL){
            free(elem);
            while((elem = elem->next) != NULL){
                free(elem);
            }
        }
    }
}
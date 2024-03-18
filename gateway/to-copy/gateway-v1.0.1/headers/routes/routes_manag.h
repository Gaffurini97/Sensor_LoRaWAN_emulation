#ifndef ROUTESMANAG_H
#define ROUTESMANAG_H
#include "../commons/hash_map_manager.h"
#include "../commons/actions_const.h"

static APMap **key_topic_map;
static int key_topic_map_max_size;

void routes_Initialize();
void routes_load_key_topic_map(char*,char*);


char* routes_getTopicFromAction(char*);

void routes_free();

#endif
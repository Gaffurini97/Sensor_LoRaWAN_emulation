#include "../../headers/commons/hash_map_manager.h"

int map_hash(char *key, int size);

APMap *map_lookup(char *key, APMap **_map, int map_size){
    APMap *elem = _map[map_hash(key, map_size)];
    while(elem != NULL){
        fflush(stdout);
        if(strcmp(key, elem->key) == 0){
            return elem;
        }
        elem = elem->next;
    }
    return NULL;
}

APMap *map_addElement(char *key, char *value, APMap **_map, int map_size){
    APMap *elem = map_lookup(key, _map, map_size);
    if(elem != NULL){
        printf("ERROR: element already exists\n");
        exit(-1);
    }
    elem = malloc(sizeof(APMap));
    if(elem == NULL){
        printf("MALLOC FAILED\n");
        return NULL;
    }
    elem->key = key;
    elem->value = value;
    elem->next = NULL;
    int hash_index = map_hash(key, map_size);
    printf("added %s %s in index %d\n", key , value, hash_index);
    if(_map[hash_index] == NULL){
        _map[hash_index] = elem;
    }else{
        _map[hash_index]->next = elem;
    }
    return elem;
}

int map_hash(char *key, int size){
    int h = 0, i = 0, len = strlen(key);
    while(i < len){
        h = 3 * h + key[i];
        i++;
    }
    return h % size;
}
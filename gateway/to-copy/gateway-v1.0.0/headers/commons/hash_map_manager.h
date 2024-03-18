#ifndef HASHMAPMANAG_H
#define HASHMAPMANAG_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/*
FROM https://stackoverflow.com/questions/4384359/quick-way-to-implement-dictionary-in-c
*/

#define APMap_SIZE 100

typedef struct routing_map{
    char *key;
    char *value;
    struct routing_map *next;
} APMap;

APMap *map_lookup(char*,APMap**,int);
APMap *map_addElement(char*, char*,APMap**,int);
#endif
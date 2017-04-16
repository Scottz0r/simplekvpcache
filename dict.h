#ifndef DICT_H_
#define DICT_H_

#include <stddef.h>

#define DEFAULT_SIZE 5
#define GROWTH_FACTOR 2
#define MAX_LOAD_FACTOR 0.9
#define HASH_MULTIPLIER 37

typedef struct kvp{
    struct kvp* nextValue;
    char* key;
    char* value;
} kvp;

typedef struct dict{
    size_t size;
    size_t count;
    kvp** table;
} dict;


dict* dictCreate(void);
void dictDestroy(dict* d);
void dictInsert(dict* d, const char* key, const char* value);
const char* dictSearch(dict* d, const char *key);
void dictDelete(dict* d, const char *key);

#endif

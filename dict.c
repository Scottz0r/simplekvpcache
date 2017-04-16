#include "dict.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static uint64_t _hashString(const char* value);
static kvp* _internalSearch(dict* d, const char* key, uint64_t hashIdx);
static void _internalInsert(dict* d, const char* key, const char* value, uint64_t hashIdx);
static void _grow(dict* d, size_t newSize);

dict* dictCreate(void){
    dict* d;
    d = malloc(sizeof(dict));

    d->size = DEFAULT_SIZE;
    d->count = 0;
    d->table = malloc(sizeof(kvp*) * d->size);

    for(size_t idx = 0; idx < d->size; idx++){
        d->table[idx] = 0;
    }

    return d;
}

void dictDestroy(dict* d){
    kvp* ele;
    kvp* next;
    for(size_t idx = 0; idx < d->size; idx++){
        for(ele = d->table[idx]; ele; ele = next){
            next = ele->nextValue;
            free(ele->key);
            free(ele->value);
            free(ele);
        }
    }
}

void dictInsert(dict* d, const char* key, const char* value){
    uint64_t hashIdx;
    hashIdx = _hashString(key) % d->size;

    // Check for existing, if existing, replace data.
    kvp* existing = _internalSearch(d, key, hashIdx);
    if(existing){
        free(existing->value);
        existing->value = strdup(value);
    } else {
        _internalInsert(d, key, value, hashIdx);
    }
}

const char* dictSearch(dict* d, const char *key){
    uint64_t hashIdx;
    hashIdx = _hashString(key) % d->size;
    kvp* search = _internalSearch(d, key, hashIdx);
    if(search){
        return search->value;
    } else {
        return 0;
    }
}

void dictDelete(dict* d, const char *key){
    uint64_t hashIdx;
    kvp* element;
    kvp** previous;

    hashIdx = _hashString(key) % d->size;
    for(previous = &(d->table[hashIdx]); *previous; previous = &((*previous)->nextValue)){
        if(strcmp((*previous)->key, key) == 0){
            element = *previous;
            *previous = (element->nextValue);

            free(element->key);
            free(element->value);
            free(element);

            return;
        }
    }

}

static uint64_t _hashString(const char* value){
    uint64_t hash = 0;
    unsigned const char* uValue;

    for(uValue = (unsigned const char*) value; *uValue > 0; uValue++){
        hash = hash * HASH_MULTIPLIER + *uValue;
    }

    return hash;
}

static kvp* _internalSearch(dict* d, const char* key, uint64_t hashIdx){
    kvp* element;

    for(element = d->table[hashIdx]; element; element = element->nextValue){
        if(strcmp(element->key, key) == 0){
            return element;
        }
    }

    return 0;
}

static void _internalInsert(dict* d, const char* key, const char* value, uint64_t hashIdx){
    kvp* element;

    element = malloc(sizeof(kvp));
    element->key = strdup(key);
    element->value = strdup(value);

    //Any existing element at the hash index will be pushed next in the linked list.
    element->nextValue = d->table[hashIdx];
    d->table[hashIdx] = element;
    d->count++;

    //Grow dictionary if needed
    if(d->count >= d->size * MAX_LOAD_FACTOR){
        _grow(d, d->size * GROWTH_FACTOR);
    }
}

static void _grow(dict* d, size_t newSize){
    kvp** newTable;
    kvp* element;
    newTable = malloc(sizeof(kvp*) * newSize);

    for(size_t idx = 0; idx < newSize; idx++){
        newTable[idx] = 0;
    }

    for(size_t idx = 0; idx < d->size; idx++){
        if(d->table[idx]){
            element = d->table[idx];
            uint64_t newHashIdx = _hashString(element->key) % newSize;

            //If there is a hash collision, then push the elements into the linked list of the last
            //element at the collision index.
            if(newTable[newHashIdx]){
                for(kvp* existingEle = newTable[newHashIdx]; existingEle; existingEle = existingEle->nextValue){
                    if(!element->nextValue){
                        existingEle->nextValue = element;
                    }
                }
            } else {
                newTable[newHashIdx] = element;
            }
        }
    }

    free(d->table);
    d->table = newTable;
    d->size = newSize;
}

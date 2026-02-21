#include "hashmap.h"

typedef struct Hashmap {

    Node **buckets;
    size_t numBuckets;
    size_t (*hashFunction)(const char *key, size_t keySize); 
    //Fptr does have overhead, but this is negligible next to the LL traversal cost 

    //Usually good practise to include a destructor fptr for elements
    //This map is for a database and only intended to store strings however, so we can just free()
} Hashmap;

typedef struct Node {

    struct Node *next;
    char *key, *value;
    size_t keySize, valueSize; 
    //Storing keySize allows for size check before strcmp() (recalculates string length every time)

} Node;



/* hashmap_init */
/* Initialise a hashmap for use */
bool hashmap_init(Hashmap *map, const size_t size, const size_t (*hashFunction)(const char *key, size_t keySize)) {
    map->hashFunction = hashFunction;
    map->numBuckets = size;
    map->buckets = calloc(size, sizeof(Node*));
    if(!map->buckets) {
        return false;
    }

    return true;
}


/* hashmap_destroy */
/* Destroy a hashmap and associated memory */
void hashmap_destroy(Hashmap *map) {

    for(size_t i = 0; i < map->numBuckets; i++) {
        Node *node = map->buckets[i]; 

        while(node) {
            Node *next = node->next;

            free(node->key);
            free(node->value);
            free(node);
            node = next;
        } 
    }
    free(map->buckets);

    map->buckets = NULL;
    map->numBuckets = 0;

    return;
}



/* hashmap_insert */
/* Insert an item to a hashmap */
const char *hashmap_insert(Hashmap *map, const char *key, const size_t keySize, const char *value, const size_t valueSize) {

    //Try allocation first - so if it fails we can exit early
    Node *newNode = malloc(sizeof(Node));
    char *newKey = malloc(keySize);
    char *newValue = malloc(valueSize);
    if(!newNode || !newKey || !newValue) {
        if(newNode) free(newNode);
        if(newKey) free(newKey);
        if(newValue) free(newValue);
        return NULL;
    }

    size_t bucketIndex = map->hashFunction(key, keySize) % map->numBuckets;

    newNode->next = map->buckets[bucketIndex];
    newNode->key = newKey;
    newNode->value = newValue;
    newNode->keySize = keySize;
    newNode->valueSize = valueSize;
    memcpy(newNode->key, key, keySize);
    memcpy(newNode->value, value, valueSize);

    map->buckets[bucketIndex] = newNode;

    return newValue;
}



/* hashmap_find */
/* Find an item in a hashmap (if present) */
const char *hashmap_find(const Hashmap *map, const char *key, const size_t keySize) { 
    //Note - returns a char* since it should be NULL terminated

    size_t bucketIndex = map->hashFunction(key, keySize) % map->numBuckets;

    Node *node = map->buckets[bucketIndex]; 
    while(node) {
        if(node->keySize == keySize) {
            if(!memcmp(node->key, key, keySize)) {
                return node->value;
            }
        }

        node = node->next;
    } 
    return NULL;
}



/* hashmap_delete */
/* Delete an item from a hashmap */
bool hashmap_delete(Hashmap *map, const char *key, const size_t keySize) {

    size_t bucketIndex = map->hashFunction(key, keySize) % map->numBuckets;

    Node *node = map->buckets[bucketIndex]; 
    Node *prev = NULL;
    while(node) {
        if(node->keySize == keySize) {
            if(!memcmp(node->key, key, keySize)) {

                //Could use double pointers, but its slower since it requires an additional dereference
                //Instead its best to just have a second few if statements in here

                if(prev) { 
                    prev->next = node->next;
                } else {
                    map->buckets[bucketIndex] = node->next;
                }

                free(node->key);
                free(node->value);
                free(node);

                return true;
            }
        }

        prev = node;
        node = node->next;
    } 

    return false;
}


/* hashmap_rehash */
/* Reallocate and rehash an entire map */
bool hashmap_rehash(Hashmap *map, const size_t newSize, const size_t (*hashFunction)(const char *key, size_t keySize)) {


    Node **newBuckets = calloc(newSize, sizeof(Node*));
    //Thought about using realloc here, but realloc allocates a second buffer before freeing the first anyway
    //So essentially realloc-ing then rehashing in place would use the same peak memory (maybe for less time though)
    if(!newBuckets) {
        return false;
    }


    if(hashFunction) { //Optional hash function size change
        map->hashFunction = hashFunction;
    }

    for(size_t i = 0; i < map->numBuckets; i++) {
        Node *bucket = map->buckets[i];

        while(bucket) {

            size_t bucketIndex = map->hashFunction(bucket->key, bucket->keySize) % newSize;

            Node *next = bucket->next;
            bucket->next = newBuckets[bucketIndex];
            newBuckets[bucketIndex] = bucket;

            bucket = next;
        }
    }
    free(map->buckets);
    map->buckets = newBuckets;
    map->numBuckets = newSize;

    return true;
}




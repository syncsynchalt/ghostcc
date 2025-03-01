#pragma once

#include <stdlib.h>

typedef struct {
    const char *key;
    void *data;
} hashmap_entry;

typedef struct _hashmap {
    size_t num_buckets;
    hashmap_entry **buckets;
    size_t *bucket_cur;
    size_t *bucket_max;
} hashmap;

typedef struct _hashmap_iter_state {
    int b;
    int i;
} hashmap_iter_state;

extern hashmap *hashmap_init(size_t buckets);
extern void hashmap_destroy(hashmap *h);
extern const void *hashmap_add(const hashmap *h, const char *key, void *data);
extern void *hashmap_get(const hashmap *h, const char *key);
extern void *hashmap_delete(const hashmap *h, const char *key);
extern hashmap_entry *hashmap_iter(const hashmap *h, hashmap_iter_state *iter);

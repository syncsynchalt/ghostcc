#include "hashmap.h"
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "die.h"

// djb2 hash function
static unsigned int hash_function(const char *k, const unsigned int mask)
{
    unsigned int hash = 5381;
    unsigned int c;

    while ((c = (unsigned char)*(k++))) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }

    return hash & mask;
}

hashmap *hashmap_init(const size_t buckets)
{
    hashmap *h;

    if (!buckets || (ceil(log2((double)buckets)) != floor(log2((double)buckets)))) {
        die("hashmap buckets must be positive power of two\n");
    }

    h = malloc(sizeof(*h));
    h->num_buckets = buckets;
    h->buckets = calloc(buckets, sizeof(hashmap_entry *));
    h->bucket_cur = calloc(buckets, sizeof(size_t));
    h->bucket_max = calloc(buckets, sizeof(size_t));
    return h;
}

void hashmap_destroy(hashmap *h)
{
    int i, b;
    for (b = 0; b < h->num_buckets; b++) {
        for (i = 0; i < h->bucket_cur[b]; i++) {
            free((char *)h->buckets[b][i].key);
        }
    }
    free(h->buckets);
    free(h->bucket_cur);
    free(h->bucket_max);
    free(h);
}

static hashmap_entry *get(const hashmap *h, const char *key)
{
    unsigned int b;
    hashmap_entry *e;
    int i;

    b = hash_function(key, h->num_buckets-1);
    for (i = 0; i < h->bucket_cur[b]; i++) {
        e = &h->buckets[b][i];
        if (e->key && strcmp(e->key, key) == 0) {
            return e;
        }
    }
    return NULL;
}

const void *hashmap_add(const hashmap *h, const char *key, void *data)
{
    unsigned int b;
    hashmap_entry *e;

    e = get(h, key);
    if (e) {
        // update
        const void *existing = e->data;
        e->data = data;
        return existing;
    }

    b = hash_function(key, h->num_buckets-1);
    if (h->bucket_cur[b] == h->bucket_max[b]) {
        h->bucket_max[b] += 5;
        h->buckets[b] = realloc(h->buckets[b], h->bucket_max[b] * sizeof(hashmap_entry));
    }
    e = &h->buckets[b][h->bucket_cur[b]];
    e->key = strdup(key);
    e->data = data;
    h->bucket_cur[b]++;
    return NULL;
}

void *hashmap_get(const hashmap *h, const char *key)
{
    if (!h) {
        return NULL;
    }
    const hashmap_entry *e = get(h, key);
    return e ? e->data : NULL;
}

hashmap_entry *hashmap_iter(const hashmap *h, hashmap_iter_state *iter)
{
    while (iter->b < h->num_buckets) {
        if (iter->i < h->bucket_cur[iter->b]) {
            return &h->buckets[iter->b][iter->i++];
        } else {
            iter->i = 0;
            iter->b++;
        }
    }
    return NULL;
}

void *hashmap_delete(const hashmap *h, const char *key)
{
    hashmap_entry *e = get(h, key);
    if (e) {
        free((char *)e->key);
        e->key = NULL;
        return e->data;
    }
    return NULL;
}

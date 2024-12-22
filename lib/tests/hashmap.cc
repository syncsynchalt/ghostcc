#include <gtest/gtest.h>
extern "C" {
#include "../hashmap.h"
}

TEST(HashMapTest, InitDestroy)
{
    hashmap *h = hashmap_init(128);
    hashmap_destroy(h);
}

TEST(HashMapTest, PowerOfTwo)
{
    EXPECT_DEATH({ hashmap_init(3); }, "positive power of two");
}

TEST(HashMapTest, AddGet)
{
    hashmap *h = hashmap_init(128);
    const char *payload = "p";

    hashmap_add(h, "foo", payload);
    hashmap_add(h, "bar", payload);
    EXPECT_EQ(payload, hashmap_get(h, "foo"));
    EXPECT_EQ(payload, hashmap_get(h, "bar"));
    EXPECT_EQ(NULL, hashmap_get(h, "baz"));
    hashmap_destroy(h);
}

TEST(HashMapTest, Overwrite)
{
    hashmap *h = hashmap_init(128);
    const char *p1 = "p";
    const char *p2 = "o";

    EXPECT_EQ(NULL, hashmap_add(h, "foo", p1));
    EXPECT_EQ(p1, hashmap_add(h, "foo", p2));
    EXPECT_EQ(p2, hashmap_get(h, "foo"));
    hashmap_destroy(h);
}

TEST(HashMapTest, Resize)
{
    hashmap *h = hashmap_init(1);
    const char *p = "0123456789";

    hashmap_add(h, "foo1", p + 1);
    hashmap_add(h, "foo2", p + 2);
    hashmap_add(h, "foo3", p + 3);
    hashmap_add(h, "foo4", p + 4);
    hashmap_add(h, "foo5", p + 5);
    hashmap_add(h, "foo6", p + 6);
    hashmap_add(h, "foo7", p + 7);
    EXPECT_EQ('1', *(char *) hashmap_get(h, "foo1"));
    EXPECT_EQ('3', *(char *) hashmap_get(h, "foo3"));
    EXPECT_EQ('6', *(char *) hashmap_get(h, "foo6"));
    EXPECT_EQ('7', *(char *) hashmap_get(h, "foo7"));
    hashmap_destroy(h);
}

TEST(HashMapTest, Iterate)
{
    int found[100] = {0};
    const char arr[100] = "";

    hashmap *h = hashmap_init(32);
    for (int i = 0; i < 100; i++) {
        char key[10];
        snprintf(key, sizeof(key), "entry_%d", i);
        hashmap_add(h, key, arr + i);
    }
    const hashmap_entry *e;
    hashmap_iter_state iter = {0};
    while ((e = hashmap_iter(h, &iter))) {
        const int offset = static_cast<const char *>(e->data) - arr;
        found[offset]++;
    }

    for (int i: found) {
        EXPECT_EQ(1, i);
    }

    hashmap_destroy(h);
}

TEST(HashMapTest, Delete)
{
    const auto payload = "abc";
    const hashmap *h = hashmap_init(64);
    hashmap_add(h, "foo", payload);
    EXPECT_EQ(payload, (char *) hashmap_get(h, "foo"));
    EXPECT_EQ(payload, (char *) hashmap_delete(h, "foo"));
    EXPECT_EQ(NULL, (char *) hashmap_get(h, "foo"));
}

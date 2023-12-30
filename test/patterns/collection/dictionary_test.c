#include "dictionary.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Test {
    int value;
};

typedef struct Test Test;

void print(Map **head) {
    for (Map *cursor = *head; cursor; cursor = cursor->next) {
        puts(cursor->key);
    }
}

void testContainsKey() {
    Dictionary *dictionary = puremvc_dictionary_new();

    Test *test1 = &(Test) {1};
    Test *test2 = &(Test) {2};
    Test *test3 = &(Test) {3};

    dictionary->put(dictionary, "key1", test1);
    dictionary->put(dictionary, "key2", test2);
    dictionary->put(dictionary, "key3", test3);

    assert(dictionary->containsKey(dictionary, "key1") == true);
    assert(dictionary->containsKey(dictionary, "key2") == true);
    assert(dictionary->containsKey(dictionary, "key3") == true);
    assert(dictionary->containsKey(dictionary, "invalid") == false);
}

void testPut() {
    Dictionary *dictionary = puremvc_dictionary_new();

    Test *test1 = &(Test) {1};
    Test *test2 = &(Test) {2};
    Test *test3 = &(Test) {3};

    assert(dictionary->put(dictionary, "key1", test1) == test1);
    assert(dictionary->put(dictionary, "key2", test2) == test2);
    assert(dictionary->put(dictionary, "key3", test3) == test3);

    assert(strcmp(dictionary->map->key, "key1") == 0);
    assert(dictionary->map->value == test1);
    assert(strcmp(dictionary->map->next->key, "key2") == 0);
    assert(dictionary->map->next->value == test2);
    assert(strcmp(dictionary->map->next->next->key, "key3") == 0);
    assert(dictionary->map->next->next->value == test3);
}

void testGet() {
    Dictionary *dictionary = puremvc_dictionary_new();
    assert(dictionary->get(dictionary, "unknown") == NULL);

    Test *test1 = &(Test) {1};
    Test *test2 = &(Test) { 2};
    Test *test3 = &(Test) {3};

    assert(dictionary->put(dictionary, "key1", test1) == test1);
    assert(dictionary->put(dictionary, "key2", test2) == test2);
    assert(dictionary->put(dictionary, "key3", test3) == test3);

    assert(dictionary->get(dictionary, "key1") == test1);
    assert(dictionary->get(dictionary, "key2") == test2);
    assert(dictionary->get(dictionary, "key3") == test3);
    assert(dictionary->get(dictionary, "unknown") == NULL);
}

void testReplace() {
    Dictionary *dictionary = puremvc_dictionary_new();

    Test *test1 = &(Test) {1};
    Test *test2 = &(Test) { 2};
    Test *test3 = &(Test) {3};

    assert(dictionary->put(dictionary, "key1", test1) == test1);
    assert(dictionary->put(dictionary, "key2", test2) == test2);
    assert(dictionary->put(dictionary, "key3", test3) == test3);

    Test *test4 = &(Test) {4};

    Test *oldValue = (Test *) (dictionary->replace(dictionary, "key3", test4));

    assert(oldValue == test3);
    assert(dictionary->containsKey(dictionary, "key3") == true);

    assert(dictionary->get(dictionary, "key3") == test4);
    assert(&test3 != NULL);
}

void testRemove() {
    Dictionary *dictionary = puremvc_dictionary_new();

    Test *test1 = &(Test) {1};
    Test *test2 = &(Test) { 2};
    Test *test3 = &(Test) {3};

    assert(dictionary->put(dictionary, "key1", test1) == test1);
    assert(dictionary->put(dictionary, "key2", test2) == test2);
    assert(dictionary->put(dictionary, "key3", test3) == test3);

    assert(dictionary->removeItem(dictionary, "key2") == test2);
    assert(dictionary->removeItem(dictionary, "key1") == test1);
    assert(dictionary->removeItem(dictionary, "key3") == test3);
}

void testDealloc() {
    Dictionary *dictionary = puremvc_dictionary_new();

    Test *test1 = malloc(sizeof(Test));
    test1->value = 1;
    Test *test2 = malloc(sizeof(Test));
    test2->value = 2;
    Test *test3 = malloc(sizeof(Test));
    test3->value = 3;

    assert(dictionary->put(dictionary, "key1", test1) == test1);
    assert(dictionary->put(dictionary, "key2", test2) == test2);
    assert(dictionary->put(dictionary, "key3", test3) == test3);

    // Clear the dictionary first if you have dynamically allocated memory items in it
    dictionary->clear(dictionary, free); // can pass NULL as the callback for static values

    puremvc_dictionary_free(&dictionary);
    assert(dictionary == NULL);
}

int main() {
    testContainsKey();
    testPut();
    testGet();
    testReplace();
    testRemove();
    testDealloc();
    return 0;
}

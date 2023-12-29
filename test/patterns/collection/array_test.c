#include "../../../include/array.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct Test Test;

struct Test {
    char *name;
    Test *next;
};

void print(List **head) {
    for (List *cursor = *head; cursor; cursor = cursor->next) {
        printf("%s\n", ((Test *) (cursor->item))->name);
    }
}

void testAddFirst() {
    Array *array = puremvc_array_new();

    print(&(array->list));
    print(&array->list);

    Test *test1 = &(Test) {"name1"};
    Test *test2 = &(Test) {"name2"};
    Test *test3 = &(Test) {"name3"};

    assert(test1->next == NULL);
    assert(test2->next == NULL);
    assert(test3->next == NULL);

    assert(array->unshift(array, test1) == test1);
    assert(array->unshift(array, test2) == test2);
    assert(array->unshift(array, test3) == test3);

    assert(array->list->item == test3);
    assert(array->list->next->item == test2);
    assert(array->list->next->next->item == test1);
}

void testAddLast() {
    Array *array = puremvc_array_new();

    Test *test1 = &(Test) {"name1"};
    Test *test2 = &(Test) {"name2"};
    Test *test3 = &(Test) {"name3"};

    assert(array->push(array, test1) == test1);
    assert(array->push(array, test2) == test2);
    assert(array->push(array, test3) == test3);

    assert(array->list->item == test1);
    assert(array->list->next->item == test2);
    assert(array->list->next->next->item == test3);
}

void testRemoveFirst() {
    Array *array = puremvc_array_new();

    assert(array->shift(array) == NULL);

    Test *test1 = &(Test) {"name1"};
    Test *test2 = &(Test) {"name2"};
    Test *test3 = &(Test) {"name3"};

    array->push(array, test1);
    array->push(array, test2);
    array->push(array, test3);

    assert(array->shift(array) == test1);
    assert(array->shift(array) == test2);
    assert(array->shift(array) == test3);
    assert(array->shift(array) == NULL);
}

void testRemoveLast() {
    Array *array = puremvc_array_new();

    assert(array->pop(array) == NULL);

    Test *test1 = &(Test) {"name1"};
    Test *test2 = &(Test) {"name2"};
    Test *test3 = &(Test) {"name3"};

    array->push(array, test1);
    array->push(array, test2);
    array->push(array, test3);

    assert(array->pop(array) == test3);
    assert(array->pop(array) == test2);
    assert(array->pop(array) == test1);
    assert(array->pop(array) == NULL);
}

void testRemove() {
    Array *array = puremvc_array_new();

    assert(array->removeItem(array, NULL) == NULL);
    assert(array->removeItem(array, &(Test) {"name4"}) == NULL);

    Test *test1 = &(Test) {"name"};
    Test *test2 = &(Test) {"name"};
    Test *test3 = &(Test) {"name"};

    array->push(array, test1);
    array->push(array, test2);
    array->push(array, test3);

    assert(array->removeItem(array, test2) == test2);
    assert(array->removeItem(array, test1) == test1);
    assert(array->removeItem(array, test3) == test3);
    assert(array->removeItem(array, &(Test) {"name4"}) == NULL);
}

void testContainsValue() {
    Array *array = puremvc_array_new();

    assert(array->containsValue(array, NULL) == false);
    assert(array->containsValue(array, "emptyList") == false);

    Test *test1 = &(Test) {"name1"};
    Test *test2 = &(Test) {"name2"};
    Test *test3 = &(Test) {"name3"};

    array->push(array, test1);
    array->push(array, test2);
    array->push(array, test3);

    assert(array->containsValue(array, test1) == true);
    assert(array->containsValue(array, test2) == true);
    assert(array->containsValue(array, test3) == true);
    assert(array->containsValue(array, &(Test) {"name4"}) == false);

    array->removeItem(array, test2);
    assert(array->containsValue(array, test2) == false);

    array->removeItem(array, test3);
    assert(array->containsValue(array, test3) == false);

    array->removeItem(array, test1);
    assert(array->containsValue(array, test1) == false);
}

void testClone() {
    Array *array = puremvc_array_new();

    assert(array->clone(array)->list == NULL);

    Test *test1 = &(Test) {"name1"};
    Test *test2 = &(Test) {"name2"};
    Test *test3 = &(Test) {"name3"};

    array->push(array, test1);
    array->push(array, test2);
    array->push(array, test3);

    assert(array->list->item == test1);
    assert(array->list->next->item == test2);
    assert(array->list->next->next->item == test3);
    assert(array->list->next->next->next == NULL);

    Array *clone = array->clone(array);

    array->clear(array, NULL);
    assert(array->size(array) == 0);

    assert(clone->list->item == test1);
    assert(clone->list->next->item == test2);
    assert(clone->list->next->next->item == test3);
    assert(clone->list->next->next->next == NULL);
}

void testSize() {
    Array *array = puremvc_array_new();

    Test *test1 = &(Test) {"name1"};
    Test *test2 = &(Test) {"name2"};
    Test *test3 = &(Test) {"name3"};

    array->push(array, test1);
    assert(array->size(array) == 1);

    array->push(array, test2);
    assert(array->size(array) == 2);

    array->push(array, test3);
    assert(array->size(array) == 3);

    array->removeItem(array, test2);
    assert(array->size(array) == 2);

    array->removeItem(array, test1);
    array->removeItem(array, test2);
    assert(array->size(array) == 1);

    array->removeItem(array, test3);
    assert(array->size(array) == 0);
}

void testClear() {
    Array *array = puremvc_array_new();

    array->clear(array, NULL);
    assert(array->list == NULL);

    Test *test1 = &(Test) {"name1"};
    Test *test2 = &(Test) {"name2"};
    Test *test3 = &(Test) {"name3"};

    array->push(array, test1);
    array->push(array, test2);
    array->push(array, test3);

    array->clear(array, NULL);
    assert(array->size(array) == 0);
    assert(array->list == NULL);
}

void testDealloc() {
    Array *array = puremvc_array_new();

    Test *test1 = malloc(sizeof(Test));
    test1->name = "name1";
    Test *test2 = malloc(sizeof(Test));
    test2->name = "name2";
    Test *test3 = malloc(sizeof(Test));
    test3->name = "name3";

    assert(array->push(array, test1) == test1);
    assert(array->push(array, test2) == test2);
    assert(array->push(array, test3) == test3);

    // Clear the array first if you have dynamically allocated memory items in it
    array->clear(array, free); // can pass NULL as the callback for static values

    puremvc_array_free(&array);
    assert(array == NULL);
}

int main() {
    testAddFirst();
    testAddLast();
    testRemoveFirst();
    testRemoveLast();
    testRemove();
    testContainsValue();
    testClone();
    testSize();
    testClear();
    testDealloc();
    return 0;
}


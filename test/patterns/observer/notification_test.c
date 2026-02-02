#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "notification_test.h"
#include "puremvc/notification.h"

int main() {
    testConstructor();
    testNameAccessors();
    testBodyAccessors();
    testTypeAccessors();
    testToString();
    return 0;
}

/**
 * Tests setting the name and body using the Notification class Constructor.
 */
void testConstructor() {
    // Create a new Notification using the Constructor to set the note name and body
    const struct Notification note = puremvc_notification("TestNote", NULL, NULL);
    const struct INotification *notification = &note.base;

    // test assertions
    assert(strcmp(notification->getName(notification), "TestNote") == 0);
    assert(notification->getBody(notification) == NULL);
    assert(strcmp(notification->getType(notification), "") == 0);
}

/**
 * Tests setting and getting the name using Notification class accessor methods.
 */
void testNameAccessors() {
    // Create a new Notification and use accessors to set the note name
    const struct Notification note = puremvc_notification("TestNote", NULL, NULL);
    const struct INotification *notification = &note.base;

    // test assertions
    assert(strcmp(notification->getName(notification), "TestNote") == 0);
}

/**
 * Tests setting and getting the body using Notification class accessor methods.
 */
void testBodyAccessors() {
    // Create a new Notification and use accessors to set the body
    struct { int value; } vo = {.value = 5};

    // Create a new Notification and use accessors to set the body
    struct Notification note = puremvc_notification("TestNote", NULL, NULL);
    struct INotification *notification = &note.base;
    notification->setBody(notification, &vo);

    // test assertions
    assert(notification->getBody(notification) == &vo);
}

/**
 * Tests setting and getting the type using Notification class accessor methods.
 */
void testTypeAccessors() {
    struct Notification note = puremvc_notification("TestNote", NULL, "TestNoteType");
    struct INotification *notification = &note.base;

    // test assertions
    assert(strcmp(notification->getType(notification), "TestNoteType") == 0);
    notification->setType(notification, "TestNoteType2");
    assert(strcmp(notification->getType(notification), "TestNoteType2") == 0);
}

/** Node Notifications */
void testToString() {
    struct { int value; } test = {.value = 5};

    const struct Notification note = puremvc_notification("TestNote", &test, "TestNoteType");
    const struct INotification *notification = &note.base;

    char buffer[256];
    notification->toString(notification, buffer, sizeof(buffer));
    printf("%s\n", buffer);

    const char *prefix = "TestNote : TestNoteType [body=";
    assert(strncmp(buffer, prefix, strlen(prefix)) == 0);
}

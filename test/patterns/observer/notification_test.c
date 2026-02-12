#include "notification_test.h"

#include "puremvc/i_notification.h"

#include <alloca.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void test(const char *name, void (*callback)(void)) {
    printf("\033[0;34m[RUNNING]\033[0m %s...\n", name);
    fflush(stdout);

    callback();

    printf("\033[0;32m[PASSED]\033[0m %s\n", name);
    fflush(stdout);
}

int main() {
    printf("\n\033[1;36m================================================\033[0m\n");
    printf("\033[1;36m[SUITE] %s\033[0m\n", "NotificationTest");
    printf("\033[1;36m================================================\033[0m\n\n");

    test("testConstructor", testConstructor);
    test("testNameAccessors", testNameAccessors);
    test("testBodyAccessors", testBodyAccessors);
    test("testTypeAccessors", testTypeAccessors);
    test("testToString", testToString);

    printf("\n\033[1;32m[DONE] All tests in suite finished.\033[0m\n");
    return 0;
}

/**
 * Tests setting the name and body using the Notification class Constructor.
 */
void testConstructor() {
    // Create a new Notification using the Constructor to set the note name and body
    const struct INotification *notification = puremvc_notification_init(alloca(puremvc_notification_size()), "TestNote", NULL, NULL);

    // test assertions
    if (strcmp(notification->getName(notification), "TestNote") != 0)
        abort();
    if (notification->getBody(notification) != NULL)
        abort();
    if (notification->getType(notification) != NULL)
        abort();
}

/**
 * Tests setting and getting the name using Notification class accessor methods.
 */
void testNameAccessors() {
    // Create a new Notification and use accessors to set the note name
    const struct INotification *notification = puremvc_notification_init(alloca(puremvc_notification_size()), "TestNote", NULL, NULL);

    // test assertions
    if (strcmp(notification->getName(notification), "TestNote") != 0)
        abort();
}

/**
 * Tests setting and getting the body using Notification class accessor methods.
 */
void testBodyAccessors() {
    // Create a new Notification and use accessors to set the body
    struct { int value; } vo = {.value = 5};

    // Create a new Notification and use accessors to set the body
    struct INotification *notification = puremvc_notification_init(alloca(puremvc_notification_size()), "TestNote", NULL, NULL);
    notification->setBody(notification, &vo);

    // test assertions
    if (notification->getBody(notification) != &vo)
        abort();
}

/**
 * Tests setting and getting the type using Notification class accessor methods.
 */
void testTypeAccessors() {
    struct INotification *notification = puremvc_notification_init(alloca(puremvc_notification_size()), "TestNote", NULL, "TestNoteType");

    // test assertions
    if (strcmp(notification->getType(notification), "TestNoteType") != 0)
        abort();
    notification->setType(notification, "TestType2");
    if (strcmp(notification->getType(notification), "TestType2") != 0)
        abort();
}

/** Node Notifications */
void testToString() {
    struct { int value; } test = {.value = 5};

    const struct INotification *notification = puremvc_notification_init(alloca(puremvc_notification_size()), "TestNote", NULL, "TestNoteType");

    char buffer[256];
    notification->toString(notification, buffer, sizeof(buffer));
    printf("%s\n", buffer);

    const char *prefix = "TestNote : TestNoteType [body=";
    if (strncmp(buffer, prefix, strlen(prefix)) != 0)
        abort();
}

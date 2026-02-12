#include "observer_test.h"

#include "puremvc/platform.h"
#include "puremvc/i_observer.h"
#include "puremvc/i_notification.h"

#include <stdio.h>
#include <stdlib.h>

static void beforeAll() {}
static void beforeEach() {}
static void afterEach() {}
static void afterAll() {}

static void test(const char *name, void (*callback)(void)) {
    printf("\033[0;34m[RUNNING]\033[0m %s...\n", name);
    fflush(stdout);

    beforeEach();
    callback();
    afterEach();

    printf("\033[0;32m[PASSED]\033[0m %s\n", name);
    fflush(stdout);
}

int main() {
    printf("\n\033[1;36m================================================\033[0m\n");
    printf("\033[1;36m[SUITE] %s\033[0m\n", "ObserverTest");
    printf("\033[1;36m================================================\033[0m\n\n");

    beforeAll();
    test("testObserverConstructor", testObserverConstructor);
    test("testObserverAccessors", testObserverAccessors);
    test("testCompareNotifyContext", testCompareNotifyContext);
    afterAll();

    printf("\n\033[1;32m[DONE] All tests in suite finished.\033[0m\n");
    return 0;
}

/**
 * A test variable that proves the notify method was
 * executed with 'handleNotification' as its execution context
 */
static struct ObserverTestVar observerTestVar = {0};

/**
 * A function that is used as the observer notification
 * method.
 */
static bool handleNotification(const void *context, const struct INotification *notification) {
    if (notification == NULL) return false;
    observerTestVar = *(struct ObserverTestVar *) notification->getBody(notification);
    return true;
}

/**
 * Tests the Constructor method of the Observer class
 */
void testObserverConstructor() {
    // Create observer
    struct Object { int x; } object = { 0 };
    const struct IObserver *observer = puremvc_observer_init(alloca(puremvc_observer_size()), handleNotification, &object);

    struct ObserverTestVar var = {.value = 5};
    struct INotification *notification = puremvc_notification_init(alloca(puremvc_notification_size()), "ObserverTestNote", &var, NULL);
    observer->notifyObserver(observer, notification);

    // test assertions
    if (observerTestVar.value != 5) abort();
    if (&object != observer->getContext(observer)) abort();
}

/**
 * Tests observer class when initialized by accessor methods.
 */
void testObserverAccessors() {
    // Create observer with null args, then
    // use accessors to set notification method and context
    struct Object {int x;} object;
    struct IObserver *observer = puremvc_observer_init(alloca(puremvc_observer_size()), NULL, NULL);

    observer->setContext(observer, &object);
    observer->setNotify(observer, handleNotification);

    if (observer->getContext(observer) != &object) abort();
    if (observer->getNotify(observer) != handleNotification) abort();

    // create a test event, setting a payload value and notify
    // the observer with it. since the observer is this class
    // and the notification method is observerTestMethod,
    // successful notification will result in our local
    // observerTestVar being set to the value we pass in
    // on the note body.
    struct ObserverTestVar vo = {.value = 10};
    struct INotification *notification = puremvc_notification_init(alloca(puremvc_notification_size()), "ObserverTestNote", &vo, NULL);
    observer->notifyObserver(observer, notification);
}

/**
 * Tests the compareNotifyContext method of the Observer class
 */
void testCompareNotifyContext() {
    // Create observer passing in notification method and context
    struct Object {char dummy;};
    struct Object object = {0};
    const struct Object negTestObj = {0};
    const struct IObserver *observer = puremvc_observer_init(alloca(puremvc_observer_size()), handleNotification, &object);

    // test assertions
    if (observer->compareNotifyContext(observer, &negTestObj) != false) abort();
    if (observer->compareNotifyContext(observer, &object) != true) abort();
}

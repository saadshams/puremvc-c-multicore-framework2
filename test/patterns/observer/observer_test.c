#include "notification.h"
#include "observer.h"
#include <assert.h>
#include <stdio.h>

typedef struct {
    int value;
} ObserverTestVar;

/**
 * A test variable that proves the notify method was
 * executed with 'handleNotification' as its execution context
 */
static ObserverTestVar observerTestVar = {0};

/**
 * A function that is used as the observer notification
 * method.
 */
static void handleNotification(void *context, struct Notification *notification) {
    observerTestVar = *(ObserverTestVar *) notification->getBody(notification);
}

/**
 * Tests the Constructor method of the Observer class
 */
void testObserverConstructor() {
    // Create observer
    struct Object {
    } object;
    struct Observer *observer = puremvc_observer_new(handleNotification, &object);
    struct Notification *notification = puremvc_notification_new("ObserverTestNote", &(ObserverTestVar) {5}, NULL);
    observer->notifyObserver(observer, notification);

    // test assertions
    assert(observerTestVar.value == 5);

    puremvc_notification_free(&notification);
    assert(notification == NULL);
    puremvc_observer_free(&observer);
    assert(observer == NULL);
}

/**
 * Tests observer class when initialized by accessor methods.
 */
void testObserverAccessors() {
    // Create observer with null args, then
    // use accessors to set notification method and context
    struct Object {} object;
    struct Observer *observer = puremvc_observer_new(NULL, NULL);
    observer->setNotifyContext(observer, &object);
    observer->setNotifyMethod(observer, handleNotification);

    assert(observer->getNotifyContext(observer) == &object);
    assert(observer->getNotifyMethod(observer) == handleNotification);

    // create a test event, setting a payload value and notify
    // the observer with it. since the observer is this class
    // and the notification method is observerTestMethod,
    // successful notification will result in our local
    // observerTestVar being set to the value we pass in
    // on the note body.
    struct Notification *notification = puremvc_notification_new("ObserverTestNote", &(ObserverTestVar) {10}, NULL);
    observer->notifyObserver(observer, notification);

    puremvc_notification_free(&notification);
    assert(notification == NULL);
    puremvc_observer_free(&observer);
    assert(observer == NULL);
}

/**
 * Tests the compareNotifyContext method of the Observer class
 */
void testCompareNotifyContext() {
    // Create observer passing in notification method and context
    struct Object {} object, negTestObj;
    struct Observer *observer = puremvc_observer_new(handleNotification, &object);

    // test assertions
    assert(observer->compareNotifyContext(observer, &negTestObj) == false);
    assert(observer->compareNotifyContext(observer, &object) == true);

    puremvc_observer_free(&observer);
    assert(observer == NULL);
}

/**
 * Tests Observer class destructor method.
 */
void testDestructor() {
    struct Observer *observer = puremvc_observer_new(handleNotification, NULL);

    assert(observer != NULL);
    puremvc_observer_free(&observer);
    assert (observer == NULL);
}

int main() {
    testObserverConstructor();
    testObserverAccessors();
    testCompareNotifyContext();
    testDestructor();
    return 0;
}

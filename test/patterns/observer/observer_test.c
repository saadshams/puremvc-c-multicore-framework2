#include <assert.h>

#include "observer_test.h"
#include "puremvc/observer.h"
#include "puremvc/notification.h"

int main() {
    testObserverConstructor();
    // testObserverAccessors();
    // testCompareNotifyContext();
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
static void handleNotification(const void *context, struct INotification *notification) {
    observerTestVar = *(struct ObserverTestVar *) notification->getBody(notification);
}

/**
 * Tests the Constructor method of the Observer class
 */
void testObserverConstructor() {
    // Create observer
    struct Object { int x; } object = { 0 };
    struct IObserver *observer = puremvc_observer(&(struct Observer){0}, handleNotification, &object);

    struct ObserverTestVar var = {.value = 5};
    struct INotification *notification = puremvc_notification(&(struct Notification){0}, "ObserverTestNote", &var, NULL);
    observer->notifyObserver(observer, notification);

    // test assertions
    assert(observerTestVar.value == 5);
    assert(&object == observer->getContext(observer));
}

/**
 * Tests observer class when initialized by accessor methods.
 */
void testObserverAccessors() {
    // Create observer with null args, then
    // use accessors to set notification method and context
    struct Object {int x;} object;
    struct IObserver *observer = puremvc_observer(&(struct Observer){0}, NULL, NULL);

    observer->setContext(observer, &object);
    observer->setNotify(observer, handleNotification);

    assert(observer->getContext(observer) == &object);
    assert(observer->getNotify(observer) == handleNotification);

    // create a test event, setting a payload value and notify
    // the observer with it. since the observer is this class
    // and the notification method is observerTestMethod,
    // successful notification will result in our local
    // observerTestVar being set to the value we pass in
    // on the note body.
    struct ObserverTestVar vo = {.value = 10};
    struct INotification *notification = puremvc_notification(&(struct Notification){0}, "ObserverTestNote", &vo, NULL);
    observer->notifyObserver(observer, notification);
}

/**
 * Tests the compareNotifyContext method of the Observer class
 */
void testCompareNotifyContext() {
    // Create observer passing in notification method and context
    struct Object {char dummy;};
    struct Object object = {0};
    struct Object negTestObj = {0};
    struct IObserver *observer = puremvc_observer(&(struct Observer){0}, handleNotification, &object);

    // test assertions
    assert(observer->compareNotifyContext(observer, &negTestObj) == false);
    assert(observer->compareNotifyContext(observer, &object) == true);
}

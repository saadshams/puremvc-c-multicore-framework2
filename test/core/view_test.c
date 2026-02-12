#include "puremvc/i_view.h"
#include "puremvc/i_notification.h"
#include "puremvc/i_mediator.h"
#include "puremvc/i_observer.h"

#include "view_test.h"

#include "view_test_mediator.h"
#include "view_test_mediator2.h"
#include "view_test_mediator3.h"
#include "view_test_mediator4.h"
#include "view_test_mediator5.h"
#include "view_test_mediator6.h"
#include "view_test_mediator7.h"

#include <alloca.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test(const char *name, void (*callback)(void)) {
    printf("\033[0;34m[RUNNING]\033[0m %s...\n", name);
    fflush(stdout);

    callback();

    printf("\033[0;32m[PASSED]\033[0m %s\n", name);
    fflush(stdout);
}

int main() {
    printf("\n\033[1;36m================================================\033[0m\n");
    printf("\033[1;36m[SUITE] %s\033[0m\n", "ViewTest");
    printf("\033[1;36m================================================\033[0m\n\n");

    test("testGetInstance", testGetInstance);
    // test("testRegisterAndNotifyObserver", testRegisterAndNotifyObserver); //
    test("testRegisterAndRetrieveMediator", testRegisterAndRetrieveMediator);
    test("testHasMediator", testHasMediator);
    test("testRegisterAndRemoveMediator", testRegisterAndRemoveMediator);
    test("testOnRegisterAndOnRemove", testOnRegisterAndOnRemove);
    test("testSuccessiveRegisterAndRemoveMediator", testSuccessiveRegisterAndRemoveMediator);
    test("testRemoveMediatorAndSubsequentNotify", testRemoveMediatorAndSubsequentNotify);
    test("testGetInstance", testGetInstance);
    test("testMediatorReregistration", testMediatorReregistration);
    test("testModifyObserverListDuringNotification", testModifyObserverListDuringNotification);
    test("testRemoveView", testRemoveView);
    test("testGarbageStorageForView", testGarbageStorageForView);
    test("testGarbageStorageForObserver", testGarbageStorageForObserver);
    test("testGarbageStorageForMediator", testGarbageStorageForMediator);
    test("testObserverMapShiftLeft", testObserverMapShiftLeft);
    test("testObserverShiftLeft", testObserverShiftLeft);
    test("testMediatorMapShiftLeft", testMediatorMapShiftLeft);
    test("testViewMapShiftLeft", testViewMapShiftLeft);

    printf("\n\033[1;32m[DONE] All tests in suite finished.\033[0m\n");
    return 0;
}

struct ViewTestVar *viewTestVar;

void handleNotification(const void *context, const struct INotification notification) {
    (void)context;
    viewTestVar = (struct ViewTestVar *)notification.getBody(&notification);
}

void testGetInstance() {
    struct ViewMap **instanceMap = (struct ViewMap *[]) {
        &(struct ViewMap){ .view = alloca(puremvc_view_size()) },
        NULL
    };

    // Test Factory Method
    struct IView *view = puremvc_view_getInstance(instanceMap, "ViewTestKey1");
    assert(view != NULL);

    view->initializeView(view, NULL, NULL);

    // test assertions
    assert(view != NULL);
    assert(view == puremvc_view_getInstance(instanceMap, "ViewTestKey1"));
    assert(strcmp(instanceMap[0]->key, "ViewTestKey1") == 0);

    assert(puremvc_view_removeView("ViewTestKey1", NULL) == true);
    assert(instanceMap[0]->key == NULL);
}

void testRegisterAndNotifyObserver() {
    struct ViewMap **instanceMap = (struct ViewMap *[]) {
        &(struct ViewMap){ .view = alloca(puremvc_view_size()) },
        NULL
    };

    struct ObserverMap **observerMap = (struct ObserverMap *[]) {
        &(struct ObserverMap) {
            .observers = (struct IObserver *[]) { memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()), NULL }
        },
        NULL
    };

    struct ViewComponent viewComponent = {0};

    // Get the Multiton View instance
    struct IView *view = puremvc_view_getInstance(instanceMap, "ViewTestKey2");
    assert(view != NULL);

    view->initializeView(view, observerMap, NULL);

    // Register Observer's interest in a particular Notification with the View, passing in notification method and context
    assert(view->registerObserver(view, "ViewTestNote1", (void (*)(const void *, const struct INotification *)) handleNotification, &viewComponent) == true);
    // view->registerObserver(view, "ViewTestNote1", observer);

    // Create a ViewTestNote, setting
    // a body value, and tell the View to notify
    // Observers. Since the Observer is this class
    // and the notification method is viewTestMethod,
    // successful notification will result in our local
    // viewTestVar being set to the value we pass in
    // on the note body.
    struct ViewTestVar vo = {.value = 10};
    const struct INotification *notification = puremvc_notification_init(alloca(puremvc_notification_size()), "ViewTestNote1", &vo, NULL);
    view->notifyObservers(view, notification);

    // test assertions
    assert(viewTestVar->value == 10);

    // remove first observer
    viewTestVar->value = 0;
    assert(view->removeObserver(view, "ViewTestNote1", &viewComponent) == true);;
    view->notifyObservers(view, notification);
    assert(viewTestVar->value == 0);

    assert(puremvc_view_removeView("ViewTestKey2", NULL) == true);
}

void testRegisterAndRetrieveMediator() {
    struct ViewMap **instanceMap = (struct ViewMap *[]) {
        &(struct ViewMap){ .view = alloca(puremvc_view_size()) },
        NULL
    };

    struct ObserverMap **observerMap = (struct ObserverMap *[]) {
        &(struct ObserverMap){ .observers = (struct IObserver *[]) { NULL } },
        NULL
    };

    struct MediatorMap **mediatorMap = (struct MediatorMap *[]) {
        &(struct MediatorMap) { .mediator = alloca(puremvc_mediator_size()) },
        NULL
    };

    // Get the Multiton View instance
    struct IView *view = puremvc_view_getInstance(instanceMap, "ViewTestKey3");
    assert(view != NULL);

    view->initializeView(view, observerMap, mediatorMap);

    size_t offset1 = sizeof(struct IView) + sizeof(const char *);
    struct ObserverMap ***ppp1 = (struct ObserverMap ***)((char *) view + offset1);
    struct ObserverMap **obsMap = *ppp1;

    size_t offset2 = offset1 + sizeof(struct ObserverMap **);
    struct MediatorMap ***ppp2 = (struct MediatorMap ***) ((char *) view + offset2);
    struct MediatorMap **medMap = *ppp2;

    // Create and register the test mediator
    struct ViewComponent viewComponent = {0};
    assert(view->registerMediator(view, puremvc_mediator_init, "testing", &viewComponent) == true);;
    assert(strcmp(medMap[0]->key, "testing") == 0);
    assert(strcmp(medMap[0]->mediator->getName(medMap[0]->mediator), "testing") == 0);

    // Retrieve the mediator
    const struct IMediator *mediator = view->retrieveMediator(view, "testing");

    // test assertions
    assert(mediator != NULL);
    assert(mediator->getComponent(mediator) == &viewComponent);
    assert(strcmp(mediator->getName(mediator), "testing") == 0);

    struct IMediator *removedMediator = NULL;
    assert(view->removeMediator(view, "testing", &removedMediator) == true);
    assert(strcmp(removedMediator->getName(removedMediator), "testing") == 0);

    assert(view->retrieveMediator(view, "testing") == NULL);;

    assert(puremvc_view_removeView("ViewTestKey3", NULL) == true);
}

void testHasMediator() {
    struct ViewMap **instanceMap = (struct ViewMap *[]) {
        &(struct ViewMap){ .view = alloca(puremvc_view_size()) },
        NULL
    };

    struct MediatorMap **mediatorMap = (struct MediatorMap *[]) {
        &(struct MediatorMap){ .mediator = alloca(puremvc_mediator_size()) },
        NULL
    };

    // Get the Multiton View instance
    struct IView *view = puremvc_view_getInstance(instanceMap, "ViewTestKey4");
    assert(view != NULL);

    view->initializeView(view, NULL, mediatorMap);

    // Create and register the test mediator
    struct ViewComponent viewComponent = {0};
    assert(view->registerMediator(view, puremvc_mediator_init, "hasMediatorTest", &viewComponent) == true);

    // assert that the view.hasMediator method returns true
    // for that mediator name
    assert(view->hasMediator(view, "hasMediatorTest") == true);

    struct IMediator *mediator = NULL;
    assert(view->removeMediator(view, "hasMediatorTest", &mediator) == true);
    assert(strcmp(mediator->getName(mediator), "hasMediatorTest") == 0);

    // assert that the view.hasMediator method returns false
    // for that mediator name
    assert(view->hasMediator(view, "hasMediatorTest") == false);

    assert(puremvc_view_removeView("ViewTestKey4", NULL) == true);
}

void testRegisterAndRemoveMediator() {
    struct ViewMap **instanceMap = (struct ViewMap *[]){
        &(struct ViewMap){ .view = alloca(puremvc_view_size()) },
        NULL
    };

    struct MediatorMap **mediatorMap = (struct MediatorMap *[]) {
        &(struct MediatorMap){ .mediator = alloca(puremvc_mediator_size()) },
        NULL
    };

    // Get the Multiton View instance
    struct IView *view = puremvc_view_getInstance(instanceMap, "ViewTestKey5");
    view->initializeView(view, NULL, mediatorMap);

    struct ViewComponent viewComponent = {0};

    // Register the mediator
    assert(view->registerMediator(view, puremvc_mediator_init, "testing", &viewComponent) == true);
    assert(view->hasMediator(view, "testing") == true);;

    // Remove the mediator
    struct IMediator *removedMediator = NULL;
    assert(view->removeMediator(view, "testing", &removedMediator) == true);
    assert(view->hasMediator(view, "testing") == false);

    // assert that we have removed the appropriate mediator
    assert(strcmp(removedMediator->getName(removedMediator), "testing") == 0);

    // assert that the mediator is no longer retrievable
    assert(view->retrieveMediator(view, "testing") == NULL);
    assert(view->removeMediator(view, "testing", NULL) == false);

    assert(puremvc_view_removeView("ViewTestKey5", NULL) == true);
}

void testOnRegisterAndOnRemove() {
    struct ViewMap **instanceMap = (struct ViewMap *[]) {
        &(struct ViewMap){ .view = alloca(puremvc_view_size()) },
        NULL
    };

    struct ObserverMap **observerMap = (struct ObserverMap *[]) {
        &(struct ObserverMap) {
            .observers = (struct IObserver *[]) { memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()), NULL }
        },
        NULL
    };

    struct MediatorMap **mediatorMap = (struct MediatorMap *[]) {
        &(struct MediatorMap){ .mediator = alloca(puremvc_mediator_size()) },
        NULL
    };

    // Get the Multiton View instance
    struct IView *view = puremvc_view_getInstance(instanceMap, "ViewTestKey6");
    view->initializeView(view, observerMap, mediatorMap);

    assert(view != NULL);

    struct ViewTest viewTest = {"", false, false, 0};

    assert(view->registerMediator(view, view_test_mediator4, view_test_mediator4_NAME, &viewTest) == true);

    assert(view->hasMediator(view, view_test_mediator4_NAME) == true);

    const struct IMediator *mediator = view->retrieveMediator(view, view_test_mediator4_NAME);
    assert(mediator != NULL);

    // assert that onRegister was called, and the mediator responded by setting our boolean
    assert(viewTest.onRegisterCalled);
    assert(((struct ViewTest *) mediator->getComponent(mediator))->onRegisterCalled == true);

    // Remove the component
    struct IMediator *removedMediator = NULL;
    assert(view->removeMediator(view, view_test_mediator4_NAME, &removedMediator) == true);;
    assert(removedMediator != NULL);
    assert(strcmp(removedMediator->getName(removedMediator), view_test_mediator4_NAME) == 0);
    assert(view->hasMediator(view, view_test_mediator4_NAME) == false);

    // assert that the mediator is no longer retrievable
    assert(view->retrieveMediator(view, view_test_mediator4_NAME) == NULL);

    // assert that onRemove was called, and the mediator responded by setting our boolean
    assert(viewTest.onRemoveCalled);
    assert(((struct ViewTest *) mediator->getComponent(mediator))->onRemoveCalled == true);

    // clean up
    assert(puremvc_view_removeView("ViewTestKey6", NULL) == true);
}

void testSuccessiveRegisterAndRemoveMediator() {
    struct ViewMap **instanceMap = (struct ViewMap *[]) {
        &(struct ViewMap){ .view = alloca(puremvc_view_size()) },
        NULL
    };

    struct ObserverMap **observerMap = (struct ObserverMap *[]) { // 3 ObserverMaps for 3 notifications "ABC", "DEF", "GHI"
        &(struct ObserverMap){ .observers = (struct IObserver *[]) { memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()), NULL } }, // 1 context
        &(struct ObserverMap){ .observers = (struct IObserver *[]) { memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()), NULL } },
        &(struct ObserverMap){ .observers = (struct IObserver *[]) { memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()), NULL } },
        NULL
    };

    struct MediatorMap **mediatorMap = (struct MediatorMap *[]) { // 1 Mediator
        &(struct MediatorMap){ .mediator = alloca(puremvc_mediator_size()) },
        NULL
    };

    // Get the Multiton View instance
    struct IView *view = puremvc_view_getInstance(instanceMap, "ViewTestKey7");
    assert(view != NULL);

    view->initializeView(view, observerMap, mediatorMap);

    // Create and register the test mediator,
    // but not so we have a reference to it
    struct ViewComponent viewComponent = {0};
    view->registerMediator(view, view_test_mediator, view_test_mediator_NAME, &viewComponent);

    // test that we can retrieve it
    assert(view->retrieveMediator(view, view_test_mediator_NAME) != NULL);

    // assertions
    assert(view->hasMediator(view, view_test_mediator_NAME) == true); // todo remove observer true

    // Remove the Mediator
    struct IMediator *removedMediator = NULL;
    assert(view->removeMediator(view, view_test_mediator_NAME, &removedMediator) == true);
    assert(view->hasMediator(view, view_test_mediator_NAME) == false);

    // confirm observers have already been removed as a result of removeMediator
    assert(view->removeObserver(view, "DEF", removedMediator) == false);
    assert(view->removeObserver(view, "GHI", removedMediator) == false);
    assert(view->removeObserver(view, "ABC", removedMediator) == false);

    // test that retrieving it now returns null
    assert(view->removeMediator(view, view_test_mediator_NAME, NULL) == false);;

    // test that removing the mediator again once its gone doesn't cause crash
    assert(view->removeMediator(view, view_test_mediator_NAME, NULL) == false);

    // Create and register another instance of the test mediator,
    assert(view->registerMediator(view, view_test_mediator, view_test_mediator_NAME, &viewComponent) == true);
    assert(view->hasMediator(view, view_test_mediator_NAME) == true);
    assert(view->retrieveMediator(view, view_test_mediator_NAME) != NULL);
    struct IMediator *retrievedMediator = view->retrieveMediator(view, view_test_mediator_NAME);
    assert(strcmp(retrievedMediator->getName(retrievedMediator), view_test_mediator_NAME) == 0);

    // Remove the Mediator
    removedMediator = NULL;
    assert(view->removeMediator(view, view_test_mediator_NAME, &removedMediator) == true);
    assert(strcmp(removedMediator->getName(removedMediator), view_test_mediator_NAME) == 0);
    assert(view->hasMediator(view, view_test_mediator_NAME) == false);
    assert(view->removeObserver(view, "DEF", removedMediator) == false); // already removed
    assert(view->removeObserver(view, "GHI", removedMediator) == false);
    assert(view->removeObserver(view, "ABC", removedMediator) == false);

    // test that retrieving it now returns null
    assert(view->retrieveMediator(view, view_test_mediator_NAME) == NULL);

    assert(puremvc_view_removeView("ViewTestKey7", NULL) == true);
}

void testRemoveMediatorAndSubsequentNotify() {
    struct ViewMap **instanceMap = (struct ViewMap *[]){
        &(struct ViewMap){ .view = alloca(puremvc_view_size()) },
        NULL
    };

    struct ObserverMap **observerMap = (struct ObserverMap *[]){ // 2 notifications, one context each
        &(struct ObserverMap){ .observers = (struct IObserver *[]){ memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()), NULL } },
        &(struct ObserverMap){ .observers = (struct IObserver *[]){ memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()), NULL } },
        NULL
    };

    struct MediatorMap **mediatorMap = (struct MediatorMap *[]){
        &(struct MediatorMap){ .mediator = alloca(puremvc_mediator_size()) },
        NULL
    };

    // Get the Multiton View instance
    struct IView *view = puremvc_view_getInstance(instanceMap, "ViewTestKey8");
    assert(view != NULL);

    view->initializeView(view, observerMap, mediatorMap);

    // Create and register the test mediator to be removed.
    struct ViewTest viewTest = {.lastNotification = NULL };
    view->registerMediator(view, view_test_mediator2, view_test_mediator2_NAME, &viewTest);

    // test that we can retrieve it
    assert(view->retrieveMediator(view, view_test_mediator2_NAME) != NULL);

    const struct IMediator *mediator = view->retrieveMediator(view, view_test_mediator2_NAME);

    // assertions
    assert(strcmp(mediator->getName(mediator), view_test_mediator2_NAME) == 0);

    // test that notifications work
    struct INotification *notification1 = puremvc_notification_init(alloca(puremvc_notification_size()), NOTE1, NULL, NULL);
    view->notifyObservers(view, notification1);
    assert(strcmp(viewTest.lastNotification, NOTE1) == 0);

    struct INotification *notification2 = puremvc_notification_init(alloca(puremvc_notification_size()), NOTE2, NULL, NULL);
    view->notifyObservers(view, notification2);
    assert(strcmp(viewTest.lastNotification, NOTE2) == 0);

    struct IMediator *removedMediator = NULL;
    assert(view->removeMediator(view, view_test_mediator2_NAME, &removedMediator) == true);;
    assert(strcmp(removedMediator->getName(removedMediator), view_test_mediator2_NAME) == 0);
    assert(view->hasMediator(view, view_test_mediator_NAME) == false);
    assert(view->removeObserver(view, NOTE1, removedMediator) == false);
    assert(view->removeObserver(view, NOTE2, removedMediator) == false);

    assert(view->retrieveMediator(view, view_test_mediator2_NAME) == NULL);

    viewTest.lastNotification = "";

    view->notifyObservers(view, notification1);
    assert(strcmp(viewTest.lastNotification, NOTE1) != 0);

    view->notifyObservers(view, notification2);
    assert(strcmp(viewTest.lastNotification, NOTE2) != 0);

    assert(puremvc_view_removeView("ViewTestKey8", NULL) == true);
}

void testRemoveOneOfTwoMediatorsAndSubsequentNotify() {
    struct ViewMap **instanceMap = (struct ViewMap *[]){
        &(struct ViewMap){ .view = alloca(puremvc_view_size()) },
        NULL
    };

    struct ObserverMap **observerMap = (struct ObserverMap *[]){ // NOTE1, NOTE2, NOTE3, each one with a context
        &(struct ObserverMap){ .observers = (struct IObserver *[]){ memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()), NULL }, },
        &(struct ObserverMap){ .observers = (struct IObserver *[]){ memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()), NULL }, },
        &(struct ObserverMap){ .observers = (struct IObserver *[]){ memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()), NULL }, },
        NULL
    };

    struct MediatorMap **mediatorMap = (struct MediatorMap *[]){ // 2 mediators
        &(struct MediatorMap){ .mediator = alloca(puremvc_mediator_size()) },
        &(struct MediatorMap){ .mediator = alloca(puremvc_mediator_size()) },
        NULL
    };

    // Get the Multiton View instance
    struct IView *view = puremvc_view_getInstance(instanceMap, "ViewTestKey9");
    assert(view != NULL);

    view->initializeView(view, observerMap, mediatorMap);

    struct ViewTest viewTest = {};

    // Create and register that responds to notifications 1 and 2
    assert(view->registerMediator(view, view_test_mediator2, view_test_mediator2_NAME, &viewTest) == true);

    // assertions
    assert(view->hasMediator(view, view_test_mediator2_NAME) == true);

    // Create and register that responds to notification 3
    assert(view->registerMediator(view, view_test_mediator3, view_test_mediator3_NAME, &viewTest) == true);

    // assertions
    assert(view->hasMediator(view, view_test_mediator3_NAME) == true);

    // test that all notifications work
    struct INotification *notification1 = puremvc_notification_init(alloca(puremvc_notification_size()), NOTE1, NULL, NULL);
    view->notifyObservers(view, notification1);
    assert(strcmp(viewTest.lastNotification, NOTE1) == 0);

    struct INotification *notification2 = puremvc_notification_init(alloca(puremvc_notification_size()), NOTE2, NULL, NULL);
    view->notifyObservers(view, notification2);
    assert(strcmp(viewTest.lastNotification, NOTE2) == 0);

    struct INotification *notification3 = puremvc_notification_init(alloca(puremvc_notification_size()), NOTE3, NULL, NULL);
    view->notifyObservers(view, notification3);
    assert(strcmp(viewTest.lastNotification, NOTE3) == 0);

    // Remove the Mediator that responds to 1 and 2
    struct IMediator *removedMediator2 = NULL;
    assert(view->removeMediator(view, view_test_mediator2_NAME, &removedMediator2) == true);
    assert(view->hasMediator(view, view_test_mediator2_NAME) == false);
    assert(view->removeObserver(view, NOTE1, removedMediator2) == false);
    assert(view->removeObserver(view, NOTE2, removedMediator2) == false);

    // test that retrieving it now returns null
    assert(view->retrieveMediator(view, view_test_mediator2_NAME) == NULL);

    // test that notifications no longer work
    // for notifications 1 and 2, but still work for 3
    viewTest.lastNotification = "";

    view->notifyObservers(view, notification1);
    assert(strcmp(viewTest.lastNotification, NOTE1) != 0);

    view->notifyObservers(view, notification2);
    assert(strcmp(viewTest.lastNotification, NOTE2) != 0);

    view->notifyObservers(view, notification3);
    assert(strcmp(viewTest.lastNotification, NOTE3) == 0);

    // Remove the Mediator that responds to 3
    struct IMediator *removedMediator3 = NULL;
    assert(view->removeMediator(view, view_test_mediator3_NAME, &removedMediator3) == true);;
    assert(strcmp(removedMediator3->getName(removedMediator3), view_test_mediator3_NAME) == 0);
    assert(view->hasMediator(view, view_test_mediator3_NAME) == false);
    assert(view->removeObserver(view, NOTE3, removedMediator3) == false);

    assert(puremvc_view_removeView("ViewTestKey9", NULL) == true);
}

void testMediatorReregistration() {
    struct ViewMap **instanceMap = (struct ViewMap *[]) {
        &(struct ViewMap){ .view = alloca(puremvc_view_size()) },
        NULL
    };

    struct ObserverMap **observerMap = (struct ObserverMap *[]) { // NOTE5
        &(struct ObserverMap){ .observers = (struct IObserver *[]){ memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()), NULL } },
        NULL
    };

    struct MediatorMap **mediatorMap = (struct MediatorMap *[]) {
        &(struct MediatorMap){ .mediator = alloca(puremvc_mediator_size()) },
        NULL
    };

    // Get the Multiton View instance
    struct IView *view = puremvc_view_getInstance(instanceMap, "ViewTestKey10");
    assert(view != NULL);

    view->initializeView(view, observerMap, mediatorMap);

    // Create the counter
    struct ViewTest viewTest = { .counter = 0 };

    // Create and register that responds to notification 5
    assert(view->registerMediator(view, view_test_mediator5, view_test_mediator5_NAME, &viewTest) == true);;

    // try to register another instance of that mediator (uses the same NAME constant). It will trigger exists warning
    assert(view->registerMediator(view, view_test_mediator5, view_test_mediator5_NAME, &viewTest) == false);

    // assertions
    assert(view->hasMediator(view, view_test_mediator5_NAME) == true);

    // test that the counter is only incremented once (mediator 5's response)
    viewTest.counter = 0;
    const struct INotification *notification = puremvc_notification_init(alloca(puremvc_notification_size()), NOTE5, NULL, NULL);
    view->notifyObservers(view, notification);
    assert(viewTest.counter == 1);

    // Remove the Mediator
    struct IMediator *removedMediator = NULL;
    assert(view->removeMediator(view, view_test_mediator5_NAME, &removedMediator) == true);
    assert(strcmp(removedMediator->getName(removedMediator), view_test_mediator5_NAME) == 0);
    assert(view->hasMediator(view, view_test_mediator5_NAME) == false);
    assert(view->removeObserver(view, NOTE5, removedMediator) == false);

    // test that retrieving it now returns null
    assert(view->retrieveMediator(view, view_test_mediator5_NAME) == NULL);

    // test that the counter is no longer incremented
    viewTest.counter = 0;
    view->notifyObservers(view, notification);
    assert(viewTest.counter == 0);

    assert(puremvc_view_removeView("ViewTestKey10", NULL) == true);
}

/**
 * Verifies that the observer list is not modified while a notification
 * dispatch is in progress.
 *
 * In this test, each mediator removes itself in response to the notification.
 * Removing a mediator also removes one of its observers, which would otherwise
 * compact the observer list (e.g., via memmove) while it is being iterated.
 *
 * Modifying the observer list during dispatch would invalidate observer
 * addresses and lead to undefined behavior. The expected behavior is that
 * removals are deferred until after notification dispatch completes.
 */
/**
 * Observers and Mediators must not be structurally modified during notification dispatch.
 * Mediators in this test remove themselves when notified, which would otherwise
 * invalidate the observer list during iteration.
 */
void testModifyObserverListDuringNotification() {
    struct ViewMap **instanceMap = (struct ViewMap *[]) {
        &(struct ViewMap){ .view = alloca(puremvc_view_size()) },
        NULL
    };

    struct ObserverMap **observerMap = (struct ObserverMap *[]) { // 1 Notification = 1 ObserverMap
        &(struct ObserverMap) {
            .observers = (struct IObserver *[]) { // 8 Observers to store contexts of 8 mediators
                memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()),
                memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()),
                memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()),
                memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()),
                memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()),
                memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()),
                memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()),
                memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()),
                NULL
            }
        },
        NULL // null terminator, extra slot
    };

    struct MediatorMap **mediatorMap = (struct MediatorMap *[]) {
        &(struct MediatorMap){ .mediator = alloca(puremvc_mediator_size()) },
        &(struct MediatorMap){ .mediator = alloca(puremvc_mediator_size()) },
        &(struct MediatorMap){ .mediator = alloca(puremvc_mediator_size()) },
        &(struct MediatorMap){ .mediator = alloca(puremvc_mediator_size()) },
        &(struct MediatorMap){ .mediator = alloca(puremvc_mediator_size()) },
        &(struct MediatorMap){ .mediator = alloca(puremvc_mediator_size()) },
        &(struct MediatorMap){ .mediator = alloca(puremvc_mediator_size()) },
        &(struct MediatorMap){ .mediator = alloca(puremvc_mediator_size()) },
        NULL // null terminator, extra slot
    };

    // Get the Multiton View instance
    struct IView *view = puremvc_view_getInstance(instanceMap, "ViewTestKey11");
    assert(view != NULL);

    view->initializeView(view, observerMap, mediatorMap);

    static char buffer[9][32]; // Create the actual bytes (9 rows, each 32 characters wide)
    static char *buffers[] = {
        buffer[0], buffer[1], buffer[2],
        buffer[3], buffer[4], buffer[5],
        buffer[6], buffer[7], NULL // NULL terminator for loops
    };

    struct ViewTest viewTest = {"", "", "", 0, .deferred = buffers };

    // Create and register several mediator instances that respond to notification 6
    // by removing themselves, which will cause the observer list for that notification
    // to change.
    assert(view->registerMediator(view, view_test_mediator6, "view_test_mediator6/1", &viewTest) == true);
    assert(view->registerMediator(view, view_test_mediator6, "view_test_mediator6/2", &viewTest) == true);
    assert(view->registerMediator(view, view_test_mediator6, "view_test_mediator6/3", &viewTest) == true);
    assert(view->registerMediator(view, view_test_mediator6, "view_test_mediator6/4", &viewTest) == true);
    assert(view->registerMediator(view, view_test_mediator6, "view_test_mediator6/5", &viewTest) == true);
    assert(view->registerMediator(view, view_test_mediator6, "view_test_mediator6/6", &viewTest) == true);
    assert(view->registerMediator(view, view_test_mediator6, "view_test_mediator6/7", &viewTest) == true);
    assert(view->registerMediator(view, view_test_mediator6, "view_test_mediator6/8", &viewTest) == true);

    // assertions
    for (size_t i = 0; i < 8; i++) {
        char mediator_name[32] = {0};
        snprintf(mediator_name, 32, "view_test_mediator6/%zu", i + 1);
        assert(view->hasMediator(view, mediator_name) == true);
    }

    // send the notification. each of the above mediators will respond by removing
    // themselves and incrementing the counter by 1. This should leave us with a
    // count of 8, since 8 mediators will respond.
    struct INotification *notification = puremvc_notification_init(alloca(puremvc_notification_size()), NOTE6, NULL, NULL);
    view->notifyObservers(view, notification);

    // assertions
    for (size_t i = 0; i < 8; i++) {
        char mediator_name[32] = {0};
        snprintf(mediator_name, 32, "view_test_mediator6/%zu", i + 1);
        assert(strcmp(viewTest.deferred[i], mediator_name) == 0);
    }

    // iterate through deferred names and call removeMediator
    for (size_t i = 0; viewTest.deferred[i] != NULL; i++) {
        const char *mediatorName = viewTest.deferred[i];
        assert(mediatorName != NULL);
        struct IMediator *removedMediator = NULL;
        assert(view->removeMediator(view, mediatorName, &removedMediator) == true);
        assert(strcmp(removedMediator->getName(removedMediator), mediatorName) == 0);
    }

    // verify the count is correct
    assert(viewTest.counter == 8);

    // clear the counter
    viewTest.counter = 0;
    view->notifyObservers(view, notification);

    // verify the count is 0
    assert(viewTest.counter == 0);

    assert(puremvc_view_removeView("ViewTestKey11", NULL) == true);
}

void testRemoveView() {
    struct ViewMap **instanceMap = (struct ViewMap *[]) {
        &(struct ViewMap){ .view = alloca(puremvc_view_size()) },
        NULL
    };

    // Get a Multiton View instance
    struct IView *view = puremvc_view_getInstance(instanceMap, "ViewTestKey12");
    assert(view != NULL);

    // remove the View
    assert(puremvc_view_removeView("ViewTestKey12", NULL) == true);

    // re-create the view without throwing an exception
    puremvc_view_getInstance(instanceMap, "ViewTestKey12");

    // try removing again
    assert(puremvc_view_removeView("ViewTestKey12", NULL) == true);
}

void testGarbageStorageForView() {
    struct ViewMap *viewMap1[] = { NULL }; // empty view
    const struct IView *view1 = puremvc_view_getInstance(viewMap1, "ViewTestKey13"); // crash test
    assert(view1 == NULL);
    assert(puremvc_view_removeView("ViewTestKey13", NULL) == true);
}

void testGarbageStorageForObserver() {
    struct ViewComponent component = {0};

    // no ObserverMap
    struct ViewMap **instanceMap1 = (struct ViewMap *[]) { &(struct ViewMap){ .view = alloca(puremvc_view_size()) }, NULL };
    struct ObserverMap **observerMap1 = NULL;
    struct IView *view1 = puremvc_view_getInstance(instanceMap1, "ViewTestKey14");
    view1->initializeView(view1, observerMap1, NULL); // no observerMap
    view1->notifyObservers(view1, NULL); // crash test
    view1->removeObserver(view1, "ViewTestKey14_testing1", &component);
    view1->registerObserver(view1, "ViewTestKey14_testing1", (void (*)(const void *, const struct INotification *)) handleNotification, &component);
    assert(puremvc_view_removeView("ViewTestKey14", NULL) == true);

    // empty observerMap
    struct ViewMap **instanceMap2 = (struct ViewMap *[]) {  &(struct ViewMap){ .view = alloca(puremvc_view_size()) }, NULL };
    struct ObserverMap **observerMap2 = (struct ObserverMap *[]) { NULL };
    struct IView *view2 = puremvc_view_getInstance(instanceMap2, "ViewTestKey14");
    view2->initializeView(view2, observerMap2, NULL);
    view2->notifyObservers(view2, NULL);
    view2->removeObserver(view2, "ViewTestKey14_testing2", &component);
    view2->registerObserver(view2, "ViewTestKey14_testing2", (void (*)(const void *, const struct INotification *)) handleNotification, &component);
    assert(puremvc_view_removeView("ViewTestKey14", NULL) == true);

    // empty ObserverMap field
    struct ViewMap **instanceMap3 = (struct ViewMap *[]) {  &(struct ViewMap){ .view = alloca(puremvc_view_size()) }, NULL };
    struct ObserverMap **observerMap3 = (struct ObserverMap *[]) { &(struct ObserverMap){}, NULL };
    struct IView *view3 = puremvc_view_getInstance(instanceMap3, "ViewTestKey14");
    view3->initializeView(view3, observerMap3, NULL);
    view3->notifyObservers(view3, NULL);
    view3->removeObserver(view3, "ViewTestKey14_testing3", &component);
    view3->registerObserver(view3, "ViewTestKey14_testing3", (void (*)(const void *, const struct INotification *)) handleNotification, &component);
    assert(puremvc_view_removeView("ViewTestKey14", NULL) == true);

    // empty observers
    struct ViewMap **instanceMap4 = (struct ViewMap *[]) {  &(struct ViewMap){ .view = alloca(puremvc_view_size()) }, NULL };
    struct IView *view4 = puremvc_view_getInstance(instanceMap4, "ViewTestKey14");
    struct ObserverMap **observerMap4 = (struct ObserverMap *[]) { &(struct ObserverMap){ .observers = (struct IObserver *[]){ NULL } }, NULL };
    view4->initializeView(view4, observerMap4, NULL);
    view4->notifyObservers(view4, NULL);
    view4->removeObserver(view4, "ViewTestKey14_testing4", &component);
    view4->registerObserver(view4, "ViewTestKey14_testing4", (void (*)(const void *, const struct INotification *)) handleNotification, &component);
    assert(puremvc_view_removeView("ViewTestKey14", NULL) == true);
}

void testGarbageStorageForMediator() {
    // missing MediatorMap
    struct ViewMap **instanceMap1 = (struct ViewMap *[]) { &(struct ViewMap){ .view = alloca(puremvc_view_size()) }, NULL };
    struct MediatorMap **mediatorMap1 = NULL;
    struct IView *view1 = puremvc_view_getInstance(instanceMap1, "ViewTestKey15");
    view1->initializeView(view1, NULL, mediatorMap1);
    assert(view1 != NULL);
    assert(view1->registerMediator(view1, puremvc_mediator_init, MEDIATOR_NAME, NULL) == false);
    assert(view1->hasMediator(view1, MEDIATOR_NAME) == false);;
    assert(view1->retrieveMediator(view1, MEDIATOR_NAME) == NULL);;
    assert(view1->removeMediator(view1, MEDIATOR_NAME, NULL) == false);
    assert(puremvc_view_removeView("ViewTestKey15", NULL) == true);

    // empty mediatorMap
    struct ViewMap **instanceMap2 = (struct ViewMap *[]) { &(struct ViewMap){ .view = alloca(puremvc_view_size()) }, NULL };
    struct MediatorMap **mediatorMap2 = (struct MediatorMap *[]){ NULL };
    struct IView *view2 = puremvc_view_getInstance(instanceMap2, "ViewTestKey15");
    view2->initializeView(view2, NULL, mediatorMap2);
    assert(view2->registerMediator(view2, puremvc_mediator_init, MEDIATOR_NAME, NULL) == false);;
    assert(view2->hasMediator(view2, MEDIATOR_NAME) == false);
    assert(view2->retrieveMediator(view2, MEDIATOR_NAME) == NULL);;
    assert(view2->removeMediator(view2, MEDIATOR_NAME, NULL) == false);
    assert(puremvc_view_removeView("ViewTestKey15", NULL) == true);
}

void testObserverMapShiftLeft() {
    // 4 ObserverMaps each with a single Observer
    struct ViewMap **instanceMap = (struct ViewMap *[]) { &(struct ViewMap){ .view = alloca(puremvc_view_size()) }, NULL };

    struct ObserverMap **observerMap = (struct ObserverMap *[]){
        &(struct ObserverMap){ .observers = (struct IObserver *[]){ memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()), NULL } },
        &(struct ObserverMap){ .observers = (struct IObserver *[]){ memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()), NULL } },
        &(struct ObserverMap){ .observers = (struct IObserver *[]){ memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()), NULL } },
        &(struct ObserverMap){ .observers = (struct IObserver *[]){ memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()), NULL } },
        NULL
    };

    struct IView *view = puremvc_view_getInstance(instanceMap, "ViewTestKey16");
    view->initializeView(view, observerMap, NULL);

    struct ViewComponent component0 = {0}, component1 = {0}, component2 = {0}, component3 = {0};

    size_t offset = sizeof(struct IView) + sizeof(const char *);
    struct ObserverMap ***ppp1 = (struct ObserverMap ***)((char *) view + offset);
    struct ObserverMap **obsMap = *ppp1;

    // register four observers, check association and remove them
    view->registerObserver(view, "observer0", NULL, (void *) &component0);
    assert(strcmp(obsMap[0]->key, "observer0") == 0);
    assert(obsMap[0]->observers[0]->getContext(obsMap[0]->observers[0]) == &component0);

    view->registerObserver(view, "observer1", NULL, (void *) &component1);
    assert(strcmp(obsMap[1]->key, "observer1") == 0);
    assert(obsMap[1]->observers[0]->getContext(obsMap[1]->observers[0]) == &component1);

    view->registerObserver(view, "observer2", NULL, (void *) &component2);
    assert(strcmp(obsMap[2]->key, "observer2") == 0);
    assert(obsMap[2]->observers[0]->getContext(obsMap[2]->observers[0]) == &component2);

    view->registerObserver(view, "observer3", NULL, (void *) &component3);
    assert(strcmp(obsMap[3]->key, "observer3") == 0);
    assert(obsMap[3]->observers[0]->getContext(obsMap[3]->observers[0]) == &component3);

    // remove the middle observer1; remaining entries (2, 3) shift left and the tail is reinit.
    view->removeObserver(view, "observer1", &component1);
    assert(strcmp(obsMap[0]->key, "observer0") == 0);
    assert(obsMap[0]->observers[0]->getContext(obsMap[0]->observers[0]) == &component0);
    assert(strcmp(obsMap[1]->key, "observer2") == 0);
    assert(obsMap[1]->observers[0]->getContext(obsMap[1]->observers[0]) == &component2);
    assert(strcmp(obsMap[2]->key, "observer3") == 0);
    assert(obsMap[2]->observers[0]->getContext(obsMap[2]->observers[0]) == &component3);
    assert(obsMap[3]->key == NULL);
    assert(obsMap[3]->observers[0]->getContext(obsMap[3]->observers[0]) == NULL);
    assert(obsMap[4] == NULL);

    // remove the last observer3 and verify the remaining entries (0, 2) stay in place
    view->removeObserver(view, "observer3", &component3);
    assert(strcmp(obsMap[0]->key, "observer0") == 0);
    assert(obsMap[0]->observers[0]->getContext(obsMap[0]->observers[0]) == &component0);
    assert(strcmp(obsMap[1]->key, "observer2") == 0);
    assert(obsMap[1]->observers[0]->getContext(obsMap[1]->observers[0]) == &component2);
    assert(obsMap[3]->key == NULL);
    assert(obsMap[3]->observers[0]->getContext(obsMap[2]->observers[0]) == NULL);
    assert(obsMap[3]->key == NULL);
    assert(obsMap[3]->observers[0]->getContext(obsMap[3]->observers[0]) == NULL);
    assert(obsMap[4] == NULL);

    // remove the first observer0 and verify that remaining observer shift left
    view->removeObserver(view, "observer0", &component0);
    assert(strcmp(obsMap[0]->key, "observer2") == 0);
    assert(obsMap[0]->observers[0]->getContext(obsMap[0]->observers[0]) == &component2);
    assert(obsMap[1]->key == NULL);
    assert(obsMap[1]->observers[0]->getContext(obsMap[1]->observers[0]) == NULL);
    assert(obsMap[2]->key == NULL);
    assert(obsMap[2]->observers[0]->getContext(obsMap[2]->observers[0]) == NULL);
    assert(obsMap[3]->key == NULL);
    assert(obsMap[3]->observers[0]->getContext(obsMap[3]->observers[0]) == NULL);
    assert(obsMap[4] == NULL);

    // Remove the remaining observer2
    view->removeObserver(view, "observer2", &component2);
    assert(obsMap[0]->key == NULL);
    assert(obsMap[0]->observers[0]->getContext(obsMap[0]->observers[0]) == NULL);
    assert(obsMap[1]->key == NULL);
    assert(obsMap[1]->observers[0]->getContext(obsMap[1]->observers[0]) == NULL);
    assert(obsMap[2]->key == NULL);
    assert(obsMap[2]->observers[0]->getContext(obsMap[2]->observers[0]) == NULL);
    assert(obsMap[3]->key == NULL);
    assert(obsMap[3]->observers[0]->getContext(obsMap[3]->observers[0]) == NULL);
    assert(obsMap[4] == NULL);

    assert(puremvc_view_removeView("ViewTestKey16", NULL) == true);
}

// another test for observers shift left in a single ObserverMap
void testObserverShiftLeft() {
    // An ObserverMaps with 4 Observers
    struct ViewMap **instanceMap = (struct ViewMap *[]) { &(struct ViewMap){ .view = alloca(puremvc_view_size()) }, NULL };

    struct ObserverMap **observerMap = (struct ObserverMap *[]){
        &(struct ObserverMap){ .observers = (struct IObserver *[]) {
            memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()),
            memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()),
            memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()),
            memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()),
            NULL
        } },
        NULL
    };

    struct ViewComponent context0 = {0}, context1 = {0}, context2 = {0}, context3 = {0};
    struct IView *view = puremvc_view_getInstance(instanceMap, "ViewTestKey17");
    view->initializeView(view, observerMap, NULL);

    size_t offset = sizeof(struct IView) + sizeof(const char *);
    struct ObserverMap ***ppp1 = (struct ObserverMap ***)((char *) view + offset);
    struct ObserverMap **obsMap = *ppp1;

    // register four observers, check association and remove them
    assert(view->registerObserver(view, "notification0", NULL, (void *) &context0) == true);
    assert(strcmp(obsMap[0]->key, "notification0") == 0); // key gets created
    assert(obsMap[0]->observers[0]->getContext(obsMap[0]->observers[0]) == &context0);

    assert(view->registerObserver(view, "notification0", NULL, &context1) == true);
    assert(obsMap[0]->observers[1]->getContext(obsMap[0]->observers[1]) == &context1);

    assert(view->registerObserver(view, "notification0", NULL, &context2) == true);
    assert(obsMap[0]->observers[2]->getContext(obsMap[0]->observers[2]) == &context2);

    assert(view->registerObserver(view, "notification0", NULL, &context3) == true);
    assert(obsMap[0]->observers[3]->getContext(obsMap[0]->observers[3]) == &context3);
    assert(obsMap[0]->observers[4] == NULL);

    // remove the middle context1; remaining entries (2, 3) shift left and the tail is reinit.
    assert(view->removeObserver(view, "notification0", &context1) == true);
    assert(obsMap[0]->observers[0]->getContext(obsMap[0]->observers[0]) == &context0);
    assert(obsMap[0]->observers[1]->getContext(obsMap[0]->observers[1]) == &context2);
    assert(obsMap[0]->observers[2]->getContext(obsMap[0]->observers[2]) == &context3);
    assert(obsMap[0]->observers[3]->getContext(obsMap[0]->observers[3]) == NULL);
    assert(obsMap[0]->observers[4] == NULL);

    // remove the last context3 and verify the remaining entries (0, 2) stay in place
    assert(view->removeObserver(view, "notification0", &context3) == true);
    assert(obsMap[0]->observers[0]->getContext(obsMap[0]->observers[0]) == &context0);
    assert(obsMap[0]->observers[1]->getContext(obsMap[0]->observers[1]) == &context2);
    assert(obsMap[0]->observers[2]->getContext(obsMap[0]->observers[2]) == NULL);
    assert(obsMap[0]->observers[3]->getContext(obsMap[0]->observers[3]) == NULL);
    assert(obsMap[0]->observers[4] == NULL);

    // remove the first context0 and verify that remaining observer shift left
    assert(view->removeObserver(view, "notification0", &context0) == true);
    assert(obsMap[0]->observers[0]->getContext(obsMap[0]->observers[0]) == &context2);
    assert(obsMap[0]->observers[1]->getContext(obsMap[0]->observers[1]) == NULL);
    assert(obsMap[0]->observers[2]->getContext(obsMap[0]->observers[2]) == NULL);
    assert(obsMap[0]->observers[3]->getContext(obsMap[0]->observers[3]) == NULL);
    assert(obsMap[0]->observers[4] == NULL);

    // Remove the remaining context2
    assert(view->removeObserver(view, "notification0", &context2) == true);
    assert(obsMap[0]->observers[0]->getContext(obsMap[0]->observers[0]) == NULL);
    assert(obsMap[0]->observers[1]->getContext(obsMap[0]->observers[1]) == NULL);
    assert(obsMap[0]->observers[2]->getContext(obsMap[0]->observers[2]) == NULL);
    assert(obsMap[0]->observers[3]->getContext(obsMap[0]->observers[3]) == NULL);
    assert(obsMap[0]->observers[4] == NULL);

    assert(obsMap[0]->key == NULL); // key gets deleted

    // Test duplicate context registration and repeated removal:
    assert(view->registerObserver(view, "notification0", NULL, &context0) == true);
    assert(view->registerObserver(view, "notification0", NULL, &context0) == false);
    assert(view->registerObserver(view, "notification0", NULL, &context0) == false);
    assert(view->registerObserver(view, "notification0", NULL, &context0) == false);

    assert(obsMap[0]->observers[0]->getContext(obsMap[0]->observers[0]) == &context0); // only one gets registered
    assert(obsMap[0]->observers[1]->getContext(obsMap[0]->observers[1]) == NULL);
    assert(obsMap[0]->observers[2]->getContext(obsMap[0]->observers[2]) == NULL);
    assert(obsMap[0]->observers[3]->getContext(obsMap[0]->observers[3]) == NULL);
    assert(obsMap[0]->observers[4] == NULL);

    assert(view->removeObserver(view, "notification0", &context0) == true);
    assert(view->removeObserver(view, "notification0", &context0) == false);
    assert(view->removeObserver(view, "notification0", &context0) == false);
    assert(view->removeObserver(view, "notification0", &context0) == false);

    // Removing a context that was never registered
    assert(view->removeObserver(view, "notification0", &(struct ViewComponent){0}) == false);
}

void testMediatorMapShiftLeft() {
    // viewMap for 4 mediators sharing one notification, one observer per notification
    struct ViewMap **instanceMap = (struct ViewMap *[]) { &(struct ViewMap){ .view = alloca(puremvc_view_size()) }, NULL };

    struct ObserverMap **observerMap = (struct ObserverMap *[]){
        &(struct ObserverMap){ .observers = (struct IObserver *[]) {
            memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()),
            memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()),
            memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()),
            memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()),
            NULL
        } },
        NULL
    };

    struct MediatorMap **mediatorMap = (struct MediatorMap *[]){ // 4 mediators
        &(struct MediatorMap){ .mediator = alloca(puremvc_mediator_size()) },
        &(struct MediatorMap){ .mediator = alloca(puremvc_mediator_size()) },
        &(struct MediatorMap){ .mediator = alloca(puremvc_mediator_size()) },
        &(struct MediatorMap){ .mediator = alloca(puremvc_mediator_size()) },
        NULL
    };

    struct IView *view = puremvc_view_getInstance(instanceMap, "ViewTestKey17");
    view->initializeView(view, observerMap, mediatorMap);

    size_t offset1 = sizeof(struct IView) + sizeof(const char *);
    struct ObserverMap ***ppp1 = (struct ObserverMap ***)((char *) view + offset1);
    struct ObserverMap **obsMap = *ppp1;

    size_t offset2 = offset1 + sizeof(struct ObserverMap **);
    struct MediatorMap ***ppp2 = (struct MediatorMap ***) ((char *) view + offset2);
    struct MediatorMap **medMap = *ppp2;

    // Register four mediators and verify that each is correctly associated to their observers
    view->registerMediator(view, view_test_mediator7, "mediator0", NULL);
    assert(strcmp(medMap[0]->mediator->getName(medMap[0]->mediator), "mediator0") == 0);
    assert(obsMap[0]->observers[0]->getContext(obsMap[0]->observers[0]) == medMap[0]->mediator);

    view->registerMediator(view, view_test_mediator7, "mediator1", NULL);
    assert(strcmp(medMap[1]->mediator->getName(medMap[1]->mediator), "mediator1") == 0);
    assert(obsMap[0]->observers[1]->getContext(obsMap[0]->observers[1]) == medMap[1]->mediator);

    view->registerMediator(view, view_test_mediator7, "mediator2", NULL);
    assert(strcmp(medMap[2]->mediator->getName(medMap[2]->mediator), "mediator2") == 0);
    assert(obsMap[0]->observers[2]->getContext(obsMap[0]->observers[2]) == medMap[2]->mediator);

    view->registerMediator(view, view_test_mediator7, "mediator3", NULL);
    assert(strcmp(medMap[3]->mediator->getName(medMap[3]->mediator), "mediator3") == 0);
    assert(obsMap[0]->observers[3]->getContext(obsMap[0]->observers[3]) == medMap[3]->mediator);

    // Verify the dictionary key for the map is correctly set
    assert(strcmp(obsMap[0]->key, NOTE7) == 0);

    // Remove the second mediator1 (middle) and verify that remaining mediators 2, 3 are shifted correctly
    struct IMediator *removedMediator1 = NULL;
    assert(view->removeMediator(view, "mediator1", &removedMediator1) == true);
    assert(strcmp(removedMediator1->getName(removedMediator1), "mediator1") == 0);
    assert(strcmp(medMap[0]->key, "mediator0") == 0);
    assert(strcmp(medMap[0]->mediator->getName(medMap[0]->mediator), "mediator0") == 0);
    assert(obsMap[0]->observers[0]->getContext(obsMap[0]->observers[0]) == medMap[0]->mediator);
    assert(strcmp(medMap[1]->key, "mediator2") == 0);
    assert(strcmp(medMap[1]->mediator->getName(medMap[1]->mediator), "mediator2") == 0);
    assert(obsMap[0]->observers[1]->getContext(obsMap[0]->observers[1]) == medMap[1]->mediator);
    assert(strcmp(medMap[2]->key, "mediator3") == 0);
    assert(strcmp(medMap[2]->mediator->getName(medMap[2]->mediator), "mediator3") == 0);
    assert(obsMap[0]->observers[2]->getContext(obsMap[0]->observers[2]) == medMap[2]->mediator);
    assert(medMap[3]->key == NULL); // mediatorMap key
    assert(obsMap[0]->key != NULL); // observerMap key

    // Remove the last mediator3 and verify the remaining mediators 0, 2 stay in place
    struct IMediator *removedMediator3 = NULL;
    assert(view->removeMediator(view, "mediator3", &removedMediator3) == true);;
    assert(strcmp(removedMediator3->getName(removedMediator3), "mediator3") == 0);
    assert(strcmp(medMap[0]->mediator->getName(medMap[0]->mediator), "mediator0") == 0);
    assert(obsMap[0]->observers[0]->getContext(obsMap[0]->observers[0]) == medMap[0]->mediator);
    assert(strcmp(medMap[1]->mediator->getName(medMap[1]->mediator), "mediator2") == 0);
    assert(obsMap[0]->observers[1]->getContext(obsMap[0]->observers[1]) == medMap[1]->mediator);
    assert(medMap[2]->key == NULL);
    assert(medMap[3]->key == NULL);
    assert(obsMap[0]->key != NULL); // ObserverMap key will persist while observers exist

    // Remove the first mediator0 and verify that subsequent mediator2 shift left
    struct IMediator *removedMediator0 = NULL;
    assert(view->removeMediator(view, "mediator0", &removedMediator0) == true);;
    assert(strcmp(removedMediator0->getName(removedMediator0), "mediator0") == 0);
    assert(strcmp(medMap[0]->mediator->getName(medMap[0]->mediator), "mediator2") == 0);
    assert(obsMap[0]->observers[0]->getContext(obsMap[0]->observers[0]) == medMap[0]->mediator);

    assert(medMap[1]->key == NULL);
    assert(medMap[2]->key == NULL);
    assert(medMap[3]->key == NULL);
    assert(obsMap[0]->key != NULL); // ObserverMap key will persist while observers exist

    // Remove the last mediator2 and confirm that the dictionary key is cleared
    struct IMediator *removedMediator2 = NULL;
    assert(view->removeMediator(view, "mediator2", &removedMediator2) == true);
    assert(strcmp(removedMediator2->getName(removedMediator2), "mediator2") == 0);
    assert(medMap[1]->key == NULL);
    assert(medMap[2]->key == NULL);
    assert(medMap[3]->key == NULL);
    assert(obsMap[0]->key == NULL); // ObserverMap key cleared since no observers left

    assert(puremvc_view_removeView("ViewTestKey17", NULL) == true);
}

void testViewMapShiftLeft() {
    struct ViewMap **instanceMap = (struct ViewMap *[]) {
        &(struct ViewMap) { .view = alloca(puremvc_view_size()) },
        &(struct ViewMap) { .view = alloca(puremvc_view_size()) },
        &(struct ViewMap) { .view = alloca(puremvc_view_size()) },
        &(struct ViewMap) { .view = alloca(puremvc_view_size()) },
        NULL
    };

    // create 4 instances
    puremvc_view_getInstance(instanceMap, "view0");
    assert(strcmp(instanceMap[0]->key, "view0") == 0);
    const char **key0 = (const char **)((char *) instanceMap[0]->view + sizeof(struct IView));
    assert(strcmp(*key0, "view0") == 0);

    puremvc_view_getInstance(instanceMap, "view1");
    assert(strcmp(instanceMap[1]->key, "view1") == 0);
    const char **key1 = (const char **)((char *) instanceMap[1]->view + sizeof(struct IView));
    assert(strcmp(*key1, "view1") == 0);

    puremvc_view_getInstance(instanceMap, "view2");
    assert(strcmp(instanceMap[2]->key, "view2") == 0);
    const char **key2 = (const char **)((char *) instanceMap[2]->view + sizeof(struct IView));
    assert(strcmp(*key2, "view2") == 0);

    puremvc_view_getInstance(instanceMap, "view3");
    assert(strcmp(instanceMap[3]->key, "view3") == 0);
    const char **key3 = (const char **)((char *) instanceMap[3]->view + sizeof(struct IView));
    assert(strcmp(*key3, "view3") == 0);

    // remove
    struct IView *view1 = NULL; // remove middle view1, remaining 0, 2, 3
    assert(puremvc_view_removeView("view1", &view1) == true);
    assert(strcmp(instanceMap[0]->key, "view0") == 0);
    assert(strcmp(instanceMap[1]->key, "view2") == 0);
    assert(strcmp(instanceMap[2]->key, "view3") == 0);
    assert(instanceMap[3]->key == NULL);
    assert(instanceMap[4] == NULL);

    struct IView *view3 = NULL; // remove last view3, remaining 0, 2
    assert(puremvc_view_removeView("view3", &view3) == true);
    assert(strcmp(instanceMap[0]->key, "view0") == 0);
    assert(strcmp(instanceMap[1]->key, "view2") == 0);
    assert(instanceMap[2]->key == NULL);
    assert(instanceMap[3]->key == NULL);
    assert(instanceMap[4] == NULL);

    struct IView *view0 = NULL; // remove first, remaining 2
    assert(puremvc_view_removeView("view0", &view0) == true);
    assert(strcmp(instanceMap[0]->key, "view2") == 0);
    assert(instanceMap[1]->key == NULL);
    assert(instanceMap[2]->key == NULL);
    assert(instanceMap[3]->key == NULL);
    assert(instanceMap[4] == NULL);

    struct IView *view2 = NULL; // remove remaining
    assert(puremvc_view_removeView("view2", &view2) == true);
    assert(instanceMap[0]->key == NULL);
    assert(instanceMap[1]->key == NULL);
    assert(instanceMap[2]->key == NULL);
    assert(instanceMap[3]->key == NULL);
    assert(instanceMap[4] == NULL);
}

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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void beforeAll() {}

static void beforeEach() {
    puremvc_view_reset();
}

static void afterEach() {
    puremvc_view_reset();
}

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
    printf("\033[1;36m[SUITE] %s\033[0m\n", "ViewTest");
    printf("\033[1;36m================================================\033[0m\n\n");

    test("testGetInstance", testGetInstance);
    test("testRegisterAndNotifyObserver", testRegisterAndNotifyObserver); //
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

void handleNotification(const void *context, const struct INotification *notification) {
    (void)context;
    viewTestVar = (struct ViewTestVar *) notification->getBody(notification);
}

void testGetInstance() {
    struct ViewMap **instanceMap = (struct ViewMap *[]) {
        &(struct ViewMap){ .view = alloca(puremvc_view_size()) },
        NULL
    };

    // Test Factory Method
    struct IView *view = puremvc_view_getInstance(instanceMap, "ViewTestKey1");
    if (view == NULL) abort();

    view->initializeView(view, NULL, NULL);

    // test assertions
    if (view == NULL) abort();
    if (view != puremvc_view_getInstance(instanceMap, "ViewTestKey1")) abort();
    if (strcmp(instanceMap[0]->key, "ViewTestKey1") != 0) abort();

    if (puremvc_view_removeView("ViewTestKey1", NULL) != true) abort();
    if (instanceMap[0]->key != NULL) abort();
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
    if (view == NULL) abort();

    view->initializeView(view, observerMap, NULL);

    // Register Observer's interest in a particular Notification with the View, passing in notification method and context
    if (view->registerObserver(view, "ViewTestNote1", (void (*)(const void *, const struct INotification *)) handleNotification, &viewComponent) != true)
        abort();

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
    if (viewTestVar->value != 10) abort();

    // remove first observer
    viewTestVar->value = 0;
    if (view->removeObserver(view, "ViewTestNote1", &viewComponent) != true) abort();;
    view->notifyObservers(view, notification);
    if (viewTestVar->value != 0) abort();

    if (puremvc_view_removeView("ViewTestKey2", NULL) != true) abort();
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
    if (view == NULL) abort();

    view->initializeView(view, observerMap, mediatorMap);

    size_t offset1 = sizeof(struct IView) + sizeof(const char *);
    struct ObserverMap ***ppp1 = (struct ObserverMap ***)((char *) view + offset1);
    struct ObserverMap **obsMap = *ppp1;

    size_t offset2 = offset1 + sizeof(struct ObserverMap **);
    struct MediatorMap ***ppp2 = (struct MediatorMap ***) ((char *) view + offset2);
    struct MediatorMap **medMap = *ppp2;

    // Create and register the test mediator
    struct ViewComponent viewComponent = {0};
    if (view->registerMediator(view, puremvc_mediator_init, "testing", &viewComponent) != true) abort();;
    if (strcmp(medMap[0]->key, "testing") != 0) abort();
    if (strcmp(medMap[0]->mediator->getName(medMap[0]->mediator), "testing") != 0) abort();

    // Retrieve the mediator
    const struct IMediator *mediator = view->retrieveMediator(view, "testing");

    // test assertions
    if (mediator == NULL) abort();
    if (mediator->getComponent(mediator) != &viewComponent) abort();
    if (strcmp(mediator->getName(mediator), "testing") != 0) abort();

    struct IMediator *removedMediator = NULL;
    if (view->removeMediator(view, "testing", &removedMediator) != true) abort();
    if (strcmp(removedMediator->getName(removedMediator), "testing") != 0) abort();

    if (view->retrieveMediator(view, "testing") != NULL) abort();;

    if (puremvc_view_removeView("ViewTestKey3", NULL) != true) abort();
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
    if (view == NULL) abort();

    view->initializeView(view, NULL, mediatorMap);

    // Create and register the test mediator
    struct ViewComponent viewComponent = {0};
    if (view->registerMediator(view, puremvc_mediator_init, "hasMediatorTest", &viewComponent) != true)
        abort();

    // assert that the view.hasMediator method returns true
    // for that mediator name
    if (view->hasMediator(view, "hasMediatorTest") != true) abort();

    struct IMediator *mediator = NULL;
    if (view->removeMediator(view, "hasMediatorTest", &mediator) != true) abort();
    if (strcmp(mediator->getName(mediator), "hasMediatorTest") != 0) abort();

    // assert that the view.hasMediator method returns false
    // for that mediator name
    if (view->hasMediator(view, "hasMediatorTest") != false) abort();

    if (puremvc_view_removeView("ViewTestKey4", NULL) != true)

        abort();
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
    if (view->registerMediator(view, puremvc_mediator_init, "testing", &viewComponent) != true) abort();
    if (view->hasMediator(view, "testing") != true) abort();;

    // Remove the mediator
    struct IMediator *removedMediator = NULL;
    if (view->removeMediator(view, "testing", &removedMediator) != true) abort();
    if (view->hasMediator(view, "testing") != false) abort();

    // assert that we have removed the appropriate mediator
    if (strcmp(removedMediator->getName(removedMediator), "testing") != 0) abort();

    // assert that the mediator is no longer retrievable
    if (view->retrieveMediator(view, "testing") != NULL) abort();
    if (view->removeMediator(view, "testing", NULL) != false) abort();

    if (puremvc_view_removeView("ViewTestKey5", NULL) != true) abort();
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

    if (view == NULL) abort();

    struct ViewTest viewTest = {"", false, false, 0};

    if (view->registerMediator(view, view_test_mediator4, view_test_mediator4_NAME, &viewTest) != true)
        abort();

    if (view->hasMediator(view, view_test_mediator4_NAME) != true) abort();

    const struct IMediator *mediator = view->retrieveMediator(view, view_test_mediator4_NAME);
    if (mediator == NULL) abort();

    // assert that onRegister was called, and the mediator responded by setting our boolean
    if(!viewTest.onRegisterCalled) abort();
    if (((struct ViewTest *) mediator->getComponent(mediator))->onRegisterCalled != true) abort();

    // Remove the component
    struct IMediator *removedMediator = NULL;
    if (view->removeMediator(view, view_test_mediator4_NAME, &removedMediator) != true) abort();;
    if (removedMediator == NULL) abort();
    if (strcmp(removedMediator->getName(removedMediator), view_test_mediator4_NAME) != 0) abort();
    if (view->hasMediator(view, view_test_mediator4_NAME) != false) abort();

    // assert that the mediator is no longer retrievable
    if (view->retrieveMediator(view, view_test_mediator4_NAME) != NULL) abort();

    // assert that onRemove was called, and the mediator responded by setting our boolean
    if(!viewTest.onRemoveCalled) abort();
    if (((struct ViewTest *) mediator->getComponent(mediator))->onRemoveCalled != true) abort();

    // clean up
    if (puremvc_view_removeView("ViewTestKey6", NULL) != true) abort();
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
    if (view == NULL) abort();

    view->initializeView(view, observerMap, mediatorMap);

    // Create and register the test mediator,
    // but not so we have a reference to it
    struct ViewComponent viewComponent = {0};
    view->registerMediator(view, view_test_mediator, view_test_mediator_NAME, &viewComponent);

    // test that we can retrieve it
    if (view->retrieveMediator(view, view_test_mediator_NAME) == NULL) abort();

    // assertions
    if (view->hasMediator(view, view_test_mediator_NAME) != true) abort(); // todo remove observer true

    // Remove the Mediator
    struct IMediator *removedMediator = NULL;
    if (view->removeMediator(view, view_test_mediator_NAME, &removedMediator) != true) abort();
    if (view->hasMediator(view, view_test_mediator_NAME) != false) abort();

    // confirm observers have already been removed as a result of removeMediator
    if (view->removeObserver(view, "DEF", removedMediator) != false) abort();
    if (view->removeObserver(view, "GHI", removedMediator) != false) abort();
    if (view->removeObserver(view, "ABC", removedMediator) != false) abort();

    // test that retrieving it now returns null
    if (view->removeMediator(view, view_test_mediator_NAME, NULL) != false) abort();;

    // test that removing the mediator again once its gone doesn't cause crash
    if (view->removeMediator(view, view_test_mediator_NAME, NULL) != false) abort();

    // Create and register another instance of the test mediator,
    if (view->registerMediator(view, view_test_mediator, view_test_mediator_NAME, &viewComponent) != true)
        abort();
    if (view->hasMediator(view, view_test_mediator_NAME) != true) abort();
    if (view->retrieveMediator(view, view_test_mediator_NAME) == NULL) abort();
    struct IMediator *retrievedMediator = view->retrieveMediator(view, view_test_mediator_NAME);
    if (strcmp(retrievedMediator->getName(retrievedMediator), view_test_mediator_NAME) != 0) abort();

    // Remove the Mediator
    removedMediator = NULL;
    if (view->removeMediator(view, view_test_mediator_NAME, &removedMediator) != true) abort();
    if (strcmp(removedMediator->getName(removedMediator), view_test_mediator_NAME) != 0) abort();
    if (view->hasMediator(view, view_test_mediator_NAME) != false) abort();
    if (view->removeObserver(view, "DEF", removedMediator) != false) abort(); // already removed
    if (view->removeObserver(view, "GHI", removedMediator) != false) abort();
    if (view->removeObserver(view, "ABC", removedMediator) != false) abort();

    // test that retrieving it now returns null
    if (view->retrieveMediator(view, view_test_mediator_NAME) != NULL) abort();

    if (puremvc_view_removeView("ViewTestKey7", NULL) != true) abort();
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
    if (view == NULL) abort();

    view->initializeView(view, observerMap, mediatorMap);

    // Create and register the test mediator to be removed.
    struct ViewTest viewTest = {.lastNotification = NULL };
    view->registerMediator(view, view_test_mediator2, view_test_mediator2_NAME, &viewTest);

    // test that we can retrieve it
    if (view->retrieveMediator(view, view_test_mediator2_NAME) == NULL) abort();

    const struct IMediator *mediator = view->retrieveMediator(view, view_test_mediator2_NAME);

    // assertions
    if (strcmp(mediator->getName(mediator), view_test_mediator2_NAME) != 0) abort();

    // test that notifications work
    struct INotification *notification1 = puremvc_notification_init(alloca(puremvc_notification_size()), NOTE1, NULL, NULL);
    view->notifyObservers(view, notification1);
    if (strcmp(viewTest.lastNotification, NOTE1) != 0) abort();

    struct INotification *notification2 = puremvc_notification_init(alloca(puremvc_notification_size()), NOTE2, NULL, NULL);
    view->notifyObservers(view, notification2);
    if (strcmp(viewTest.lastNotification, NOTE2) != 0) abort();

    struct IMediator *removedMediator = NULL;
    if (view->removeMediator(view, view_test_mediator2_NAME, &removedMediator) != true) abort();;
    if (strcmp(removedMediator->getName(removedMediator), view_test_mediator2_NAME) != 0) abort();
    if (view->hasMediator(view, view_test_mediator_NAME) != false) abort();
    if (view->removeObserver(view, NOTE1, removedMediator) != false) abort();
    if (view->removeObserver(view, NOTE2, removedMediator) != false) abort();

    if (view->retrieveMediator(view, view_test_mediator2_NAME) != NULL)

        abort();

    viewTest.lastNotification = "";

    view->notifyObservers(view, notification1);
    if (strcmp(viewTest.lastNotification, NOTE1) == 0) abort();

    view->notifyObservers(view, notification2);
    if (strcmp(viewTest.lastNotification, NOTE2) == 0) abort();

    if (puremvc_view_removeView("ViewTestKey8", NULL) != true) abort();
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
    if (view == NULL) abort();

    view->initializeView(view, observerMap, mediatorMap);

    struct ViewTest viewTest = {};

    // Create and register that responds to notifications 1 and 2
    if (view->registerMediator(view, view_test_mediator2, view_test_mediator2_NAME, &viewTest) != true)
        abort();

    // assertions
    if (view->hasMediator(view, view_test_mediator2_NAME) != true) abort();

    // Create and register that responds to notification 3
    if (view->registerMediator(view, view_test_mediator3, view_test_mediator3_NAME, &viewTest) != true)
        abort();

    // assertions
    if (view->hasMediator(view, view_test_mediator3_NAME) != true) abort();

    // test that all notifications work
    struct INotification *notification1 = puremvc_notification_init(alloca(puremvc_notification_size()), NOTE1, NULL, NULL);
    view->notifyObservers(view, notification1);
    if (strcmp(viewTest.lastNotification, NOTE1) != 0) abort();

    struct INotification *notification2 = puremvc_notification_init(alloca(puremvc_notification_size()), NOTE2, NULL, NULL);
    view->notifyObservers(view, notification2);
    if (strcmp(viewTest.lastNotification, NOTE2) != 0) abort();

    struct INotification *notification3 = puremvc_notification_init(alloca(puremvc_notification_size()), NOTE3, NULL, NULL);
    view->notifyObservers(view, notification3);
    if (strcmp(viewTest.lastNotification, NOTE3) != 0) abort();

    // Remove the Mediator that responds to 1 and 2
    struct IMediator *removedMediator2 = NULL;
    if (view->removeMediator(view, view_test_mediator2_NAME, &removedMediator2) != true) abort();
    if (view->hasMediator(view, view_test_mediator2_NAME) != false) abort();
    if (view->removeObserver(view, NOTE1, removedMediator2) != false) abort();
    if (view->removeObserver(view, NOTE2, removedMediator2) != false) abort();

    // test that retrieving it now returns null
    if (view->retrieveMediator(view, view_test_mediator2_NAME) != NULL) abort();

    // test that notifications no longer work
    // for notifications 1 and 2, but still work for 3
    viewTest.lastNotification = "";

    view->notifyObservers(view, notification1);
    if (strcmp(viewTest.lastNotification, NOTE1) == 0) abort();

    view->notifyObservers(view, notification2);
    if (strcmp(viewTest.lastNotification, NOTE2) == 0) abort();

    view->notifyObservers(view, notification3);
    if (strcmp(viewTest.lastNotification, NOTE3) != 0) abort();

    // Remove the Mediator that responds to 3
    struct IMediator *removedMediator3 = NULL;
    if (view->removeMediator(view, view_test_mediator3_NAME, &removedMediator3) != true) abort();;
    if (strcmp(removedMediator3->getName(removedMediator3), view_test_mediator3_NAME) != 0) abort();
    if (view->hasMediator(view, view_test_mediator3_NAME) != false) abort();
    if (view->removeObserver(view, NOTE3, removedMediator3) != false) abort();

    if (puremvc_view_removeView("ViewTestKey9", NULL) != true) abort();
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
    if (view == NULL) abort();

    view->initializeView(view, observerMap, mediatorMap);

    // Create the counter
    struct ViewTest viewTest = { .counter = 0 };

    // Create and register that responds to notification 5
    if (view->registerMediator(view, view_test_mediator5, view_test_mediator5_NAME, &viewTest) != true)
        abort();;

    // try to register another instance of that mediator (uses the same NAME constant). It will trigger exists warning
    if (view->registerMediator(view, view_test_mediator5, view_test_mediator5_NAME, &viewTest) != false)
        abort();

    // assertions
    if (view->hasMediator(view, view_test_mediator5_NAME) != true) abort();

    // test that the counter is only incremented once (mediator 5's response)
    viewTest.counter = 0;
    const struct INotification *notification = puremvc_notification_init(alloca(puremvc_notification_size()), NOTE5, NULL, NULL);
    view->notifyObservers(view, notification);
    if (viewTest.counter != 1) abort();

    // Remove the Mediator
    struct IMediator *removedMediator = NULL;
    if (view->removeMediator(view, view_test_mediator5_NAME, &removedMediator) != true) abort();
    if (strcmp(removedMediator->getName(removedMediator), view_test_mediator5_NAME) != 0) abort();
    if (view->hasMediator(view, view_test_mediator5_NAME) != false) abort();
    if (view->removeObserver(view, NOTE5, removedMediator) != false) abort();

    // test that retrieving it now returns null
    if (view->retrieveMediator(view, view_test_mediator5_NAME) != NULL) abort();

    // test that the counter is no longer incremented
    viewTest.counter = 0;
    view->notifyObservers(view, notification);
    if (viewTest.counter != 0) abort();

    if (puremvc_view_removeView("ViewTestKey10", NULL) != true) abort();
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
    if (view == NULL) abort();

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
    if (view->registerMediator(view, view_test_mediator6, "view_test_mediator6/1", &viewTest) != true)
        abort();
    if (view->registerMediator(view, view_test_mediator6, "view_test_mediator6/2", &viewTest) != true)
        abort();
    if (view->registerMediator(view, view_test_mediator6, "view_test_mediator6/3", &viewTest) != true)
        abort();
    if (view->registerMediator(view, view_test_mediator6, "view_test_mediator6/4", &viewTest) != true)
        abort();
    if (view->registerMediator(view, view_test_mediator6, "view_test_mediator6/5", &viewTest) != true)
        abort();
    if (view->registerMediator(view, view_test_mediator6, "view_test_mediator6/6", &viewTest) != true)
        abort();
    if (view->registerMediator(view, view_test_mediator6, "view_test_mediator6/7", &viewTest) != true)
        abort();
    if (view->registerMediator(view, view_test_mediator6, "view_test_mediator6/8", &viewTest) != true)
        abort();

    // assertions
    for (size_t i = 0; i < 8; i++) {
        char mediator_name[32] = {0};
        snprintf(mediator_name, 32, "view_test_mediator6/%zu", i + 1);
        if (view->hasMediator(view, mediator_name) != true) abort();
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
        if (strcmp(viewTest.deferred[i], mediator_name) != 0) abort();
    }

    // iterate through deferred names and call removeMediator
    for (size_t i = 0; viewTest.deferred[i] != NULL; i++) {
        const char *mediatorName = viewTest.deferred[i];
        if (mediatorName == NULL) abort();
        struct IMediator *removedMediator = NULL;
        if (view->removeMediator(view, mediatorName, &removedMediator) != true) abort();
        if (strcmp(removedMediator->getName(removedMediator), mediatorName) != 0) abort();
    }

    // verify the count is correct
    if (viewTest.counter != 8) abort();

    // clear the counter
    viewTest.counter = 0;
    view->notifyObservers(view, notification);

    // verify the count is 0
    if (viewTest.counter != 0) abort();

    if (puremvc_view_removeView("ViewTestKey11", NULL) != true) abort();
}

void testRemoveView() {
    struct ViewMap **instanceMap = (struct ViewMap *[]) {
        &(struct ViewMap){ .view = alloca(puremvc_view_size()) },
        NULL
    };

    // Get a Multiton View instance
    struct IView *view = puremvc_view_getInstance(instanceMap, "ViewTestKey12");
    if (view == NULL) abort();

    // remove the View
    if (puremvc_view_removeView("ViewTestKey12", NULL) != true) abort();

    // re-create the view without throwing an exception
    puremvc_view_getInstance(instanceMap, "ViewTestKey12");

    // try removing again
    if (puremvc_view_removeView("ViewTestKey12", NULL) != true) abort();
}

void testGarbageStorageForView() {
    struct ViewMap *viewMap1[] = { NULL }; // empty view
    const struct IView *view1 = puremvc_view_getInstance(viewMap1, "ViewTestKey13"); // crash test
    if (view1 != NULL) abort();
    if (puremvc_view_removeView("ViewTestKey13", NULL) != true) abort();
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
    if (puremvc_view_removeView("ViewTestKey14", NULL) != true) abort();

    // empty observerMap
    struct ViewMap **instanceMap2 = (struct ViewMap *[]) {  &(struct ViewMap){ .view = alloca(puremvc_view_size()) }, NULL };
    struct ObserverMap **observerMap2 = (struct ObserverMap *[]) { NULL };
    struct IView *view2 = puremvc_view_getInstance(instanceMap2, "ViewTestKey14");
    view2->initializeView(view2, observerMap2, NULL);
    view2->notifyObservers(view2, NULL);
    view2->removeObserver(view2, "ViewTestKey14_testing2", &component);
    view2->registerObserver(view2, "ViewTestKey14_testing2", (void (*)(const void *, const struct INotification *)) handleNotification, &component);
    if (puremvc_view_removeView("ViewTestKey14", NULL) != true) abort();

    // empty ObserverMap field
    struct ViewMap **instanceMap3 = (struct ViewMap *[]) {  &(struct ViewMap){ .view = alloca(puremvc_view_size()) }, NULL };
    struct ObserverMap **observerMap3 = (struct ObserverMap *[]) { &(struct ObserverMap){}, NULL };
    struct IView *view3 = puremvc_view_getInstance(instanceMap3, "ViewTestKey14");
    view3->initializeView(view3, observerMap3, NULL);
    view3->notifyObservers(view3, NULL);
    view3->removeObserver(view3, "ViewTestKey14_testing3", &component);
    view3->registerObserver(view3, "ViewTestKey14_testing3", (void (*)(const void *, const struct INotification *)) handleNotification, &component);
    if (puremvc_view_removeView("ViewTestKey14", NULL) != true) abort();

    // empty observers
    struct ViewMap **instanceMap4 = (struct ViewMap *[]) {  &(struct ViewMap){ .view = alloca(puremvc_view_size()) }, NULL };
    struct IView *view4 = puremvc_view_getInstance(instanceMap4, "ViewTestKey14");
    struct ObserverMap **observerMap4 = (struct ObserverMap *[]) { &(struct ObserverMap){ .observers = (struct IObserver *[]){ NULL } }, NULL };
    view4->initializeView(view4, observerMap4, NULL);
    view4->notifyObservers(view4, NULL);
    view4->removeObserver(view4, "ViewTestKey14_testing4", &component);
    view4->registerObserver(view4, "ViewTestKey14_testing4", (void (*)(const void *, const struct INotification *)) handleNotification, &component);
    if (puremvc_view_removeView("ViewTestKey14", NULL) != true) abort();
}

void testGarbageStorageForMediator() {
    // missing MediatorMap
    struct ViewMap **instanceMap1 = (struct ViewMap *[]) { &(struct ViewMap){ .view = alloca(puremvc_view_size()) }, NULL };
    struct MediatorMap **mediatorMap1 = NULL;
    struct IView *view1 = puremvc_view_getInstance(instanceMap1, "ViewTestKey15");
    view1->initializeView(view1, NULL, mediatorMap1);
    if (view1 == NULL) abort();
    if (view1->registerMediator(view1, puremvc_mediator_init, MEDIATOR_NAME, NULL) != false) abort();
    if (view1->hasMediator(view1, MEDIATOR_NAME) != false) abort();;
    if (view1->retrieveMediator(view1, MEDIATOR_NAME) != NULL) abort();;
    if (view1->removeMediator(view1, MEDIATOR_NAME, NULL) != false) abort();
    if (puremvc_view_removeView("ViewTestKey15", NULL) != true) abort();

    // empty mediatorMap
    struct ViewMap **instanceMap2 = (struct ViewMap *[]) { &(struct ViewMap){ .view = alloca(puremvc_view_size()) }, NULL };
    struct MediatorMap **mediatorMap2 = (struct MediatorMap *[]){ NULL };
    struct IView *view2 = puremvc_view_getInstance(instanceMap2, "ViewTestKey15");
    view2->initializeView(view2, NULL, mediatorMap2);
    if (view2->registerMediator(view2, puremvc_mediator_init, MEDIATOR_NAME, NULL) != false) abort();;
    if (view2->hasMediator(view2, MEDIATOR_NAME) != false) abort();
    if (view2->retrieveMediator(view2, MEDIATOR_NAME) != NULL) abort();;
    if (view2->removeMediator(view2, MEDIATOR_NAME, NULL) != false) abort();
    if (puremvc_view_removeView("ViewTestKey15", NULL) != true) abort();
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
    if (strcmp(obsMap[0]->key, "observer0") != 0) abort();
    if (obsMap[0]->observers[0]->getContext(obsMap[0]->observers[0]) != &component0) abort();

    view->registerObserver(view, "observer1", NULL, (void *) &component1);
    if (strcmp(obsMap[1]->key, "observer1") != 0) abort();
    if (obsMap[1]->observers[0]->getContext(obsMap[1]->observers[0]) != &component1) abort();

    view->registerObserver(view, "observer2", NULL, (void *) &component2);
    if (strcmp(obsMap[2]->key, "observer2") != 0) abort();
    if (obsMap[2]->observers[0]->getContext(obsMap[2]->observers[0]) != &component2) abort();

    view->registerObserver(view, "observer3", NULL, (void *) &component3);
    if (strcmp(obsMap[3]->key, "observer3") != 0) abort();
    if (obsMap[3]->observers[0]->getContext(obsMap[3]->observers[0]) != &component3) abort();

    // remove the middle observer1; remaining entries (2, 3) shift left and the tail is reinit.
    view->removeObserver(view, "observer1", &component1);
    if (strcmp(obsMap[0]->key, "observer0") != 0) abort();
    if (obsMap[0]->observers[0]->getContext(obsMap[0]->observers[0]) != &component0) abort();
    if (strcmp(obsMap[1]->key, "observer2") != 0) abort();
    if (obsMap[1]->observers[0]->getContext(obsMap[1]->observers[0]) != &component2) abort();
    if (strcmp(obsMap[2]->key, "observer3") != 0) abort();
    if (obsMap[2]->observers[0]->getContext(obsMap[2]->observers[0]) != &component3) abort();
    if (obsMap[3]->key != NULL) abort();
    if (obsMap[3]->observers[0]->getContext(obsMap[3]->observers[0]) != NULL) abort();
    if (obsMap[4] != NULL) abort();

    // remove the last observer3 and verify the remaining entries (0, 2) stay in place
    view->removeObserver(view, "observer3", &component3);
    if (strcmp(obsMap[0]->key, "observer0") != 0) abort();
    if (obsMap[0]->observers[0]->getContext(obsMap[0]->observers[0]) != &component0) abort();
    if (strcmp(obsMap[1]->key, "observer2") != 0) abort();
    if (obsMap[1]->observers[0]->getContext(obsMap[1]->observers[0]) != &component2) abort();
    if (obsMap[3]->key != NULL) abort();
    if (obsMap[3]->observers[0]->getContext(obsMap[2]->observers[0]) != NULL) abort();
    if (obsMap[3]->key != NULL) abort();
    if (obsMap[3]->observers[0]->getContext(obsMap[3]->observers[0]) != NULL) abort();
    if (obsMap[4] != NULL) abort();

    // remove the first observer0 and verify that remaining observer shift left
    view->removeObserver(view, "observer0", &component0);
    if (strcmp(obsMap[0]->key, "observer2") != 0) abort();
    if (obsMap[0]->observers[0]->getContext(obsMap[0]->observers[0]) != &component2) abort();
    if (obsMap[1]->key != NULL) abort();
    if (obsMap[1]->observers[0]->getContext(obsMap[1]->observers[0]) != NULL) abort();
    if (obsMap[2]->key != NULL) abort();
    if (obsMap[2]->observers[0]->getContext(obsMap[2]->observers[0]) != NULL) abort();
    if (obsMap[3]->key != NULL) abort();
    if (obsMap[3]->observers[0]->getContext(obsMap[3]->observers[0]) != NULL) abort();
    if (obsMap[4] != NULL) abort();

    // Remove the remaining observer2
    view->removeObserver(view, "observer2", &component2);
    if (obsMap[0]->key != NULL) abort();
    if (obsMap[0]->observers[0]->getContext(obsMap[0]->observers[0]) != NULL) abort();
    if (obsMap[1]->key != NULL) abort();
    if (obsMap[1]->observers[0]->getContext(obsMap[1]->observers[0]) != NULL) abort();
    if (obsMap[2]->key != NULL) abort();
    if (obsMap[2]->observers[0]->getContext(obsMap[2]->observers[0]) != NULL) abort();
    if (obsMap[3]->key != NULL) abort();
    if (obsMap[3]->observers[0]->getContext(obsMap[3]->observers[0]) != NULL) abort();
    if (obsMap[4] != NULL) abort();

    if (puremvc_view_removeView("ViewTestKey16", NULL) != true) abort();
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
    if (view->registerObserver(view, "notification0", NULL, (void *) &context0) != true) abort();
    if (strcmp(obsMap[0]->key, "notification0") != 0) abort(); // key gets created
    if (obsMap[0]->observers[0]->getContext(obsMap[0]->observers[0]) != &context0) abort();

    if (view->registerObserver(view, "notification0", NULL, &context1) != true) abort();
    if (obsMap[0]->observers[1]->getContext(obsMap[0]->observers[1]) != &context1) abort();

    if (view->registerObserver(view, "notification0", NULL, &context2) != true) abort();
    if (obsMap[0]->observers[2]->getContext(obsMap[0]->observers[2]) != &context2) abort();

    if (view->registerObserver(view, "notification0", NULL, &context3) != true) abort();
    if (obsMap[0]->observers[3]->getContext(obsMap[0]->observers[3]) != &context3) abort();
    if (obsMap[0]->observers[4] != NULL) abort();

    // remove the middle context1; remaining entries (2, 3) shift left and the tail is reinit.
    if (view->removeObserver(view, "notification0", &context1) != true) abort();
    if (obsMap[0]->observers[0]->getContext(obsMap[0]->observers[0]) != &context0) abort();
    if (obsMap[0]->observers[1]->getContext(obsMap[0]->observers[1]) != &context2) abort();
    if (obsMap[0]->observers[2]->getContext(obsMap[0]->observers[2]) != &context3) abort();
    if (obsMap[0]->observers[3]->getContext(obsMap[0]->observers[3]) != NULL) abort();
    if (obsMap[0]->observers[4] != NULL) abort();

    // remove the last context3 and verify the remaining entries (0, 2) stay in place
    if (view->removeObserver(view, "notification0", &context3) != true) abort();
    if (obsMap[0]->observers[0]->getContext(obsMap[0]->observers[0]) != &context0) abort();
    if (obsMap[0]->observers[1]->getContext(obsMap[0]->observers[1]) != &context2) abort();
    if (obsMap[0]->observers[2]->getContext(obsMap[0]->observers[2]) != NULL) abort();
    if (obsMap[0]->observers[3]->getContext(obsMap[0]->observers[3]) != NULL) abort();
    if (obsMap[0]->observers[4] != NULL) abort();

    // remove the first context0 and verify that remaining observer shift left
    if (view->removeObserver(view, "notification0", &context0) != true) abort();
    if (obsMap[0]->observers[0]->getContext(obsMap[0]->observers[0]) != &context2) abort();
    if (obsMap[0]->observers[1]->getContext(obsMap[0]->observers[1]) != NULL) abort();
    if (obsMap[0]->observers[2]->getContext(obsMap[0]->observers[2]) != NULL) abort();
    if (obsMap[0]->observers[3]->getContext(obsMap[0]->observers[3]) != NULL) abort();
    if (obsMap[0]->observers[4] != NULL) abort();

    // Remove the remaining context2
    if (view->removeObserver(view, "notification0", &context2) != true) abort();
    if (obsMap[0]->observers[0]->getContext(obsMap[0]->observers[0]) != NULL) abort();
    if (obsMap[0]->observers[1]->getContext(obsMap[0]->observers[1]) != NULL) abort();
    if (obsMap[0]->observers[2]->getContext(obsMap[0]->observers[2]) != NULL) abort();
    if (obsMap[0]->observers[3]->getContext(obsMap[0]->observers[3]) != NULL) abort();
    if (obsMap[0]->observers[4] != NULL) abort();

    if (obsMap[0]->key != NULL) abort(); // key gets deleted

    // Test duplicate context registration and repeated removal:
    if (view->registerObserver(view, "notification0", NULL, &context0) != true) abort();
    if (view->registerObserver(view, "notification0", NULL, &context0) != false) abort();
    if (view->registerObserver(view, "notification0", NULL, &context0) != false) abort();
    if (view->registerObserver(view, "notification0", NULL, &context0) != false) abort();

    if (obsMap[0]->observers[0]->getContext(obsMap[0]->observers[0]) != &context0) abort(); // only one gets registered
    if (obsMap[0]->observers[1]->getContext(obsMap[0]->observers[1]) != NULL) abort();
    if (obsMap[0]->observers[2]->getContext(obsMap[0]->observers[2]) != NULL) abort();
    if (obsMap[0]->observers[3]->getContext(obsMap[0]->observers[3]) != NULL) abort();
    if (obsMap[0]->observers[4] != NULL) abort();

    if (view->removeObserver(view, "notification0", &context0) != true) abort();
    if (view->removeObserver(view, "notification0", &context0) != false) abort();
    if (view->removeObserver(view, "notification0", &context0) != false) abort();
    if (view->removeObserver(view, "notification0", &context0) != false) abort();

    // Removing a context that was never registered
    if (view->removeObserver(view, "notification0", &(struct ViewComponent){0}) != false) abort();
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
    if (strcmp(medMap[0]->mediator->getName(medMap[0]->mediator), "mediator0") != 0) abort();
    if (obsMap[0]->observers[0]->getContext(obsMap[0]->observers[0]) != medMap[0]->mediator) abort();

    view->registerMediator(view, view_test_mediator7, "mediator1", NULL);
    if (strcmp(medMap[1]->mediator->getName(medMap[1]->mediator), "mediator1") != 0) abort();
    if (obsMap[0]->observers[1]->getContext(obsMap[0]->observers[1]) != medMap[1]->mediator) abort();

    view->registerMediator(view, view_test_mediator7, "mediator2", NULL);
    if (strcmp(medMap[2]->mediator->getName(medMap[2]->mediator), "mediator2") != 0) abort();
    if (obsMap[0]->observers[2]->getContext(obsMap[0]->observers[2]) != medMap[2]->mediator) abort();

    view->registerMediator(view, view_test_mediator7, "mediator3", NULL);
    if (strcmp(medMap[3]->mediator->getName(medMap[3]->mediator), "mediator3") != 0) abort();
    if (obsMap[0]->observers[3]->getContext(obsMap[0]->observers[3]) != medMap[3]->mediator) abort();

    // Verify the dictionary key for the map is correctly set
    if (strcmp(obsMap[0]->key, NOTE7) != 0) abort();

    // Remove the second mediator1 (middle) and verify that remaining mediators 2, 3 are shifted correctly
    struct IMediator *removedMediator1 = NULL;
    if (view->removeMediator(view, "mediator1", &removedMediator1) != true) abort();
    if (strcmp(removedMediator1->getName(removedMediator1), "mediator1") != 0) abort();
    if (strcmp(medMap[0]->key, "mediator0") != 0) abort();
    if (strcmp(medMap[0]->mediator->getName(medMap[0]->mediator), "mediator0") != 0) abort();
    if (obsMap[0]->observers[0]->getContext(obsMap[0]->observers[0]) != medMap[0]->mediator) abort();
    if (strcmp(medMap[1]->key, "mediator2") != 0) abort();
    if (strcmp(medMap[1]->mediator->getName(medMap[1]->mediator), "mediator2") != 0) abort();
    if (obsMap[0]->observers[1]->getContext(obsMap[0]->observers[1]) != medMap[1]->mediator) abort();
    if (strcmp(medMap[2]->key, "mediator3") != 0) abort();
    if (strcmp(medMap[2]->mediator->getName(medMap[2]->mediator), "mediator3") != 0) abort();
    if (obsMap[0]->observers[2]->getContext(obsMap[0]->observers[2]) != medMap[2]->mediator) abort();
    if (medMap[3]->key != NULL) abort(); // mediatorMap key
    if (obsMap[0]->key == NULL) abort(); // observerMap key

    // Remove the last mediator3 and verify the remaining mediators 0, 2 stay in place
    struct IMediator *removedMediator3 = NULL;
    if (view->removeMediator(view, "mediator3", &removedMediator3) != true) abort();;
    if (strcmp(removedMediator3->getName(removedMediator3), "mediator3") != 0) abort();
    if (strcmp(medMap[0]->mediator->getName(medMap[0]->mediator), "mediator0") != 0) abort();
    if (obsMap[0]->observers[0]->getContext(obsMap[0]->observers[0]) != medMap[0]->mediator) abort();
    if (strcmp(medMap[1]->mediator->getName(medMap[1]->mediator), "mediator2") != 0) abort();
    if (obsMap[0]->observers[1]->getContext(obsMap[0]->observers[1]) != medMap[1]->mediator) abort();
    if (medMap[2]->key != NULL) abort();
    if (medMap[3]->key != NULL) abort();
    if (obsMap[0]->key == NULL) abort(); // ObserverMap key will persist while observers exist

    // Remove the first mediator0 and verify that subsequent mediator2 shift left
    struct IMediator *removedMediator0 = NULL;
    if (view->removeMediator(view, "mediator0", &removedMediator0) != true) abort();;
    if (strcmp(removedMediator0->getName(removedMediator0), "mediator0") != 0) abort();
    if (strcmp(medMap[0]->mediator->getName(medMap[0]->mediator), "mediator2") != 0) abort();
    if (obsMap[0]->observers[0]->getContext(obsMap[0]->observers[0]) != medMap[0]->mediator) abort();

    if (medMap[1]->key != NULL) abort();
    if (medMap[2]->key != NULL) abort();
    if (medMap[3]->key != NULL) abort();
    if (obsMap[0]->key == NULL) abort(); // ObserverMap key will persist while observers exist

    // Remove the last mediator2 and confirm that the dictionary key is cleared
    struct IMediator *removedMediator2 = NULL;
    if (view->removeMediator(view, "mediator2", &removedMediator2) != true) abort();
    if (strcmp(removedMediator2->getName(removedMediator2), "mediator2") != 0) abort();
    if (medMap[1]->key != NULL) abort();
    if (medMap[2]->key != NULL) abort();
    if (medMap[3]->key != NULL) abort();
    if (obsMap[0]->key != NULL) abort(); // ObserverMap key cleared since no observers left

    if (puremvc_view_removeView("ViewTestKey17", NULL) != true) abort();
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
    if (strcmp(instanceMap[0]->key, "view0") != 0) abort();
    const char **key0 = (const char **)((char *) instanceMap[0]->view + sizeof(struct IView));
    if (strcmp(*key0, "view0") != 0) abort();

    puremvc_view_getInstance(instanceMap, "view1");
    if (strcmp(instanceMap[1]->key, "view1") != 0) abort();
    const char **key1 = (const char **)((char *) instanceMap[1]->view + sizeof(struct IView));
    if (strcmp(*key1, "view1") != 0) abort();

    puremvc_view_getInstance(instanceMap, "view2");
    if (strcmp(instanceMap[2]->key, "view2") != 0) abort();
    const char **key2 = (const char **)((char *) instanceMap[2]->view + sizeof(struct IView));
    if (strcmp(*key2, "view2") != 0) abort();

    puremvc_view_getInstance(instanceMap, "view3");
    if (strcmp(instanceMap[3]->key, "view3") != 0) abort();
    const char **key3 = (const char **)((char *) instanceMap[3]->view + sizeof(struct IView));
    if (strcmp(*key3, "view3") != 0) abort();

    // remove
    struct IView *view1 = NULL; // remove middle view1, remaining 0, 2, 3
    if (puremvc_view_removeView("view1", &view1) != true) abort();
    if (strcmp(instanceMap[0]->key, "view0") != 0) abort();
    if (strcmp(instanceMap[1]->key, "view2") != 0) abort();
    if (strcmp(instanceMap[2]->key, "view3") != 0) abort();
    if (instanceMap[3]->key != NULL) abort();
    if (instanceMap[4] != NULL) abort();

    struct IView *view3 = NULL; // remove last view3, remaining 0, 2
    if (puremvc_view_removeView("view3", &view3) != true) abort();
    if (strcmp(instanceMap[0]->key, "view0") != 0) abort();
    if (strcmp(instanceMap[1]->key, "view2") != 0) abort();
    if (instanceMap[2]->key != NULL) abort();
    if (instanceMap[3]->key != NULL) abort();
    if (instanceMap[4] != NULL) abort();

    struct IView *view0 = NULL; // remove first, remaining 2
    if (puremvc_view_removeView("view0", &view0) != true) abort();
    if (strcmp(instanceMap[0]->key, "view2") != 0) abort();
    if (instanceMap[1]->key != NULL) abort();
    if (instanceMap[2]->key != NULL) abort();
    if (instanceMap[3]->key != NULL) abort();
    if (instanceMap[4] != NULL) abort();

    struct IView *view2 = NULL; // remove remaining
    if (puremvc_view_removeView("view2", &view2) != true) abort();
    if (instanceMap[0]->key != NULL) abort();
    if (instanceMap[1]->key != NULL) abort();
    if (instanceMap[2]->key != NULL) abort();
    if (instanceMap[3]->key != NULL) abort();
    if (instanceMap[4] != NULL) abort();
}

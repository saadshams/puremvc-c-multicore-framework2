#include "puremvc/view.h"
#include "puremvc/notification.h"
#include "puremvc/mediator.h"
#include "puremvc/observer.h"

#include "view_test.h"

#include "view_test_mediator.h"
#include "view_test_mediator2.h"
#include "view_test_mediator3.h"
#include "view_test_mediator4.h"
#include "view_test_mediator5.h"
#include "view_test_mediator6.h"
#include "view_test_mediator7.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main() {
    testGetInstance();
    testRegisterAndNotifyObserver();
    testRegisterAndRetrieveMediator();
    testHasMediator();
    testRegisterAndRemoveMediator();
    testOnRegisterAndOnRemove();
    testSuccessiveRegisterAndRemoveMediator();
    testRemoveMediatorAndSubsequentNotify();
    testRemoveOneOfTwoMediatorsAndSubsequentNotify();
    testMediatorReregistration();
    testModifyObserverListDuringNotification();
    testRemoveView();
    testGarbageStorageForView();
    testGarbageStorageForObserver();
    testGarbageStorageForMediator();
    testObserverMapShiftLeft();
    testObserverShiftLeft();
    testMediatorMapShiftLeft();
    testViewShiftLeft();
    return 0;
}

struct ViewTestVar *viewTestVar;

void handleNotification(const void *context, struct INotification notification) {
    (void)context;
    viewTestVar = (struct ViewTestVar *)notification.getBody(&notification);
}

void testGetInstance() {
    // Test Factory Method
    struct ViewMap *storage[] = { &(struct ViewMap) {
        .key = "",
        .view = { .multitonKey = "", .observerMap = {}, .mediatorMap = {} },
    }, NULL };

    const struct IView *view = puremvc_view_getInstance(storage, "ViewTestKey1");

    // test assertions
    assert(view == puremvc_view_getInstance(storage, "ViewTestKey1"));
    assert(strcmp(storage[0]->key, "ViewTestKey1") == 0);
    assert(strcmp(storage[0]->view.multitonKey, "ViewTestKey1") == 0);

    puremvc_view_removeView(storage, "ViewTestKey1");
    assert(storage[0]->key[0] == '\0');
    assert(storage[0]->view.multitonKey[0] == '\0');
}

void testRegisterAndNotifyObserver() {
    struct ViewMap *storage[] = { &(struct ViewMap) {
        .view = {
            .observerMap = (struct ObserverMap *[]) { &(struct ObserverMap) { .observers = (struct IObserver*[]){ &(struct Observer){0}.base, NULL } }, NULL },
        }
    }, NULL };

    struct ViewComponent viewComponent = {0};

    // Get the Multiton View instance
    const struct IView *view = puremvc_view_getInstance(storage, "ViewTestKey2");

    // Create observer, passing in notification method and context
    puremvc_observer_init(&(struct Observer){0}, (void (*)(const void *, struct INotification *)) handleNotification, &viewComponent);

    // Register Observer's interest in a particular Notification with the View
    view->registerObserver(view, "ViewTestNote1", (void (*)(const void *, struct INotification *)) handleNotification, &viewComponent);
    // view->registerObserver(view, "ViewTestNote1", observer);

    // Create a ViewTestNote, setting
    // a body value, and tell the View to notify
    // Observers. Since the Observer is this class
    // and the notification method is viewTestMethod,
    // successful notification will result in our local
    // viewTestVar being set to the value we pass in
    // on the note body.
    struct ViewTestVar vo = {.value = 10};
    struct INotification *notification = puremvc_notification_init(&(struct Notification){0}, "ViewTestNote1", &vo, NULL);
    view->notifyObservers(view, notification);

    // test assertions
    assert(viewTestVar->value == 10);

    // remove first observer
    viewTestVar->value = 0;
    view->removeObserver(view, "ViewTestNote1", &viewComponent);
    view->notifyObservers(view, notification);
    assert(viewTestVar->value == 0);

    puremvc_view_removeView(storage, "ViewTestKey2");
    view = NULL;
}

void testRegisterAndRetrieveMediator() {
    struct ViewMap *storage[] = { &(struct ViewMap) {
        .view = {
            .mediatorMap = (struct MediatorMap *[]) { &(struct MediatorMap){ .mediator = &(struct Mediator){0}.base }, NULL },
            .observerMap = (struct ObserverMap *[]) { &(struct ObserverMap) { .observers = (struct IObserver*[]){ &(struct Observer){0}.base, NULL } }, NULL },
        }
    }, NULL };

    // Get the Multiton View instance
    const struct IView *view = puremvc_view_getInstance(storage, "ViewTestKey3");

    // Create and register the test mediator
    struct ViewComponent viewComponent = {0};
    view->registerMediator(view, puremvc_mediator_init, "testing", &viewComponent);
    assert(strcmp(storage[0]->view.mediatorMap[0]->key, "testing") == 0);
    assert(strcmp(storage[0]->view.mediatorMap[0]->mediator->getName(storage[0]->view.mediatorMap[0]->mediator), "testing") == 0);

    // Retrieve the mediator
    const struct IMediator *mediator = view->retrieveMediator(view, "testing");

    // test assertions
    assert(mediator != NULL);
    assert(mediator->getComponent(mediator) == &viewComponent);
    assert(strcmp(mediator->getName(mediator), "testing") == 0);

    // clean up
    const struct Mediator removedMediator = view->removeMediator(view, "testing");
    assert(storage[0]->view.mediatorMap[0]->key[0] == '\0');
    assert(strcmp(removedMediator.name, "testing") == 0);

    puremvc_view_removeView(storage, "ViewTestKey3");
    view = NULL;
}

void testHasMediator() {
    struct ViewMap *storage[] = { &(struct ViewMap) {
        .view = { .mediatorMap = (struct MediatorMap *[]) { &(struct MediatorMap){ .mediator = &(struct Mediator){}.base }, NULL }, }
    }, NULL };

    // Get the Multiton View instance
    const struct IView *view = puremvc_view_getInstance(storage, "ViewTestKey4");

    // Create and register the test mediator
    struct ViewComponent viewComponent = {0};
    view->registerMediator(view, puremvc_mediator_init, "hasMediatorTest", &viewComponent);
    assert(strcmp(storage[0]->view.mediatorMap[0]->key, "hasMediatorTest") == 0);
    assert(strcmp(storage[0]->view.mediatorMap[0]->mediator->getName(storage[0]->view.mediatorMap[0]->mediator), "hasMediatorTest") == 0);

    // assert that the view.hasMediator method returns true
    // for that mediator name
    assert(view->hasMediator(view, "hasMediatorTest") == true);

    struct Mediator mediator = view->removeMediator(view, "hasMediatorTest");
    assert(storage[0]->view.mediatorMap[0]->key[0] == '\0');
    assert(strcmp(mediator.name, "hasMediatorTest") == 0);

    // assert that the view.hasMediator method returns false
    // for that mediator name
    assert(view->hasMediator(view, "hasMediatorTest") == false);

    puremvc_view_removeView(storage, "ViewTestKey4");
    view = NULL;
}

void testRegisterAndRemoveMediator() {
    struct ViewMap *storage[] = { &(struct ViewMap) {
        .view = { .mediatorMap = (struct MediatorMap *[]) { &(struct MediatorMap){ .mediator = &(struct Mediator){0}.base }, NULL }, }
    }, NULL };

    // Get the Multiton View instance
    const struct IView *view = puremvc_view_getInstance(storage, "ViewTestKey5");

    struct ViewComponent viewComponent = {0};

    // Register the mediator
    view->registerMediator(view, puremvc_mediator_init, "testing", &viewComponent);
    assert(strcmp(storage[0]->view.mediatorMap[0]->key, "testing") == 0);
    assert(strcmp(storage[0]->view.mediatorMap[0]->mediator->getName(storage[0]->view.mediatorMap[0]->mediator), "testing") == 0);

    // Remove the mediator
    const struct Mediator removedMediator = view->removeMediator(view, "testing");
    assert(storage[0]->view.mediatorMap[0]->key[0] == '\0');

    // assert that we have removed the appropriate mediator
    assert(strcmp(removedMediator.name, "testing") == 0);

    // assert that the mediator is no longer retrievable
    assert(view->retrieveMediator(view, "testing") == NULL);

    puremvc_view_removeView(storage, "ViewTestKey5");
    view = NULL;
}

void testOnRegisterAndOnRemove() {
    struct ViewMap *storage[] = { &(struct ViewMap) {
        .view = {
            .mediatorMap = (struct MediatorMap *[]) { &(struct MediatorMap){ .mediator = &(struct Mediator){0}.base }, NULL },
            .observerMap = (struct ObserverMap *[]) { &(struct ObserverMap) { .observers = (struct IObserver*[]){ &(struct Observer){0}.base, NULL } }, NULL },
        },
    }, NULL };

    // Get the Multiton View instance
    const struct IView *view = puremvc_view_getInstance(storage, "ViewTestKey6");
    struct ViewTest viewTest = {"", false, false, 0};

    view->registerMediator(view, view_test_mediator4, view_test_mediator4_NAME, &viewTest);
    assert(strcmp(storage[0]->view.mediatorMap[0]->key, view_test_mediator4_NAME) == 0);
    assert(storage[0]->view.mediatorMap[0]->mediator != NULL);
    assert(strcmp(storage[0]->view.mediatorMap[0]->mediator->getName(storage[0]->view.mediatorMap[0]->mediator), view_test_mediator4_NAME) == 0); // crash
    assert(strcmp(storage[0]->view.observerMap[0]->key, NOTE4) == 0);
    assert(storage[0]->view.observerMap[0]->observers[0]->getContext(storage[0]->view.observerMap[0]->observers[0]) == storage[0]->view.mediatorMap[0]->mediator);

    // assert that onRegister was called, and the mediator responded by setting our boolean
    assert(viewTest.onRegisterCalled);

    // Remove the component
    struct Mediator removedMediator = view->removeMediator(view, view_test_mediator4_NAME);
    assert(storage[0]->view.mediatorMap[0]->key[0] == '\0'); // mediatorMap
    assert(storage[0]->view.mediatorMap[0]->mediator != NULL);
    assert(strcmp(removedMediator.name, view_test_mediator4_NAME) == 0);
    assert(storage[0]->view.observerMap[0]->key[0] == '\0'); // observerMap
    assert(storage[0]->view.observerMap[0]->observers[0]->getContext(storage[0]->view.observerMap[0]->observers[0]) == NULL);

    // assert that the mediator is no longer retrievable
    assert(view->retrieveMediator(view, view_test_mediator4_NAME) == NULL);

    // assert that onRemove was called, and the mediator responded by setting our boolean
    assert(viewTest.onRemoveCalled);

    // clean up
    puremvc_view_removeView(storage, "ViewTestKey6");
    view = NULL;
}

void testSuccessiveRegisterAndRemoveMediator() {
    struct ViewMap *storage[] = { &(struct ViewMap){
        .view = {
            .mediatorMap = (struct MediatorMap*[]){ &(struct MediatorMap){ .mediator = &(struct Mediator){0}.base }, NULL },
            .observerMap = (struct ObserverMap*[]){
                &(struct ObserverMap){ .observers = (struct IObserver *[]){ &(struct Observer){0}.base, NULL } },
                &(struct ObserverMap){ .observers = (struct IObserver *[]){ &(struct Observer){0}.base, NULL } },
                &(struct ObserverMap){ .observers = (struct IObserver *[]){ &(struct Observer){0}.base, NULL } },
                NULL
            },
        },
    }, NULL };

    // Get the Multiton View instance
    const struct IView *view = puremvc_view_getInstance(storage, "ViewTestKey7");
    assert(strcmp(storage[0]->key, "ViewTestKey7") == 0);

    // Create and register the test mediator,
    // but not so we have a reference to it
    struct ViewComponent viewComponent = {0};
    view->registerMediator(view, view_test_mediator, view_test_mediator_NAME, &viewComponent);

    // test that we can retrieve it
    assert(view->retrieveMediator(view, view_test_mediator_NAME) != NULL);

    const struct IMediator *mediator = view->retrieveMediator(view, view_test_mediator_NAME);

    // assertions
    assert(strcmp(storage[0]->view.mediatorMap[0]->key, view_test_mediator_NAME) == 0);
    assert(storage[0]->view.mediatorMap[0]->mediator == mediator);
    const char **interest = (const char *[]){ "ABC", "DEF", "GHI", NULL }; // notifications of test mediator
    for (int i = 0; i < 2; i++) { // assert observer's context == mediator
        assert(strcmp(storage[0]->view.observerMap[i]->key, interest[i]) == 0);
        assert(storage[0]->view.observerMap[i]->observers[0]->getContext(storage[0]->view.observerMap[i]->observers[0]) == storage[0]->view.mediatorMap[0]->mediator);
    }

    // Remove the Mediator
    view->removeMediator(view, view_test_mediator_NAME);
    assert(storage[0]->view.mediatorMap[0]->key[0] == '\0');
    for (size_t i = 0; i < 2; i++) {
        assert(storage[0]->view.observerMap[i]->key[0] == '\0');
        assert(storage[0]->view.observerMap[i]->observers[0]->getContext(storage[0]->view.observerMap[i]->observers[0]) == NULL);
    }

    // test that retrieving it now returns null
    const struct Mediator removedMediator = view->removeMediator(view, view_test_mediator_NAME);
    assert(removedMediator.name[0] == '\0');
    assert(strcmp(view->removeMediator(view, view_test_mediator_NAME).name, "") == 0);

    // test that removing the mediator again once its gone doesn't cause crash
    view->removeMediator(view, view_test_mediator_NAME);

    // Create and register another instance of the test mediator,
    view->registerMediator(view, view_test_mediator, view_test_mediator_NAME, &viewComponent);
    assert(strcmp(storage[0]->view.mediatorMap[0]->key, view_test_mediator_NAME) == 0);
    assert(strcmp(storage[0]->view.mediatorMap[0]->mediator->getName(storage[0]->view.mediatorMap[0]->mediator), view_test_mediator_NAME) == 0);

    for (int i = 0; interest[i] != NULL; i++) {
        assert(strcmp(storage[0]->view.observerMap[i]->key, interest[i]) == 0);
        assert(storage[0]->view.observerMap[i]->observers[0]->getContext(storage[0]->view.observerMap[i]->observers[0]) == storage[0]->view.mediatorMap[0]->mediator);
    }

    assert(view->retrieveMediator(view, view_test_mediator_NAME) != NULL);

    // Remove the Mediator
    view->removeMediator(view, view_test_mediator_NAME);
    assert(storage[0]->view.mediatorMap[0]->key[0] == '\0');
    for (size_t i = 0; i < 2; i++) {
        assert(storage[0]->view.observerMap[i]->key[0] == '\0');
        assert(storage[0]->view.observerMap[i]->observers[0]->getContext(storage[0]->view.observerMap[i]->observers[0]) == NULL);
    }

    // test that retrieving it now returns null
    assert(view->retrieveMediator(view, view_test_mediator_NAME) == NULL);

    puremvc_view_removeView(storage, "ViewTestKey7");
    assert(storage[0]->key[0] == '\0');
    view = NULL;
}

void testRemoveMediatorAndSubsequentNotify() {
    struct ViewMap *storage[] = { &(struct ViewMap) {
        .view = {
            .mediatorMap = (struct MediatorMap *[]) { &(struct MediatorMap){ .mediator = &(struct Mediator){0}.base }, NULL },
            .observerMap = (struct ObserverMap *[]) { // for 2 notifications of view_test_mediator2
                &(struct ObserverMap) { .observers = (struct IObserver*[]){ &(struct Observer){0}.base, NULL } },
                &(struct ObserverMap) { .observers = (struct IObserver*[]){ &(struct Observer){0}.base, NULL } },
                NULL
            },
        },
    }, NULL };

    // Get the Multiton View instance
    const struct IView *view = puremvc_view_getInstance(storage, "ViewTestKey8");

    // Create and register the test mediator to be removed.
    struct ViewTest viewTest = {.lastNotification = NULL };
    // struct IMediator *mediator = view_test_mediator2(&(struct Mediator){0}, &viewTest);
    view->registerMediator(view, view_test_mediator2, view_test_mediator2_NAME, &viewTest);

    // test that we can retrieve it
    assert(view->retrieveMediator(view, view_test_mediator2_NAME) != NULL);

    const struct IMediator *mediator = view->retrieveMediator(view, view_test_mediator2_NAME);

    // assertions
    assert(strcmp(storage[0]->view.mediatorMap[0]->key, view_test_mediator2_NAME) == 0);
    assert(strcmp(storage[0]->view.mediatorMap[0]->mediator->getName(storage[0]->view.mediatorMap[0]->mediator), view_test_mediator2_NAME) == 0);
    const char **interest = mediator->listNotificationInterests(mediator);
    for (int i = 0; interest[i] != NULL; i++) {
        assert(strcmp(storage[0]->view.observerMap[i]->key, interest[i]) == 0);
        assert(storage[0]->view.observerMap[i]->observers[0]->getContext(storage[0]->view.observerMap[i]->observers[0]) == storage[0]->view.mediatorMap[0]->mediator);
    }

    // test that notifications work
    struct INotification *notification1 = puremvc_notification_init(&(struct Notification){0}, NOTE1, NULL, NULL);
    view->notifyObservers(view, notification1);
    assert(strcmp(viewTest.lastNotification, NOTE1) == 0);

    struct INotification *notification2 = puremvc_notification_init(&(struct Notification){0}, NOTE2, NULL, NULL);
    view->notifyObservers(view, notification2);
    assert(strcmp(viewTest.lastNotification, NOTE2) == 0);

    view->removeMediator(view, view_test_mediator2_NAME);
    assert(storage[0]->view.mediatorMap[0]->key[0] == '\0');
    for (size_t i = 0; i < 2; i++) { // 2 notifications
        assert(storage[0]->view.observerMap[i]->key[0] == '\0');
        assert(storage[0]->view.observerMap[i]->observers[0]->getContext(storage[0]->view.observerMap[i]->observers[0]) == NULL);
    }

    assert(view->retrieveMediator(view, view_test_mediator2_NAME) == NULL);

    viewTest.lastNotification = "";

    view->notifyObservers(view, notification1);
    assert(strcmp(viewTest.lastNotification, NOTE1) != 0);

    view->notifyObservers(view, notification2);
    assert(strcmp(viewTest.lastNotification, NOTE2) != 0);

    puremvc_view_removeView(storage, "ViewTestKey8");
    assert(storage[0]->key[0] == '\0');
    view = NULL;
}

void testRemoveOneOfTwoMediatorsAndSubsequentNotify() {
    struct ViewMap *storage[] = { &(struct ViewMap) {
        .view = {
            .mediatorMap = (struct MediatorMap *[]) {
                &(struct MediatorMap){ .mediator = &(struct Mediator){0}.base }, &(struct MediatorMap){ .mediator = &(struct Mediator){0}.base },
                NULL
            },
            .observerMap = (struct ObserverMap *[]) {
                &(struct ObserverMap) { .observers = (struct IObserver*[]){ &(struct Observer){0}.base, NULL } }, // NOTE1
                &(struct ObserverMap) { .observers = (struct IObserver*[]){ &(struct Observer){0}.base, NULL } }, // NOTE2
                &(struct ObserverMap) { .observers = (struct IObserver*[]){ &(struct Observer){0}.base, NULL } }, // NOTE3
                NULL
            },
        },
    }, NULL };

    // Get the Multiton View instance
    const struct IView *view = puremvc_view_getInstance(storage, "ViewTestKey9");
    struct ViewTest viewTest = {};

    // Create and register that responds to notifications 1 and 2
    view->registerMediator(view, view_test_mediator2, view_test_mediator2_NAME, &viewTest);

    // assertions
    assert(strcmp(storage[0]->view.mediatorMap[0]->key, view_test_mediator2_NAME) == 0);
    assert(strcmp(storage[0]->view.mediatorMap[0]->mediator->getName(storage[0]->view.mediatorMap[0]->mediator), view_test_mediator2_NAME) == 0);
    assert(strcmp(storage[0]->view.observerMap[0]->key, NOTE1) == 0);
    assert(storage[0]->view.observerMap[0]->observers[0]->getContext(storage[0]->view.observerMap[0]->observers[0]) == storage[0]->view.mediatorMap[0]->mediator);
    assert(strcmp(storage[0]->view.observerMap[1]->key, NOTE2) == 0);
    assert(storage[0]->view.observerMap[1]->observers[0]->getContext(storage[0]->view.observerMap[1]->observers[0]) == storage[0]->view.mediatorMap[0]->mediator);

    // Create and register that responds to notification 3
    view->registerMediator(view, view_test_mediator3, view_test_mediator3_NAME, &viewTest);

    // assertions
    assert(strcmp(storage[0]->view.mediatorMap[1]->key, view_test_mediator3_NAME) == 0);
    assert(strcmp(storage[0]->view.mediatorMap[1]->mediator->getName(storage[0]->view.mediatorMap[1]->mediator), view_test_mediator3_NAME) == 0);
    assert(strcmp(storage[0]->view.observerMap[2]->key, NOTE3) == 0);
    assert(storage[0]->view.observerMap[2]->observers[0]->getContext(storage[0]->view.observerMap[2]->observers[0]) == storage[0]->view.mediatorMap[1]->mediator);

    // test that all notifications work
    struct INotification *notification1 = puremvc_notification_init(&(struct Notification){0}, NOTE1, NULL, NULL);
    view->notifyObservers(view, notification1);
    assert(strcmp(viewTest.lastNotification, NOTE1) == 0);

    struct INotification *notification2 = puremvc_notification_init(&(struct Notification){0}, NOTE2, NULL, NULL);
    view->notifyObservers(view, notification2);
    assert(strcmp(viewTest.lastNotification, NOTE2) == 0);

    struct INotification *notification3 = puremvc_notification_init(&(struct Notification){0}, NOTE3, NULL, NULL);
    view->notifyObservers(view, notification3);
    assert(strcmp(viewTest.lastNotification, NOTE3) == 0);

    // Remove the Mediator that responds to 1 and 2
    view->removeMediator(view, view_test_mediator2_NAME);
    assert(strcmp(storage[0]->view.mediatorMap[0]->key, view_test_mediator3_NAME) == 0); // mediator3 shifted left
    assert(strcmp(storage[0]->view.mediatorMap[0]->mediator->getName(storage[0]->view.mediatorMap[0]->mediator), view_test_mediator3_NAME) == 0);
    assert(strcmp(storage[0]->view.observerMap[0]->key, NOTE3) == 0);
    assert(storage[0]->view.observerMap[0]->observers[0]->getContext(storage[0]->view.observerMap[0]->observers[0]) == storage[0]->view.mediatorMap[0]->mediator);
    assert(storage[0]->view.observerMap[1]->key[0] == '\0');
    assert(storage[0]->view.observerMap[1]->observers[0]->getContext(storage[0]->view.observerMap[1]->observers[0]) == NULL);
    assert(storage[0]->view.observerMap[2]->key[0] == '\0');
    assert(storage[0]->view.observerMap[2]->observers[0]->getContext(storage[0]->view.observerMap[2]->observers[0]) == NULL);

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

    view->removeMediator(view, view_test_mediator3_NAME);
    assert(storage[0]->view.mediatorMap[0]->key[0] == '\0');
    for (size_t i = 0; i < 3; i++) {
        assert(storage[0]->view.observerMap[i]->key[0] == '\0');
        assert(storage[0]->view.observerMap[i]->observers[0]->getContext(storage[0]->view.observerMap[i]->observers[0]) == NULL);
    }

    puremvc_view_removeView(storage, "ViewTestKey9");
    view = NULL;
}

void testMediatorReregistration() {
    struct ViewMap *storage[] = { &(struct ViewMap) {
        .view = {
            .mediatorMap = (struct MediatorMap *[]) { &(struct MediatorMap){ .mediator = &(struct Mediator){0}.base }, NULL },
            .observerMap = (struct ObserverMap *[]) {
                &(struct ObserverMap) { .observers = (struct IObserver*[]){ &(struct Observer){0}.base, NULL } }, // NOTE5
                NULL
            },
        },
    }, NULL };

    // Get the Multiton View instance
    const struct IView *view = puremvc_view_getInstance(storage, "ViewTestKey10");

    // Create and register that responds to notification 5
    struct ViewTest viewTest = {};

    // try to register another instance of that mediator (uses the same NAME constant).
    view->registerMediator(view, view_test_mediator5, view_test_mediator5_NAME, &viewTest);

    // assertions
    assert(strcmp(storage[0]->view.mediatorMap[0]->key, view_test_mediator5_NAME) == 0);
    assert(strcmp(storage[0]->view.mediatorMap[0]->mediator->getName(storage[0]->view.mediatorMap[0]->mediator), view_test_mediator5_NAME) == 0);
    assert(strcmp(storage[0]->view.observerMap[0]->key, NOTE5) == 0);
    assert(storage[0]->view.observerMap[0]->observers[0]->getContext(storage[0]->view.observerMap[0]->observers[0]) == storage[0]->view.mediatorMap[0]->mediator);

    // test that the counter is only incremented once (mediator 5's response)
    viewTest.counter = 0;
    struct INotification *notification = puremvc_notification_init(&(struct Notification){0}, NOTE5, NULL, NULL);
    view->notifyObservers(view, notification);
    assert(viewTest.counter == 1);

    // Remove the Mediator
    view->removeMediator(view, view_test_mediator5_NAME);
    assert(storage[0]->view.mediatorMap[0]->key[0] == '\0');
    assert(storage[0]->view.mediatorMap[0]->mediator != NULL);
    assert(storage[0]->view.observerMap[0]->key[0] == '\0');
    assert(storage[0]->view.observerMap[0]->observers[0]->getContext(storage[0]->view.observerMap[0]->observers[0]) == NULL);

    // test that retrieving it now returns null
    assert(view->retrieveMediator(view, view_test_mediator5_NAME) == NULL);

    // test that the counter is no longer incremented
    viewTest.counter = 0;
    view->notifyObservers(view, notification);
    assert(viewTest.counter == 0);

    puremvc_view_removeView(storage, "ViewTestKey10");
    view = NULL;
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
    struct ViewMap *storage[] = { &(struct ViewMap) {
        .view = {
            .mediatorMap = (struct MediatorMap *[]) { // null terminated 8 slots for MediatorMap
                &(struct MediatorMap){ .mediator = &(struct Mediator){0}.base }, &(struct MediatorMap){ .mediator = &(struct Mediator){0}.base },
                &(struct MediatorMap){ .mediator = &(struct Mediator){0}.base }, &(struct MediatorMap){ .mediator = &(struct Mediator){0}.base },
                &(struct MediatorMap){ .mediator = &(struct Mediator){0}.base }, &(struct MediatorMap){ .mediator = &(struct Mediator){0}.base },
                &(struct MediatorMap){ .mediator = &(struct Mediator){0}.base }, &(struct MediatorMap){ .mediator = &(struct Mediator){0}.base },
                NULL
            },
            .observerMap = (struct ObserverMap *[]) {
                &(struct ObserverMap) { // NOTE5
                    .key = "",
                    .observers = (struct IObserver *[]) { // observer per notification
                        &(struct Observer){0}.base, &(struct Observer){0}.base, &(struct Observer){0}.base, &(struct Observer){0}.base,
                        &(struct Observer){0}.base, &(struct Observer){0}.base, &(struct Observer){0}.base, &(struct Observer){0}.base, NULL
                    }
                },
                NULL
            },
        },
    }, NULL };

    // Get the Multiton View instance
    const struct IView *view = puremvc_view_getInstance(storage, "ViewTestKey11");

    static char buffer[9][KEY_SIZE]; // Create the actual bytes (9 rows, each KEY_SIZE wide)
    static char *buffers[] = {
        buffer[0], buffer[1], buffer[2],
        buffer[3], buffer[4], buffer[5],
        buffer[6], buffer[7], buffer[8], NULL // NULL terminator for loops
    };

    struct ViewTest viewTest = {"", "", "", 0, .deferred = buffers };

    // Create and register several mediator instances that respond to notification 6
    // by removing themselves, which will cause the observer list for that notification
    // to change.
    view->registerMediator(view, view_test_mediator6, "view_test_mediator6/1", &viewTest);
    view->registerMediator(view, view_test_mediator6, "view_test_mediator6/2", &viewTest);
    view->registerMediator(view, view_test_mediator6, "view_test_mediator6/3", &viewTest);
    view->registerMediator(view, view_test_mediator6, "view_test_mediator6/4", &viewTest);
    view->registerMediator(view, view_test_mediator6, "view_test_mediator6/5", &viewTest);
    view->registerMediator(view, view_test_mediator6, "view_test_mediator6/6", &viewTest);
    view->registerMediator(view, view_test_mediator6, "view_test_mediator6/7", &viewTest);
    view->registerMediator(view, view_test_mediator6, "view_test_mediator6/8", &viewTest);

    // assertions
    for (size_t i = 0; i < 8; i++) {
        char expected[KEY_SIZE];
        snprintf(expected, KEY_SIZE, "view_test_mediator6/%zu", i + 1);
        assert(strcmp(storage[0]->view.mediatorMap[i]->key, expected) == 0);
        assert(storage[0]->view.observerMap[0]->observers[i]->getContext(storage[0]->view.observerMap[0]->observers[i]) == storage[0]->view.mediatorMap[i]->mediator);
    }

    // send the notification. each of the above mediators will respond by removing
    // themselves and incrementing the counter by 1. This should leave us with a
    // count of 8, since 8 mediators will respond.
    struct INotification *notification = puremvc_notification_init(&(struct Notification){}, NOTE6, NULL, NULL);
    view->notifyObservers(view, notification);

    // assertions
    for (size_t i = 0; i < 8; i++) {
        char expected[KEY_SIZE];
        snprintf(expected, KEY_SIZE, "view_test_mediator6/%zu", i + 1);
        assert(strcmp(viewTest.deferred[i], expected) == 0);
    }

    // iterate through deferred names and call removeMediator
    for (size_t i = 0; viewTest.deferred[i] != NULL; i++) {
        const char *mediatorName = viewTest.deferred[i];
        assert(mediatorName != NULL);
        view->removeMediator(view, mediatorName);
    }

    // verify the count is correct
    assert(viewTest.counter == 8);

    // clear the counter
    viewTest.counter = 0;
    view->notifyObservers(view, notification);

    // verify the count is 0
    assert(viewTest.counter == 0);

    puremvc_view_removeView(storage, "ViewTestKey11");
    view = NULL;
}

void testRemoveView() {
    struct ViewMap *storage[] = { &(struct ViewMap) {}, NULL };

    // Get a Multiton View instance
    puremvc_view_getInstance(storage, "ViewTestKey12");

    assert(strcmp(storage[0]->key, "ViewTestKey12") == 0);
    assert(strcmp(storage[0]->view.multitonKey, "ViewTestKey12") == 0);

    // remove the View
    puremvc_view_removeView(storage, "ViewTestKey12");
    assert(storage[0]->key[0] == '\0');
    assert(storage[0]->view.multitonKey[0] == '\0');

    // re-create the view without throwing an exception
    puremvc_view_getInstance(storage, "ViewTestKey12");
    assert(strcmp(storage[0]->key, "ViewTestKey12") == 0);
    assert(strcmp(storage[0]->view.multitonKey, "ViewTestKey12") == 0);

    // cleanup
    puremvc_view_removeView(storage, "ViewTestKey12");
    assert(storage[0]->key[0] == '\0');
    assert(storage[0]->view.multitonKey[0] == '\0');
}

void testGarbageStorageForView() {
    struct ViewMap *storage1[] = { NULL }; // empty view
    const struct IView *view1 = puremvc_view_getInstance(storage1, "ViewTestKey13"); // crash test
    assert(view1 == NULL);
    puremvc_view_removeView(storage1, "ViewTestKey13");
}

void testGarbageStorageForObserver() {
    struct ViewComponent component = {0};

    struct ViewMap *storage1[] = { &(struct ViewMap) { // missing observerMap field
        .view = {}
    }, NULL };
    const struct IView *view1 = puremvc_view_getInstance(storage1, "ViewTestKey14");
    view1->notifyObservers(view1, NULL); // crash test
    view1->removeObserver(view1, "ViewTestKey14_testing1", &component);
    view1->registerObserver(view1, "ViewTestKey14_testing1", (void (*)(const void *, struct INotification *)) handleNotification, &component);
    assert(storage1[0]->view.observerMap == NULL);
    puremvc_view_removeView(storage1, "ViewTestKey14");

    struct ViewMap *storage2[] = { &(struct ViewMap) { // empty observerMap
        .view = {
            .observerMap = (struct ObserverMap *[]) { NULL },
        }
    }, NULL };
    const struct IView *view2 = puremvc_view_getInstance(storage2, "ViewTestKey14");
    view2->notifyObservers(view2, NULL);
    view2->removeObserver(view2, "ViewTestKey14_testing2", &component);
    view2->registerObserver(view2, "ViewTestKey14_testing2", (void (*)(const void *, struct INotification *)) handleNotification, &component);
    assert(storage2[0]->view.observerMap[0] == NULL);
    puremvc_view_removeView(storage2, "ViewTestKey14");

    struct ViewMap *storage3[] = { &(struct ViewMap) { // missing observers field
        .view = {
            .observerMap = (struct ObserverMap *[]) { &(struct ObserverMap) {}, NULL },
        }
    }, NULL };
    const struct IView *view3 = puremvc_view_getInstance(storage3, "ViewTestKey14");
    view3->notifyObservers(view3, NULL);
    view3->removeObserver(view3, "ViewTestKey14_testing3", &component);
    view3->registerObserver(view3, "ViewTestKey14_testing3", (void (*)(const void *, struct INotification *)) handleNotification, &component);
    assert(storage3[0]->view.observerMap != NULL);
    assert(storage3[0]->view.observerMap[0]->key[0] == '\0');
    puremvc_view_removeView(storage3, "ViewTestKey14");

    struct ViewMap *storage4[] = { &(struct ViewMap) { // empty observers
        .view = {
            .observerMap = (struct ObserverMap *[]) { &(struct ObserverMap) {
                .observers = (struct IObserver*[]) { NULL }
            }, NULL },
        }
    }, NULL };
    const struct IView *view4 = puremvc_view_getInstance(storage4, "ViewTestKey14");
    view4->notifyObservers(view4, NULL);
    view4->removeObserver(view4, "ViewTestKey14_testing4", &component);
    view4->registerObserver(view4, "ViewTestKey14_testing4", (void (*)(const void *, struct INotification *)) handleNotification, &component);
    assert(storage4[0]->view.observerMap != NULL);
    assert(storage4[0]->view.observerMap[0]->key[0] == '\0');
    assert(storage4[0]->view.observerMap[0]->observers != NULL);
    assert(storage4[0]->view.observerMap[0]->observers[0] == NULL);
    puremvc_view_removeView(storage4, "ViewTestKey14");
}

void testGarbageStorageForMediator() {
    struct ViewMap *storage1[] = { &(struct ViewMap) { // missing mediatorMap
        .view = { },
    }, NULL };

    const struct IView *view1 = puremvc_view_getInstance(storage1, "ViewTestKey15");
    view1->registerMediator(view1, puremvc_mediator_init, MEDIATOR_NAME, NULL);
    view1->hasMediator(view1, MEDIATOR_NAME);
    view1->retrieveMediator(view1, MEDIATOR_NAME);
    view1->removeMediator(view1, MEDIATOR_NAME);
    assert(storage1[0]->view.mediatorMap == NULL);
    puremvc_view_removeView(storage1, "ViewTestKey15");

    struct ViewMap *storage2[] = { &(struct ViewMap) { // empty mediatorMap
        .view = { .mediatorMap = (struct MediatorMap*[]){ NULL} },
    }, NULL };
    const struct IView *view2 = puremvc_view_getInstance(storage2, "ViewTestKey15");
    view2->registerMediator(view2, puremvc_mediator_init, MEDIATOR_NAME, NULL);
    view2->hasMediator(view2, MEDIATOR_NAME);
    view2->retrieveMediator(view2, MEDIATOR_NAME);
    view2->removeMediator(view2, MEDIATOR_NAME);
    assert(storage2[0]->view.mediatorMap[0] == NULL);
    puremvc_view_removeView(storage2, "ViewTestKey15");
}

void testObserverMapShiftLeft() {
    struct ViewMap *storage[] = { &(struct ViewMap) {
        .view = {
            .observerMap = (struct ObserverMap*[]) { // 4 ObserverMaps each with a single Observer
                &(struct ObserverMap) { .observers = (struct IObserver*[]){ &(struct Observer){0}.base, NULL } },
                &(struct ObserverMap) { .observers = (struct IObserver*[]){ &(struct Observer){0}.base, NULL } },
                &(struct ObserverMap) { .observers = (struct IObserver*[]){ &(struct Observer){0}.base, NULL } },
                &(struct ObserverMap) { .observers = (struct IObserver*[]){ &(struct Observer){0}.base, NULL } },
                NULL
            },
        }
    }, NULL };

    const struct IView *view = puremvc_view_getInstance(storage, "ViewTestKey16");
    const struct View *self = (struct View *) view;

    // register four observers, check association and remove them
    view->registerObserver(view, "observer0", (void (*)(const void *, struct INotification *)) handleNotification, (void *) view);
    assert(strcmp(self->observerMap[0]->key, "observer0") == 0);
    assert(self->observerMap[0]->observers[0]->getContext(self->observerMap[0]->observers[0]) == self);

    view->registerObserver(view, "observer1", (void (*)(const void *, struct INotification *)) handleNotification, (void *) view);
    assert(strcmp(self->observerMap[1]->key, "observer1") == 0);
    assert(self->observerMap[1]->observers[0]->getContext(self->observerMap[1]->observers[0]) == self);

    view->registerObserver(view, "observer2", (void (*)(const void *, struct INotification *)) handleNotification, (void *) view);
    assert(strcmp(self->observerMap[2]->key, "observer2") == 0);
    assert(self->observerMap[2]->observers[0]->getContext(self->observerMap[2]->observers[0]) == self);

    view->registerObserver(view, "observer3", (void (*)(const void *, struct INotification *)) handleNotification, (void *) view);
    assert(strcmp(self->observerMap[3]->key, "observer3") == 0);
    assert(self->observerMap[3]->observers[0]->getContext(self->observerMap[3]->observers[0]) == self);

    // remove the middle observer2; remaining entries (2, 3) shift left and the tail is reinit.
    view->removeObserver(view, "observer1", self);
    assert(strcmp(self->observerMap[0]->key, "observer0") == 0);
    assert(self->observerMap[0]->observers[0]->getContext(self->observerMap[0]->observers[0]) == self);
    assert(strcmp(self->observerMap[1]->key, "observer2") == 0);
    assert(self->observerMap[1]->observers[0]->getContext(self->observerMap[0]->observers[0]) == self);
    assert(strcmp(self->observerMap[2]->key, "observer3") == 0);
    assert(self->observerMap[2]->observers[0]->getContext(self->observerMap[0]->observers[0]) == self);
    assert(self->observerMap[3]->key[0] == '\0');
    assert(self->observerMap[3]->observers[0]->getContext(self->observerMap[3]->observers[0]) == NULL);
    assert(self->observerMap[4] == NULL);

    // remove the last observer3 and verify the remaining entries (0, 2) stay in place
    view->removeObserver(view, "observer3", self);
    assert(strcmp(self->observerMap[0]->key, "observer0") == 0);
    assert(self->observerMap[0]->observers[0]->getContext(self->observerMap[0]->observers[0]) == self);
    assert(strcmp(self->observerMap[1]->key, "observer2") == 0);
    assert(self->observerMap[1]->observers[0]->getContext(self->observerMap[0]->observers[0]) == self);
    assert(self->observerMap[2]->key[0] == '\0');
    assert(self->observerMap[2]->observers[0]->getContext(self->observerMap[3]->observers[0]) == NULL);
    assert(self->observerMap[2]->key[0] == '\0');
    assert(self->observerMap[3]->observers[0]->getContext(self->observerMap[3]->observers[0]) == NULL);
    assert(self->observerMap[4] == NULL);

    // remove the first observer0 and verify that remaining observer shift left
    view->removeObserver(view, "observer0", self);
    assert(strcmp(self->observerMap[0]->key, "observer2") == 0);
    assert(self->observerMap[0]->observers[0]->getContext(self->observerMap[0]->observers[0]) == self);
    assert(self->observerMap[1]->key[0] == '\0');
    assert(self->observerMap[1]->observers[0]->getContext(self->observerMap[3]->observers[0]) == NULL);
    assert(self->observerMap[2]->key[0] == '\0');
    assert(self->observerMap[2]->observers[0]->getContext(self->observerMap[3]->observers[0]) == NULL);
    assert(self->observerMap[3]->key[0] == '\0');
    assert(self->observerMap[3]->observers[0]->getContext(self->observerMap[3]->observers[0]) == NULL);
    assert(self->observerMap[4] == NULL);

    // Remove the remaining observer2
    view->removeObserver(view, "observer2", self);
    assert(self->observerMap[0]->key[0] == '\0');
    assert(self->observerMap[0]->observers[0]->getContext(self->observerMap[0]->observers[0]) == NULL);
    assert(self->observerMap[1]->key[0] == '\0');
    assert(self->observerMap[1]->observers[0]->getContext(self->observerMap[3]->observers[0]) == NULL);
    assert(self->observerMap[2]->key[0] == '\0');
    assert(self->observerMap[2]->observers[0]->getContext(self->observerMap[3]->observers[0]) == NULL);
    assert(self->observerMap[3]->key[0] == '\0');
    assert(self->observerMap[3]->observers[0]->getContext(self->observerMap[3]->observers[0]) == NULL);
    assert(self->observerMap[4] == NULL);

    puremvc_view_removeView(storage, "ViewTestKey16");
    self = NULL;
}

// another test for observers shift left in a single ObserverMap
void testObserverShiftLeft() {
    struct ViewMap *storage[] = { &(struct ViewMap) {
        .view = {
            .observerMap = (struct ObserverMap*[]) { // An ObserverMaps with 4 Observers
                &(struct ObserverMap) {
                    .observers = (struct IObserver*[]){
                        &(struct Observer){0}.base, &(struct Observer){0}.base,
                        &(struct Observer){0}.base, &(struct Observer){0}.base,
                        NULL
                    }
                },
                NULL
            },
        }
    }, NULL };

    const struct IView *view = puremvc_view_getInstance(storage, "ViewTestKey17");

    struct ViewComponent context0 = {0}, context1 = {0}, context2 = {0}, context3 = {0};

    // register four observers, check association and remove them
    view->registerObserver(view, "notification0", (void (*)(const void *, struct INotification *)) handleNotification, &context0);

    assert(strcmp(storage[0]->view.observerMap[0]->key, "notification0") == 0); // key gets created

    assert(storage[0]->view.observerMap[0]->observers[0]->getContext(storage[0]->view.observerMap[0]->observers[0]) == &context0);
    view->registerObserver(view, "notification0", (void (*)(const void *, struct INotification *)) handleNotification, &context1);
    assert(storage[0]->view.observerMap[0]->observers[1]->getContext(storage[0]->view.observerMap[0]->observers[1]) == &context1);
    view->registerObserver(view, "notification0", (void (*)(const void *, struct INotification *)) handleNotification, &context2);
    assert(storage[0]->view.observerMap[0]->observers[2]->getContext(storage[0]->view.observerMap[0]->observers[2]) == &context2);
    view->registerObserver(view, "notification0", (void (*)(const void *, struct INotification *)) handleNotification, &context3);
    assert(storage[0]->view.observerMap[0]->observers[3]->getContext(storage[0]->view.observerMap[0]->observers[3]) == &context3);
    assert(storage[0]->view.observerMap[0]->observers[4] == NULL);

    // remove the middle context1; remaining entries (2, 3) shift left and the tail is reinit.
    view->removeObserver(view, "notification0", &context1);
    assert(storage[0]->view.observerMap[0]->observers[0]->getContext(storage[0]->view.observerMap[0]->observers[0]) == &context0);
    assert(storage[0]->view.observerMap[0]->observers[1]->getContext(storage[0]->view.observerMap[0]->observers[1]) == &context2);
    assert(storage[0]->view.observerMap[0]->observers[2]->getContext(storage[0]->view.observerMap[0]->observers[2]) == &context3);
    assert(storage[0]->view.observerMap[0]->observers[3]->getContext(storage[0]->view.observerMap[0]->observers[3]) == NULL);
    assert(storage[0]->view.observerMap[0]->observers[4] == NULL);

    // remove the last context3 and verify the remaining entries (0, 2) stay in place
    view->removeObserver(view, "notification0", &context3);
    assert(storage[0]->view.observerMap[0]->observers[0]->getContext(storage[0]->view.observerMap[0]->observers[0]) == &context0);
    assert(storage[0]->view.observerMap[0]->observers[1]->getContext(storage[0]->view.observerMap[0]->observers[1]) == &context2);
    assert(storage[0]->view.observerMap[0]->observers[2]->getContext(storage[0]->view.observerMap[0]->observers[2]) == NULL);
    assert(storage[0]->view.observerMap[0]->observers[3]->getContext(storage[0]->view.observerMap[0]->observers[3]) == NULL);
    assert(storage[0]->view.observerMap[0]->observers[4] == NULL);

    // remove the first context0 and verify that remaining observer shift left
    view->removeObserver(view, "notification0", &context0);
    assert(storage[0]->view.observerMap[0]->observers[0]->getContext(storage[0]->view.observerMap[0]->observers[0]) == &context2);
    assert(storage[0]->view.observerMap[0]->observers[1]->getContext(storage[0]->view.observerMap[0]->observers[1]) == NULL);
    assert(storage[0]->view.observerMap[0]->observers[2]->getContext(storage[0]->view.observerMap[0]->observers[2]) == NULL);
    assert(storage[0]->view.observerMap[0]->observers[3]->getContext(storage[0]->view.observerMap[0]->observers[3]) == NULL);
    assert(storage[0]->view.observerMap[0]->observers[4] == NULL);

    // Remove the remaining context2
    view->removeObserver(view, "notification0", &context2);
    assert(storage[0]->view.observerMap[0]->observers[0]->getContext(storage[0]->view.observerMap[0]->observers[0]) == NULL);
    assert(storage[0]->view.observerMap[0]->observers[1]->getContext(storage[0]->view.observerMap[0]->observers[1]) == NULL);
    assert(storage[0]->view.observerMap[0]->observers[2]->getContext(storage[0]->view.observerMap[0]->observers[2]) == NULL);
    assert(storage[0]->view.observerMap[0]->observers[3]->getContext(storage[0]->view.observerMap[0]->observers[3]) == NULL);
    assert(storage[0]->view.observerMap[0]->observers[4] == NULL);

    assert(storage[0]->view.observerMap[0]->key[0] == '\0'); // key gets created

    // Test duplicate context registration and repeated removal:
    view->registerObserver(view, "notification0", (void (*)(const void *, struct INotification *)) handleNotification, &context0);
    view->registerObserver(view, "notification0", (void (*)(const void *, struct INotification *)) handleNotification, &context0);
    view->registerObserver(view, "notification0", (void (*)(const void *, struct INotification *)) handleNotification, &context0);
    view->registerObserver(view, "notification0", (void (*)(const void *, struct INotification *)) handleNotification, &context0);

    view->removeObserver(view, "notification0", &context0);
    view->removeObserver(view, "notification0", &context0);
    view->removeObserver(view, "notification0", &context0);
    view->removeObserver(view, "notification0", &context0);

    // Removing a context that was never registered
    view->removeObserver(view, "notification0", &(struct ViewComponent){0});
}

void testMediatorMapShiftLeft() {
    struct ViewMap *storage[] = { &(struct ViewMap) { // storage for 4 mediators sharing one notification, one observer per notification
        .view = {
            .mediatorMap = (struct MediatorMap *[]) {
                &(struct MediatorMap){ .mediator = &(struct Mediator){0}.base },
                &(struct MediatorMap){ .mediator = &(struct Mediator){0}.base },
                &(struct MediatorMap){ .mediator = &(struct Mediator){0}.base },
                &(struct MediatorMap){ .mediator = &(struct Mediator){0}.base },
                NULL
            },
            .observerMap = (struct ObserverMap *[]) { &(struct ObserverMap) {
                .observers = (struct IObserver*[]) {
                    &(struct Observer){0}.base,
                    &(struct Observer){0}.base,
                    &(struct Observer){0}.base,
                    &(struct Observer){0}.base,
                    NULL
                }
            }, NULL }
        },
    }, NULL };

    const struct IView *view = puremvc_view_getInstance(storage, "ViewTestKey17");

    // Register four mediators and verify that each is correctly associated to their observers
    view->registerMediator(view, view_test_mediator7, "mediator0", NULL);
    assert(strcmp(storage[0]->view.mediatorMap[0]->mediator->getName(storage[0]->view.mediatorMap[0]->mediator), "mediator0") == 0);
    assert(storage[0]->view.observerMap[0]->observers[0]->getContext(storage[0]->view.observerMap[0]->observers[0]) == storage[0]->view.mediatorMap[0]->mediator);

    view->registerMediator(view, view_test_mediator7, "mediator1", NULL);
    assert(strcmp(storage[0]->view.mediatorMap[1]->mediator->getName(storage[0]->view.mediatorMap[1]->mediator), "mediator1") == 0);
    assert(storage[0]->view.observerMap[0]->observers[1]->getContext(storage[0]->view.observerMap[0]->observers[1]) == storage[0]->view.mediatorMap[1]->mediator);

    view->registerMediator(view, view_test_mediator7, "mediator2", NULL);
    assert(strcmp(storage[0]->view.mediatorMap[2]->mediator->getName(storage[0]->view.mediatorMap[2]->mediator), "mediator2") == 0);
    assert(storage[0]->view.observerMap[0]->observers[2]->getContext(storage[0]->view.observerMap[0]->observers[2]) == storage[0]->view.mediatorMap[2]->mediator);

    view->registerMediator(view, view_test_mediator7, "mediator3", NULL);
    assert(strcmp(storage[0]->view.mediatorMap[3]->mediator->getName(storage[0]->view.mediatorMap[3]->mediator), "mediator3") == 0);
    assert(storage[0]->view.observerMap[0]->observers[3]->getContext(storage[0]->view.observerMap[0]->observers[3]) == storage[0]->view.mediatorMap[3]->mediator);

    // Verify the dictionary key for the map is correctly set
    assert(strcmp(storage[0]->view.observerMap[0]->key, NOTE7) == 0);

    // Remove the second mediator1 (middle) and verify that remaining mediators 2, 3 are shifted correctly
    view->removeMediator(view, "mediator1");
    assert(strcmp(storage[0]->view.mediatorMap[0]->key, "mediator0") == 0);
    assert(strcmp(storage[0]->view.mediatorMap[0]->mediator->getName(storage[0]->view.mediatorMap[0]->mediator), "mediator0") == 0);
    assert(storage[0]->view.observerMap[0]->observers[0]->getContext(storage[0]->view.observerMap[0]->observers[0]) == storage[0]->view.mediatorMap[0]->mediator);
    assert(strcmp(storage[0]->view.mediatorMap[1]->key, "mediator2") == 0);
    assert(strcmp(storage[0]->view.mediatorMap[1]->mediator->getName(storage[0]->view.mediatorMap[1]->mediator), "mediator2") == 0);
    assert(storage[0]->view.observerMap[0]->observers[1]->getContext(storage[0]->view.observerMap[0]->observers[1]) == storage[0]->view.mediatorMap[1]->mediator);
    assert(strcmp(storage[0]->view.mediatorMap[2]->key, "mediator3") == 0);
    assert(strcmp(storage[0]->view.mediatorMap[2]->mediator->getName(storage[0]->view.mediatorMap[2]->mediator), "mediator3") == 0);
    assert(storage[0]->view.observerMap[0]->observers[2]->getContext(storage[0]->view.observerMap[0]->observers[2]) == storage[0]->view.mediatorMap[2]->mediator);

    assert(storage[0]->view.mediatorMap[3]->key[0] == '\0'); // mediatorMap key
    assert(storage[0]->view.observerMap[0]->key[0] != '\0'); // observerMap key

    // Remove the last mediator3 and verify the remaining mediators 0, 2 stay in place
    view->removeMediator(view, "mediator3");
    assert(strcmp(storage[0]->view.mediatorMap[0]->mediator->getName(storage[0]->view.mediatorMap[0]->mediator), "mediator0") == 0);
    assert(storage[0]->view.observerMap[0]->observers[0]->getContext(storage[0]->view.observerMap[0]->observers[0]) == storage[0]->view.mediatorMap[0]->mediator);
    assert(strcmp(storage[0]->view.mediatorMap[1]->mediator->getName(storage[0]->view.mediatorMap[1]->mediator), "mediator2") == 0);
    assert(storage[0]->view.observerMap[0]->observers[1]->getContext(storage[0]->view.observerMap[0]->observers[1]) == storage[0]->view.mediatorMap[1]->mediator);

    assert(storage[0]->view.mediatorMap[2]->key[0] == '\0');
    assert(storage[0]->view.mediatorMap[3]->key[0] == '\0');
    assert(storage[0]->view.observerMap[0]->key[0] != '\0'); // ObserverMap key will persist while observers exist

    // Remove the first mediator0 and verify that subsequent mediator2 shift left
    view->removeMediator(view, "mediator0");
    assert(strcmp(storage[0]->view.mediatorMap[0]->mediator->getName(storage[0]->view.mediatorMap[0]->mediator), "mediator2") == 0);
    assert(storage[0]->view.observerMap[0]->observers[0]->getContext(storage[0]->view.observerMap[0]->observers[0]) == storage[0]->view.mediatorMap[0]->mediator);

    assert(storage[0]->view.mediatorMap[1]->key[0] == '\0');
    assert(storage[0]->view.mediatorMap[2]->key[0] == '\0');
    assert(storage[0]->view.mediatorMap[3]->key[0] == '\0');
    assert(storage[0]->view.observerMap[0]->key[0] != '\0'); // ObserverMap key will persist while observers exist

    // Remove the last mediator2 and confirm that the dictionary key is cleared
    view->removeMediator(view, "mediator2");
    assert(storage[0]->view.mediatorMap[1]->key[0] == '\0');
    assert(storage[0]->view.mediatorMap[2]->key[0] == '\0');
    assert(storage[0]->view.mediatorMap[3]->key[0] == '\0');
    assert(storage[0]->view.observerMap[0]->key[0] == '\0'); // ObserverMap key cleared since no observers left

    puremvc_view_removeView(storage, "ViewTestKey17");
    view = NULL;
}

void testViewShiftLeft() {
    struct ViewMap *storage[] = {
        &(struct ViewMap) { .view = {} }, &(struct ViewMap) { .view = {} },
        &(struct ViewMap) { .view = {} }, &(struct ViewMap) { .view = {} },
        NULL
    };

    // get 4 instances
    puremvc_view_getInstance(storage, "view0");
    assert(strcmp(storage[0]->key, "view0") == 0);
    assert(strcmp(storage[0]->view.multitonKey, "view0") == 0);

    puremvc_view_getInstance(storage, "view1");
    assert(strcmp(storage[1]->key, "view1") == 0);
    assert(strcmp(storage[1]->view.multitonKey, "view1") == 0);

    puremvc_view_getInstance(storage, "view2");
    assert(strcmp(storage[2]->key, "view2") == 0);
    assert(strcmp(storage[2]->view.multitonKey, "view2") == 0);

    puremvc_view_getInstance(storage, "view3");
    assert(strcmp(storage[3]->key, "view3") == 0);
    assert(strcmp(storage[3]->view.multitonKey, "view3") == 0);

    // remove
    puremvc_view_removeView(storage, "view1"); // remove middle, remaining 0, 2, 3
    assert(strcmp(storage[0]->key, "view0") == 0);
    assert(strcmp(storage[0]->view.multitonKey, "view0") == 0);
    assert(strcmp(storage[1]->key, "view2") == 0);
    assert(strcmp(storage[1]->view.multitonKey, "view2") == 0);
    assert(strcmp(storage[2]->key, "view3") == 0);
    assert(strcmp(storage[2]->view.multitonKey, "view3") == 0);
    assert(storage[3]->key[0] == '\0');

    puremvc_view_removeView(storage, "view3"); // remove last, remaining 0, 2
    assert(strcmp(storage[0]->key, "view0") == 0);
    assert(strcmp(storage[0]->view.multitonKey, "view0") == 0);
    assert(strcmp(storage[1]->key, "view2") == 0);
    assert(strcmp(storage[1]->view.multitonKey, "view2") == 0);
    assert(storage[2]->key[0] == '\0');
    assert(storage[3]->key[0] == '\0');

    puremvc_view_removeView(storage, "view0"); // remove first, remaining 2
    assert(strcmp(storage[0]->key, "view2") == 0);
    assert(strcmp(storage[0]->view.multitonKey, "view2") == 0);
    assert(storage[1]->key[0] == '\0');
    assert(storage[2]->key[0] == '\0');
    assert(storage[3]->key[0] == '\0');

    puremvc_view_removeView(storage, "view2"); // remove remaining
    assert(storage[0]->key[0] == '\0');
    assert(storage[1]->key[0] == '\0');
    assert(storage[2]->key[0] == '\0');
    assert(storage[3]->key[0] == '\0');
}

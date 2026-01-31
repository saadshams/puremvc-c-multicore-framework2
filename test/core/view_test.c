#include <assert.h>
#include <string.h>

#include "puremvc/view.h"
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
    testRegisterAndRemoveMultipleObservers();
    testRegisterAndRemoveMultipleMediators();
    return 0;
}

struct ViewTestVar *viewTestVar;

void viewTestMethod(const void *context, const struct Notification notification) {
    viewTestVar = (struct ViewTestVar *)notification.getBody(&notification);
}

void testGetInstance() {
    // Test Factory Method
    struct View *view = puremvc_view_getInstance("ViewTestKey1", puremvc_view);
    view->initializeView(view);

    // test assertions
    assert(view == puremvc_view_getInstance("ViewTestKey1", puremvc_view));

    puremvc_view_removeView("ViewTestKey1");
}

void testRegisterAndNotifyObserver() {
    // Get the Multiton View instance
    struct ViewComponent viewComponent = {};
    struct View *view = puremvc_view_getInstance("ViewTestKey2", puremvc_view);
    view->initializeView(view);

    // Create observer, passing in notification method and context
    struct Observer observer = puremvc_observer((void (*)(const void *, struct Notification)) viewTestMethod, &viewComponent);

    // Register Observer's interest in a particular Notification with the View
    view->registerObserver(view, "ViewTestNote1", observer);

    // Create a ViewTestNote, setting
    // a body value, and tell the View to notify
    // Observers. Since the Observer is this class
    // and the notification method is viewTestMethod,
    // successful notification will result in our local
    // viewTestVar being set to the value we pass in
    // on the note body.
    struct ViewTestVar vo = {.value = 10};
    const struct Notification note = puremvc_notification("ViewTestNote1", &vo, NULL);
    view->notifyObservers(view, note);

    // test assertions
    assert(viewTestVar->value == 10);

    // remove first observer
    viewTestVar->value = 0;
    view->removeObserver(view, "ViewTestNote1", &viewComponent);
    view->notifyObservers(view, note);
    assert(viewTestVar->value == 0);

    puremvc_view_removeView("ViewTestKey2");
    view = NULL;
}

void testRegisterAndRetrieveMediator() {
    // Get the Multiton View instance
    struct View *view = puremvc_view_getInstance("ViewTestKey3", puremvc_view);
    view->initializeView(view);

    // Create and register the test mediator
    struct ViewComponent viewComponent = {};
    view->registerMediator(view, puremvc_mediator("testing", &viewComponent));

    // Retrieve the component
    const struct Mediator *mediator = view->retrieveMediator(view, "testing");

    // test assertions
    assert(mediator != NULL);
    assert(mediator->getComponent(mediator) == &viewComponent);
    assert(strcmp(mediator->getName(mediator), "testing") == 0);

    // clean up
    view->removeMediator(view, "testing");
    puremvc_view_removeView("ViewTestKey3");
    view = NULL;
}

void testHasMediator() {
    // Get the Multiton View instance
    struct View *view = puremvc_view_getInstance("ViewTestKey4", puremvc_view);
    view->initializeView(view);

    // Create and register the test mediator
    struct ViewComponent viewComponent = {};
    struct Mediator mediator = puremvc_mediator("hasMediatorTest", &viewComponent);
    view->registerMediator(view, mediator);

    // assert that the view.hasMediator method returns true
    // for that mediator name
    assert(view->hasMediator(view, "hasMediatorTest") == true);

    view->removeMediator(view, "hasMediatorTest");

    // assert that the view.hasMediator method returns false
    // for that mediator name
    assert(view->hasMediator(view, "hasMediatorTest") == false);

    puremvc_view_removeView("ViewTestKey4");
    view = NULL;
}

void testRegisterAndRemoveMediator() {
    // Get the Multiton View instance
    struct View *view = puremvc_view_getInstance("ViewTestKey6", puremvc_view);
    view->initializeView(view);

    // Create and register the test mediator
    struct ViewComponent viewComponent = {};
    struct Mediator mediator = puremvc_mediator("testing", &viewComponent);
    view->registerMediator(view, mediator);

    // Remove the component
    const struct Mediator removedMediator = view->removeMediator(view, "testing");

    // assert that we have removed the appropriate mediator
    assert(strcmp(removedMediator.getName(&removedMediator), "testing") == 0);

    // assert that the mediator is no longer retrievable
    assert(view->retrieveMediator(view, "testing") == NULL);

    puremvc_view_removeView("ViewTestKey6");
    view = NULL;
}

void testOnRegisterAndOnRemove() {
    // Get the Multiton View instance
    struct View *view = puremvc_view_getInstance("ViewTestKey5", puremvc_view);
    view->initializeView(view);
    struct ViewTest viewTest = {"", false, false, 0};

    // Create and register the test mediator
    struct Mediator mediator = view_test_mediator4(&viewTest);
    view->registerMediator(view, mediator);

    // assert that onRegister was called, and the mediator responded by setting our boolean
    assert(viewTest.onRegisterCalled);

    // Remove the component
    view->removeMediator(view, view_test_mediator4_NAME);

    // assert that the mediator is no longer retrievable
    assert(view->retrieveMediator(view, view_test_mediator4_NAME) == NULL);

    // assert that onRemove was called, and the mediator responded by setting our boolean
    assert(viewTest.onRemoveCalled);

    // clean up
    puremvc_view_removeView("ViewTestKey5");
    view = NULL;
}

void testSuccessiveRegisterAndRemoveMediator() {
    // Get the Multiton View instance
    struct View *view = puremvc_view_getInstance("ViewTestKey7", puremvc_view);
    view->initializeView(view);

    // Create and register the test mediator,
    // but not so we have a reference to it
    struct ViewComponent viewComponent = {};
    // struct Mediator mediator = view_test_mediator(&viewComponent);

    view->registerMediator(view, view_test_mediator(&viewComponent));

    // test that we can retrieve it
    assert(view->retrieveMediator(view, view_test_mediator_NAME) != NULL);

    // Remove the Mediator
    view->removeMediator(view, view_test_mediator_NAME);

    // test that retrieving it now returns null
    assert(view->removeMediator(view, view_test_mediator_NAME).name[0] == '\0');

    // test that removing the mediator again once its gone doesn't cause crash
    assert(view->removeMediator(view, view_test_mediator_NAME).name[0] == '\0');

    // Create and register another instance of the test mediator,
    view->registerMediator(view, puremvc_mediator(view_test_mediator_NAME, &viewComponent));

    assert(view->retrieveMediator(view, view_test_mediator_NAME) != NULL);

    // Remove the Mediator
    view->removeMediator(view, view_test_mediator_NAME);

    // test that retrieving it now returns null
    assert(view->retrieveMediator(view, view_test_mediator_NAME) == NULL);

    puremvc_view_removeView("ViewTestKey7");
    view = NULL;
}

void testRemoveMediatorAndSubsequentNotify() {
    // Get the Multiton View instance
    struct View *view = puremvc_view_getInstance("ViewTestKey8", puremvc_view);
    view->initializeView(view);

    // Create and register the test mediator to be removed.
    struct ViewTest viewTest = {.lastNotification = NULL };
    view->registerMediator(view, view_test_mediator2(&viewTest));

    // test that notifications work
    struct Notification notification1 = puremvc_notification(NOTE1, NULL, NULL);
    view->notifyObservers(view, notification1);
    assert(strcmp(viewTest.lastNotification, NOTE1) == 0);

    struct Notification notification2 = puremvc_notification(NOTE2, NULL, NULL);
    view->notifyObservers(view, notification2);
    assert(strcmp(viewTest.lastNotification, NOTE2) == 0);

    view->removeMediator(view, view_test_mediator2_NAME);

    assert(view->retrieveMediator(view, view_test_mediator2_NAME) == NULL);

    viewTest.lastNotification = "";

    view->notifyObservers(view, notification1);
    assert(strcmp(viewTest.lastNotification, NOTE1) != 0);

    view->notifyObservers(view, notification2);
    assert(strcmp(viewTest.lastNotification, NOTE2) != 0);

    puremvc_view_removeView("ViewTestKey8");
    view = NULL;
}

void testRemoveOneOfTwoMediatorsAndSubsequentNotify() {
    // Get the Multiton View instance
    struct View *view = puremvc_view_getInstance("ViewTestKey9", puremvc_view);
    view->initializeView(view);
    struct ViewTest viewTest = {};

    // Create and register that responds to notifications 1 and 2
    // view->registerMediator(view, view_test_mediator2(&viewTest));

    // Create and register that responds to notification 3
    view->registerMediator(view, view_test_mediator3(&viewTest));

    // test that all notifications work
    struct Notification notification1 = puremvc_notification(NOTE1, NULL, NULL);
    // view->notifyObservers(view, &notification1);
    // assert(strcmp(viewTest.lastNotification, NOTE1) == 0);

    struct Notification notification2 = puremvc_notification(NOTE2, NULL, NULL);
    // view->notifyObservers(view, &notification2);
    // assert(strcmp(viewTest.lastNotification, NOTE2) == 0);

    struct Notification notification3 = puremvc_notification(NOTE3, NULL, NULL);
    // view->notifyObservers(view, &notification3);
    // assert(strcmp(viewTest.lastNotification, NOTE3) == 0);

    // Remove the Mediator that responds to 1 and 2
    view->removeMediator(view, view_test_mediator2_NAME);

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

    puremvc_view_removeView("ViewTestKey9");
    view = NULL;
}

void testMediatorReregistration() {
    // Get the Singleton View instance
    struct View *view = puremvc_view_getInstance("ViewTestKey10", puremvc_view);
    view->initializeView(view);

    // Create and register that responds to notification 5
    struct ViewTest viewTest = {};
    const struct Mediator mediator = view_test_mediator5(&viewTest);

    // try to register another instance of that mediator (uses the same NAME constant).
    view->registerMediator(view, mediator);

    // test that the counter is only incremented once (mediator 5's response)
    viewTest.counter = 0;
    struct Notification notification = puremvc_notification(NOTE5, NULL, NULL);
    view->notifyObservers(view, notification);
    assert(viewTest.counter == 1);

    // Remove the Mediator
    view->removeMediator(view, view_test_mediator5_NAME);

    // test that retrieving it now returns null
    assert(view->retrieveMediator(view, view_test_mediator5_NAME) == NULL);

    // test that the counter is no longer incremented
    viewTest.counter = 0;
    view->notifyObservers(view, notification);
    assert(viewTest.counter == 0);

    puremvc_view_removeView("ViewTestKey10");
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
    // Get the Singleton View instance
    struct View *view = puremvc_view_getInstance("ViewTestKey11", puremvc_view);
    view->initializeView(view);

    struct ViewTest viewTest = {"", "", "", 0, .deferred = {0}};

    // Create and register several mediator instances that respond to notification 6
    // by removing themselves, which will cause the observer list for that notification
    // to change.

    view->registerMediator(view, view_test_mediator6("view_test_mediator6/1", &viewTest));
    view->registerMediator(view, view_test_mediator6("view_test_mediator6/2", &viewTest));
    view->registerMediator(view, view_test_mediator6("view_test_mediator6/3", &viewTest));
    view->registerMediator(view, view_test_mediator6("view_test_mediator6/4", &viewTest));
    view->registerMediator(view, view_test_mediator6("view_test_mediator6/5", &viewTest));
    view->registerMediator(view, view_test_mediator6("view_test_mediator6/6", &viewTest));
    view->registerMediator(view, view_test_mediator6("view_test_mediator6/7", &viewTest));
    view->registerMediator(view, view_test_mediator6("view_test_mediator6/8", &viewTest));

    // send the notification. each of the above mediators will respond by removing
    // themselves and incrementing the counter by 1. This should leave us with a
    // count of 8, since 8 mediators will respond.
    const struct Notification notification = puremvc_notification(NOTE6, NULL, NULL);
    view->notifyObservers(view, notification);

    // iterate through deferred names and call removeMediator
    for (size_t i = 0; i < MEDIATOR_MAP_SIZE && viewTest.deferred[i][0] != '\0'; i++) {
        view->removeMediator(view, viewTest.deferred[i]);
    }

    // verify the count is correct
    assert(viewTest.counter == 8);

    // clear the counter
    viewTest.counter = 0;
    view->notifyObservers(view, notification);

    // verify the count is 0
    assert(viewTest.counter == 0);

    puremvc_view_removeView("ViewTestKey11");
    view = NULL;
}

void testRemoveView() {
    // Get a Multiton View instance
    struct View *view = puremvc_view_getInstance("ViewTestKey12", puremvc_view);
    view->initializeView(view);

    // remove the View
    puremvc_view_removeView("ViewTestKey12");

    // re-create the view without throwing an exception
    puremvc_view("ViewTestKey12");

    // cleanup
    puremvc_view_removeView("ViewTestKey12");
}

void testRegisterAndRemoveMultipleObservers() {
    struct View *view = puremvc_view_getInstance("ViewTestKey13", puremvc_view);
    view->initializeView(view);

    struct Observer observer = puremvc_observer((void (*)(const void *, struct Notification)) viewTestMethod, view);

    view->removeObserver(view, "observer0", view); // crash test
    view->removeObserver(view, "observer0", view);

    // register one observer and remove it
    view->registerObserver(view, "observer0", observer);
    assert(strcmp(view->observerMap[0].key, "observer0") == 0);
    assert(view->observerMap[0].observers[0].context == view);
    view->removeObserver(view, "observer0", view);
    assert(view->observerMap[0].key[0] == '\0');
    assert(view->observerMap[0].observers[0].context == NULL);;

    // register four observers, check association and remove them
    view->registerObserver(view, "observer1", observer);
    assert(strcmp(view->observerMap[0].key, "observer1") == 0);
    assert(view->observerMap[0].observers[0].context == view);

    view->registerObserver(view, "observer2", observer);
    assert(strcmp(view->observerMap[1].key, "observer2") == 0);
    assert(view->observerMap[1].observers[0].context == view);

    view->registerObserver(view, "observer3", observer);
    assert(strcmp(view->observerMap[2].key, "observer3") == 0);
    assert(view->observerMap[2].observers[0].context == view);

    view->registerObserver(view, "observer4", observer);
    assert(strcmp(view->observerMap[3].key, "observer4") == 0);
    assert(view->observerMap[3].observers[0].context == view);

    // remove the second (middle) observer and verify that remaining mediators 3, 4 are shifted correctly
    view->removeObserver(view, "observer2", view);
    assert(strcmp(view->observerMap[0].key, "observer1") == 0);
    assert(strcmp(view->observerMap[1].key, "observer3") == 0);
    assert(strcmp(view->observerMap[2].key, "observer4") == 0);

    // Remove the last mediator and verify the remaining mediators 1, 3 stay in place
    view->removeObserver(view, "observer4", view);
    assert(strcmp(view->observerMap[0].key, "observer1") == 0);
    assert(strcmp(view->observerMap[1].key, "observer3") == 0);

    // Remove the first mediator and verify that subsequent mediator 3 shift left
    view->removeObserver(view, "observer1", view);
    assert(strcmp(view->observerMap[0].key, "observer3") == 0);

    // Remove the remaining mediator
    view->removeObserver(view, "observer3", view);
    assert(view->observerMap[0].key[0] == '\0');

    puremvc_view_removeView("ViewTestKey13");
    view = NULL;
}

void testRegisterAndRemoveMultipleMediators() {
    struct View *view = puremvc_view_getInstance("ViewTestKey14", puremvc_view);
    view->initializeView(view);

    assert(view->retrieveMediator(view, "mediator0") == NULL); // crash test
    assert(view->retrieveMediator(view, "mediator0") == NULL);

    // Register one mediator, verify associations and remove it
    view->registerMediator(view, view_test_mediator7("mediator0", NULL));
    assert(strcmp(view->mediatorMap[0].mediator.name, "mediator0") == 0);
    assert(strcmp(((struct Mediator *)view->observerMap[0].observers[0].context)->name, "mediator0") == 0);
    view->removeMediator(view, "mediator0");
    assert(view->hasMediator(view, "mediator0") == false);
    assert(view->mediatorMap[0].key[0] == '\0');

    // Register four mediators and verify that each is correctly associated to their observers
    view->registerMediator(view, view_test_mediator7("mediator1", NULL));
    assert(strcmp(view->mediatorMap[0].mediator.name, "mediator1") == 0);
    assert(strcmp(((struct Mediator *)view->observerMap[0].observers[0].context)->name, "mediator1") == 0);

    view->registerMediator(view, view_test_mediator7("mediator2", NULL));
    assert(strcmp(view->mediatorMap[1].mediator.name, "mediator2") == 0);
    assert(strcmp(((struct Mediator *)view->observerMap[0].observers[1].context)->name, "mediator2") == 0);

    view->registerMediator(view, view_test_mediator7("mediator3", NULL));
    const struct Mediator *mediator3 = view->retrieveMediator(view, "mediator3");
    assert(strcmp(view->mediatorMap[2].mediator.name, "mediator3") == 0);
    assert(strcmp(((struct Mediator *)view->observerMap[0].observers[2].context)->name, "mediator3") == 0);

    view->registerMediator(view, view_test_mediator7("mediator4", NULL));
    assert(strcmp(view->mediatorMap[3].mediator.name, "mediator4") == 0);
    assert(strcmp(((struct Mediator *)view->observerMap[0].observers[3].context)->name, "mediator4") == 0);

    // Verify the dictionary key for the map is correctly set
    assert(strcmp(view->observerMap[0].key, NOTE7) == 0);

    // Remove the second mediator (middle) and verify that remaining mediators 3, 4 are shifted correctly
    view->removeMediator(view, "mediator2");
    assert(strcmp(view->mediatorMap[0].mediator.name, "mediator1") == 0);
    assert(strcmp(((struct Mediator *)view->observerMap[0].observers[0].context)->name, "mediator1") == 0);
    assert(strcmp(view->mediatorMap[1].mediator.name, "mediator3") == 0);
    assert(strcmp(((struct Mediator *)view->observerMap[0].observers[1].context)->name, "mediator3") == 0);
    assert(strcmp(view->mediatorMap[2].mediator.name, "mediator4") == 0);
    assert(strcmp(((struct Mediator *)view->observerMap[0].observers[2].context)->name, "mediator4") == 0);
    assert(view->observerMap[0].key[0] != '\0'); // Key persists while observers exist

    // Remove the last mediator and verify the remaining mediators 1, 3 stay in place
    view->removeMediator(view, "mediator4");
    assert(strcmp(view->mediatorMap[0].mediator.name, "mediator1") == 0);
    assert(strcmp(((struct Mediator *)view->observerMap[0].observers[0].context)->name, "mediator1") == 0);
    assert(strcmp(view->mediatorMap[1].mediator.name, "mediator3") == 0);
    assert(strcmp(((struct Mediator *)view->observerMap[0].observers[1].context)->name, "mediator3") == 0);
    assert(view->mediatorMap[2].key[0] == '\0'); // last mediator deleted
    assert(view->observerMap[0].key[0] != '\0'); // Key persists while observers exist

    // Remove the first mediator and verify that subsequent mediator 3 shift left
    view->removeMediator(view, "mediator1");
    assert(strcmp(view->mediatorMap[0].mediator.name, "mediator3") == 0);
    assert(strcmp(((struct Mediator *)view->observerMap[0].observers[0].context)->name, "mediator3") == 0);

    // Remove all remaining mediators and confirm that the dictionary key is cleared
    view->removeMediator(view, "mediator3");
    assert(view->observerMap[0].key[0] == '\0'); // Dictionary key cleared observers are empty

    puremvc_view_removeView("ViewTestKey14");
    view = NULL;
}

void testGetAndRemoveMultipleInstances() {
    puremvc_view_getInstance("view1", puremvc_view);
    puremvc_view_getInstance("view2", puremvc_view);
    puremvc_view_getInstance("view3", puremvc_view);
    puremvc_view_getInstance("view4", puremvc_view);

    puremvc_view_removeView("view2"); // remove middle
    puremvc_view_removeView("view4"); // remove last
    puremvc_view_removeView("view1"); // remove first
    puremvc_view_removeView("view3"); // remove remaining
}

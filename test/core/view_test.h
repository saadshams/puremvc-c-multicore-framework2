#pragma once

#include <stdbool.h>

#define NOTE1 "Notification1"
#define NOTE2 "Notification2"
#define NOTE3 "Notification3"
#define NOTE4 "Notification4"
#define NOTE5 "Notification5"
#define NOTE6 "Notification6"

void testGetInstance();
void testRegisterAndNotifyObserver();
void testRegisterAndRetrieveMediator();
void testHasMediator();
void testRegisterAndRemoveMediator();
void testOnRegisterAndOnRemove();
void testSuccessiveRegisterAndRemoveMediator();
void testRemoveMediatorAndSubsequentNotify();
void testRemoveOneOfTwoMediatorsAndSubsequentNotify();
void testMediatorReregistration();
void testModifyObserverListDuringNotification();
void testRemoveView();

struct ViewTest {
    const char *lastNotification;
    bool onRegisterCalled;
    bool onRemoveCalled;
    int counter;
};

struct ViewTestVar {
    int value;
};

struct ViewComponent {
    int x;
};

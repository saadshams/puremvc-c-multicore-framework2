#pragma once

#include <stdbool.h>

#define NOTE1 "Notification1"
#define NOTE2 "Notification2"
#define NOTE3 "Notification3"
#define NOTE4 "Notification4"
#define NOTE5 "Notification5"
#define NOTE6 "Notification6"
#define NOTE7 "Notification7"

void testGetInstance(void);
void testRegisterAndNotifyObserver(void);
void testRegisterAndRetrieveMediator(void);
void testHasMediator(void);
void testRegisterAndRemoveMediator(void);
void testOnRegisterAndOnRemove(void);
void testSuccessiveRegisterAndRemoveMediator(void);
void testRemoveMediatorAndSubsequentNotify(void);
void testRemoveOneOfTwoMediatorsAndSubsequentNotify(void);
void testMediatorReregistration(void);
void testModifyObserverListDuringNotification(void);
void testRemoveView(void);
void testObserverMapShiftLeft(void);
void testObserverShiftLeft(void);
void testMediatorMapShiftLeft(void);
void testViewMapShiftLeft(void);

struct ViewTest {
    const char *lastNotification;
    bool onRegisterCalled;
    bool onRemoveCalled;
    int counter;
    char **deferred;
};

struct ViewTestVar {
    int value;
};

struct ViewComponent {
    int x;
};

#ifndef PUREMVC_VIEW_TEST_H
#define PUREMVC_VIEW_TEST_H

#include <stdbool.h>

int main();

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

#endif //PUREMVC_VIEW_TEST_H

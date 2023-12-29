#include "view_test_component.h"

void viewTestMethod(void *context, struct Notification *notification) {
    viewTestVar1 = notification->getBody(notification);
}
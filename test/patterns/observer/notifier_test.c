#include "notifier_test.h"
#include "puremvc/facade.h"
#include "puremvc/notifier.h"
#include "puremvc/simple_command.h"

#include <assert.h>

int main() {
    // testInstance();
    // testRegisterCommandAndSendNotification();
    return 0;
}

struct Object {
    int value;
    int result;
};

static void execute(const struct ICommand *self, struct INotification *notification) {
    struct Object *temp = (struct Object *)notification->getBody(notification);

    // fabricate a result
    temp->result = temp->value * 4;
}

static struct SimpleCommand command() {
    struct SimpleCommand command = puremvc_simple_command();
    command.base.execute = execute;
    return command;
}

// void testInstance() {
//     // create notifier instance
//     struct Notifier n = puremvc_notifier();
//     struct INotifier *notifier = &n.base;
//
//     // initialize facade
//     notifier->initializeNotifier(notifier, "NotifierTest1");
//     notifier->getFacade(notifier);
//
//     assert(notifier->getFacade(notifier) != NULL);
//
//     puremvc_facade_removeFacade("NotifierTest1");
// }

// void testRegisterCommandAndSendNotification() {
//     // create a notifier
//     struct Notifier notifier = puremvc_notifier();
//
//     // initialize facade
//     notifier.base.initializeNotifier(&notifier.base, "NotifierTest2");
//
//     struct Object temp = {4};
//
//     // get facade instance
//     struct IFacade *facade = notifier.base.getFacade(&notifier.base);
//
//     // register a command and send notification
//     facade->registerCommand(facade, "TestNote", command);
//     notifier.base.sendNotification(&notifier.base, "TestNote", &temp, NULL);
//
//     // assert result
//     assert(temp.result == 16);
//
//     facade->removeCommand(facade, "TestNote");
//     puremvc_facade_removeFacade("NotifierTest2");
// }

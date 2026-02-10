#include "notifier_test.h"
#include "../../../src/patterns/facade/facade.h"
#include "puremvc/i_notifier.h"
#include "puremvc/i_command.h"

#include <alloca.h>
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

static struct ICommand *command() {
    struct ICommand *command = puremvc_simple_command_init(alloca(puremvc_simple_command_size()));
    command->execute = execute;
    return command;
}

void testInstance() {
    // create notifier instance
    struct INotifier *notifier = puremvc_notifier_init(alloca(puremvc_notifier_size()));

    // initialize facade
    notifier->initializeNotifier(notifier, "NotifierTest1");
    // notifier->getFacade(notifier);

    // assert(notifier->getFacade(notifier) != NULL);

    // puremvc_facade_removeFacade("NotifierTest1");
}

// void testRegisterCommandAndSendNotification() {
//     // create a notifier
//     struct Notifier notifier = puremvc_notifier();
//
//     // initialize facade
//     notifier.initializeNotifier(&notifier, "NotifierTest2");
//
//     struct Object temp = {4};
//
//     // get facade instance
//     struct IFacade *facade = notifier.getFacade(&notifier);
//
//     // register a command and send notification
//     facade->registerCommand(facade, "TestNote", command);
//     notifier.sendNotification(&notifier, "TestNote", &temp, NULL);
//
//     // assert result
//     assert(temp.result == 16);
//
//     facade->removeCommand(facade, "TestNote");
//     puremvc_facade_removeFacade("NotifierTest2");
// }

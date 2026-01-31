#include <assert.h>
#include <stdio.h>

#include "notifier_test.h"
#include "puremvc/notifier.h"

int main() {
    // testInstance();
    // testRegisterCommandAndSendNotification();
    return 0;
}

struct Object {
    int value;
    int result;
};
//
// static void execute(const struct ICommand *self, struct INotification *notification, const char **error) {
//     struct Object *temp = (struct Object *)notification->getBody(notification);
//
//     // fabricate a result
//     temp->result = temp->value * 4;
// }
//
// static struct ICommand *command_new() {
//     const char *error = NULL;
//     struct ICommand *command = puremvc_simple_command_new(&error);
//     command->execute = execute;
//     return command;
// }
//
// void testInstance() {
//     // create notifier instance
//     const char *error = NULL;
//     struct INotifier *notifier = puremvc_notifier_new(&error);
//
//     // initialize facade
//     notifier->initializeNotifier(notifier, "NotifierTest1", &error);
//     notifier->getFacade(notifier, &error);
//
//     assert(notifier != NULL);
//     assert(notifier->getFacade(notifier, &error) != NULL);
//
//     puremvc_facade_removeFacade("NotifierTest1");
//     puremvc_notifier_free(&notifier);
// }
//
// void testRegisterCommandAndSendNotification() {
//     // create a notifier
//     const char *error = NULL;
//     struct INotifier *notifier = puremvc_notifier_new(&error);
//
//     // initialize facade
//     notifier->initializeNotifier(notifier, "NotifierTest2", &error);
//
//     struct Object temp = {4};
//
//     // get facade instance
//     struct IFacade *facade = notifier->getFacade(notifier, &error);
//
//     // register a command and send notification
//     facade->registerCommand(facade, "TestNote", command_new, &error);
//     notifier->sendNotification(notifier, "TestNote", &temp, NULL, &error);
//
//     // assert result
//     assert(temp.result == 16);
//
//     facade->removeCommand(facade, "TestNote");
//     puremvc_facade_removeFacade("NotifierTest2");
//     puremvc_notifier_free(&notifier);
// }

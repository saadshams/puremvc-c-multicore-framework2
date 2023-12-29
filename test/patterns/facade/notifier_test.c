#include "../../../include/notifier.h"
#include <assert.h>
#include <stdio.h>

void testInstance() {
    // create notifier instance
    struct Notifier *notifier = puremvc_notifier_new();

    // initialize facade
    notifier->initializeNotifier(notifier, "Test1");

    // assert
    assert(notifier != NULL);
    assert(notifier->getFacade(notifier) != NULL);

    puremvc_facade_removeFacade("Test1");
    puremvc_notifier_free(&notifier);
}

typedef struct {
    int value;
    int result;
} Object;

static void execute(struct SimpleCommand *self, struct Notification *notification) {
    Object *temp = notification->getBody(notification);

    // fabricate a result
    temp->result = temp->value * 4;
}

static struct SimpleCommand *command_new() {
    struct SimpleCommand *command = puremvc_simple_command_new();
    command->execute = execute;
    return command;
}

void testRegisterCommandAndSendNotification() {
    // create a notifier
    struct Notifier *notifier = puremvc_notifier_new();

    // initialize facade
    notifier->initializeNotifier(notifier, "Test2");

    Object temp = {4};

    // get facade instance
    struct Facade *facade = notifier->getFacade(notifier);

    // register a command and send notification
    facade->registerCommand(facade, "TestNote", command_new);
    notifier->sendNotification(notifier, "TestNote", &temp, NULL);

    // assert result
    assert(temp.result == 16);

    facade->removeCommand(facade, "TestNote");
    puremvc_facade_removeFacade("Test2");
    puremvc_notifier_free(&notifier);
}

int main() {
    testInstance();
    testRegisterCommandAndSendNotification();
    return 0;
}

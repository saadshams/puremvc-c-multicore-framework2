#include "notifier_test.h"

#include "puremvc/i_facade.h"
#include "puremvc/i_notifier.h"
#include "puremvc/i_command.h"

#include <alloca.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test(const char *name, void (*callback)(void)) {
    printf("\033[0;34m[RUNNING]\033[0m %s...\n", name);
    fflush(stdout);

    callback();

    printf("\033[0;32m[PASSED]\033[0m %s\n", name);
    fflush(stdout);
}

int main() {
    printf("\n\033[1;36m================================================\033[0m\n");
    printf("\033[1;36m[SUITE] %s\033[0m\n", "NotifierTest");
    printf("\033[1;36m================================================\033[0m\n\n");

    test("testInstance", testInstance);
    // test("testRegisterCommandAndSendNotification", testRegisterCommandAndSendNotification); //

    printf("\n\033[1;32m[DONE] All tests in suite finished.\033[0m\n");
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
    if (strcmp(notifier->getMultitonKey(notifier), "NotifierTest1") != 0)
        abort();

    if (notifier->getFacade(notifier) == NULL) abort();

    puremvc_facade_removeFacade("NotifierTest1", NULL);
}

void testRegisterCommandAndSendNotification() {
    // create a notifier
    struct INotifier *notifier = puremvc_notifier_init(alloca(puremvc_notifier_size()));

    // initialize facade
    notifier->initializeNotifier(notifier, "NotifierTest2");

    struct Object temp = {4};

    // get facade instance
    // const struct IFacade *facade = notifier->getFacade(notifier);

    // register a command and send notification
    // assert(facade->registerCommand(facade, "TestNote", command) == 0);
    // notifier->sendNotification(notifier, "TestNote", &temp, NULL);

    // assert result
    // assert(temp.result == 16);

    // facade->removeCommand(facade, "TestNote", NULL);
    // puremvc_facade_removeFacade("NotifierTest2", NULL);
}

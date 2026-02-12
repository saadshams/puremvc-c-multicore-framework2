#include "notifier_test.h"

#include "puremvc/platform.h"
#include "puremvc/i_facade.h"
#include "puremvc/i_notifier.h"
#include "puremvc/i_observer.h"
#include "puremvc/i_command.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void beforeAll() {}
static void beforeEach() {}
static void afterEach() {}
static void afterAll() {}

static void test(const char *name, void (*callback)(void)) {
    printf("\033[0;34m[RUNNING]\033[0m %s...\n", name);
    fflush(stdout);

    beforeEach();
    callback();
    afterEach();

    printf("\033[0;32m[PASSED]\033[0m %s\n", name);
    fflush(stdout);
}

int main() {
    printf("\n\033[1;36m================================================\033[0m\n");
    printf("\033[1;36m[SUITE] %s\033[0m\n", "NotifierTest");
    printf("\033[1;36m================================================\033[0m\n\n");

    beforeAll();
    test("testInstance", testInstance);
    test("testRegisterCommandAndSendNotification", testRegisterCommandAndSendNotification);
    afterAll();

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

static struct ICommand *notifier_command() {
    struct ICommand *command = puremvc_simple_command_init(alloca(puremvc_simple_command_size()));
    command->execute = execute;
    return command;
}

void testInstance() {
    struct FacadeMap **facadeMap = (struct FacadeMap *[]) {
        &(struct FacadeMap){ .facade = alloca(puremvc_facade_size())},
        NULL
    };
    struct IFacade *facade = puremvc_facade_getInstance(facadeMap, "NotifierTest1");
    facade->initializeFacade(facade, NULL, NULL, NULL);

    // create notifier instance
    struct INotifier *notifier = puremvc_notifier_init(alloca(puremvc_notifier_size()));

    // initialize facade
    notifier->initializeNotifier(notifier, "NotifierTest1");
    if (strcmp(notifier->getMultitonKey(notifier), "NotifierTest1") != 0) abort();

    if (notifier->getFacade(notifier) == NULL) abort();

    puremvc_facade_removeFacade("NotifierTest1", NULL);
}

void testRegisterCommandAndSendNotification() {
    struct ObserverMap **observerMap = (struct ObserverMap *[]) {
        &(struct ObserverMap) {
            .observers = (struct IObserver *[]){
                memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()),
                NULL
            }
        }, NULL };
    struct ViewMap **viewMap = (struct ViewMap *[]) {
        &(struct ViewMap){ .view = alloca(puremvc_view_size())},
        NULL
    };
    struct IView *view = puremvc_view_getInstance(viewMap, "NotifierTest2");
    view->initializeView(view, observerMap, NULL);

    struct ControllerMap **controllerMap = (struct ControllerMap *[]) {
        &(struct ControllerMap){ .controller = alloca(puremvc_controller_size()) },
        NULL
    };
    struct CommandMap **commandMap = (struct CommandMap *[]) {
        &(struct CommandMap){},
        NULL
    };

    struct IController *controller = puremvc_controller_getInstance(controllerMap, "NotifierTest2");
    controller->initializeController(controller, view, commandMap);

    struct FacadeMap **facadeMap = (struct FacadeMap *[]) {
        &(struct FacadeMap){ .facade = alloca(puremvc_facade_size()) },
        NULL
    };
    struct IFacade *facade = puremvc_facade_getInstance(facadeMap, "NotifierTest2");
    facade->initializeFacade(facade, NULL, view, controller);

    // create a notifier
    struct INotifier *notifier = puremvc_notifier_init(alloca(puremvc_notifier_size()));

    // initialize facade
    notifier->initializeNotifier(notifier, "NotifierTest2");

    struct Object temp = {4};

    // get facade instance
    const struct IFacade *facade2 = notifier->getFacade(notifier);

    // register a command and send notification
    if (facade2->registerCommand(facade, "TestNote", notifier_command) == false) abort();
    notifier->sendNotification(notifier, "TestNote", &temp, NULL);

    // assert result
    if (temp.result != 16) abort();

    facade->removeCommand(facade, "TestNote", NULL);
    puremvc_facade_removeFacade("NotifierTest2", NULL);
}

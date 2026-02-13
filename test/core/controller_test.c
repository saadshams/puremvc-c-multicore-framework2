#include "controller_test.h"
#include "controller_test_command.h"
#include "controller_test_command2.h"
#include "controller_test_vo.h"

#include "puremvc/platform.h"
#include "puremvc/i_controller.h"
#include "puremvc/i_view.h"
#include "puremvc/i_observer.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
    beforeAll();
    printf("\n\033[1;36m================================================\033[0m\n");
    printf("\033[1;36m[SUITE] %s\033[0m\n", "ControllerTest");
    printf("\033[1;36m================================================\033[0m\n\n");

    test("testGetInstance", testGetInstance);
    test("testRegisterAndExecuteCommand", testRegisterAndExecuteCommand);
    test("testRegisterAndRemoveCommand", testRegisterAndRemoveCommand);
    test("testHasCommand", testHasCommand);
    test("testReregisterAndExecuteCommand", testReregisterAndExecuteCommand);
    test("testRegisterAndUpdateCommand", testRegisterAndUpdateCommand);
    test("testRemoveController", testRemoveController);
    // test("testCommandMapShiftLeft", testCommandMapShiftLeft);
    // test("TestControllerMapShiftLeft", TestControllerMapShiftLeft);

    printf("\n\033[1;32m[DONE] All tests in suite finished.\033[0m\n");
    afterAll();
    return 0;
}

void testGetInstance() {
    struct ViewMap **viewMap = (struct ViewMap *[]) {
        &(struct ViewMap){ .view = alloca(puremvc_view_size()) },
        NULL
    };

    // Test Factory Method
   struct IView *view = puremvc_view_getInstance(viewMap, "ControllerTestKey1"); // pre-initialize View for the Controller

    struct ControllerMap **controllerMap = (struct ControllerMap *[]) {
        &(struct ControllerMap){ .controller = alloca(puremvc_controller_size()) },
        NULL
    };

    struct CommandMap **commandMap = (struct CommandMap *[]) {
        &(struct CommandMap){},
        NULL
    };

    // Test Factory Method
    struct IController *controller = puremvc_controller_getInstance(controllerMap, "ControllerTestKey1");
    controller->initializeController(controller, view, commandMap);

    // test assertions
    if (controller == NULL) abort();
    if (controller != puremvc_controller_getInstance(controllerMap, "ControllerTestKey1")) abort();

    struct IController *removedController = NULL;
    if (puremvc_controller_removeController("ControllerTestKey1", &removedController) != true) abort();

    struct IView *removedView = NULL;
    if (puremvc_view_removeView("ControllerTestKey1", &removedView) != true) abort();
}

void testRegisterAndExecuteCommand() {
    struct ViewMap **viewMap = (struct ViewMap *[]) {
        &(struct ViewMap){ .view = alloca(puremvc_view_size()) },
        NULL
    };

    struct ObserverMap **observerMap = (struct ObserverMap *[]) {
        &(struct ObserverMap) {
            .observers = (struct IObserver *[]) {
                memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()),
                NULL
            }
        },
        NULL
    };

    struct IView *view = puremvc_view_getInstance(viewMap, "ControllerTestKey2");
    view->initializeView(view, observerMap, NULL);

    struct ControllerMap **controllerMap = (struct ControllerMap *[]) {
        &(struct ControllerMap) { .controller = alloca(puremvc_controller_size()) },
        NULL
    };

    struct CommandMap **commandMap = (struct CommandMap *[]) {
        &(struct CommandMap){},
        NULL
    };

    // Create the controller, register the ControllerTestCommand to handle 'ControllerTest' notes
    struct IController *controller = puremvc_controller_getInstance(controllerMap, "ControllerTestKey2");
    if (controller == NULL) abort();

    controller->initializeController(controller, view, commandMap);

    if (controller->registerCommand(controller, "ControllerTest1", test_controller_command_init) != true)
        abort();

    // Create a 'ControllerTest' note
    struct ControllerTestVO vo = {12, 0};
    struct INotification *notification = puremvc_notification_init(alloca(puremvc_notification_size()), "ControllerTest1", &vo, NULL);

    // Tell the controller to execute the Command associated with the note
    // the ControllerTestCommand invoked will multiply the vo.input value
    // by 2 and set the result on vo.result
    controller->executeCommand(controller, notification);

    // test assertions
    if (vo.result != 24) abort();

    struct ICommand *(*factory)(void *) = 0;
    if (controller->removeCommand(controller, "ControllerTest1", &factory) != true) abort();;

    if (puremvc_controller_removeController("ControllerTestKey2", NULL) != true) abort();
    if (puremvc_view_removeView("ControllerTestKey2", NULL) != true) abort();;
}

void testRegisterAndRemoveCommand() {
   struct ViewMap **viewMap = (struct ViewMap *[]) {
       &(struct ViewMap){ .view = alloca(puremvc_view_size()) },
       NULL
   };

    struct ObserverMap **observerMap = (struct ObserverMap *[]) {
        &(struct ObserverMap) {
            .observers = (struct IObserver *[]) {
                memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()),
                NULL
            }
        },
        NULL
    };

    // Initialize View before Controller (same multiton key)
    struct IView *view = puremvc_view_getInstance(viewMap, "ControllerTestKey3"); // dependency for the controller
    view->initializeView(view, observerMap, NULL);

    struct ControllerMap **controllerMap = (struct ControllerMap *[]) {
        &(struct ControllerMap){ .controller = alloca(puremvc_controller_size()) },
        NULL
    };

    struct CommandMap **commandMap = (struct CommandMap *[]) {
        &(struct CommandMap){},
        NULL
    };

    // Create the controller, register the ControllerTestCommand to handle 'ControllerTest' notes
    struct IController *controller = puremvc_controller_getInstance(controllerMap, "ControllerTestKey3");
    controller->initializeController(controller, view, commandMap);

    if (controller->registerCommand(controller, "ControllerRemoveTest", test_controller_command_init) != true)
        abort();;

    // Create a 'ControllerTest' note
    struct ControllerTestVO vo = {12, 0};
    struct INotification *notification = puremvc_notification_init(alloca(puremvc_notification_size()), "ControllerRemoveTest", &vo, NULL);

    // Tell the controller to execute the Command associated with the note
    // the ControllerTestCommand invoked will multiply the vo.input value
    // by 2 and set the result on vo.result
    controller->executeCommand(controller, notification);

    // test assertions
    if (vo.result != 24) abort();

    // Reset result
    vo.result = 0;

    // Remove the Command from the Controller
    if (controller->removeCommand(controller, "ControllerRemoveTest", NULL) != true) abort();;

    // Tell the controller to execute the Command associated with the
    // note. This time, it should not be registered, and our vo result
    // will not change
    controller->executeCommand(controller, notification);

    // test assertions
    if (vo.result != 0) abort();
    if (puremvc_controller_removeController("ControllerTestKey3", NULL) != true) abort();;
    if (puremvc_view_removeView("ControllerTestKey3", NULL) != true) abort();
}

void testHasCommand() {
    struct ViewMap **viewMap = (struct ViewMap *[]) {
        &(struct ViewMap){ .view = alloca(puremvc_view_size()) },
        NULL
    };

    struct ObserverMap **observerMap = (struct ObserverMap *[]) {
        &(struct ObserverMap) { .observers = (struct IObserver *[]){ memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()), NULL } },
        NULL
    };

    struct IView *view = puremvc_view_getInstance(viewMap, "ControllerTestKey4"); // dependency for the Controller
    view->initializeView(view, observerMap, NULL);

    struct ControllerMap **controllerMap = (struct ControllerMap *[]) {
        &(struct ControllerMap){ .controller = alloca(puremvc_controller_size()) },
        NULL
    };

    struct CommandMap **commandMap = (struct CommandMap *[]) {
        &(struct CommandMap){},
        NULL
    };

    // register the ControllerTestCommand to handle 'hasCommandTest' notes
    struct IController *controller = puremvc_controller_getInstance(controllerMap, "ControllerTestKey4");
    controller->initializeController(controller, view, commandMap);

    // test that hasCommand returns true for hasCommandTest notifications
    if (controller->registerCommand(controller, "hasCommandTest", test_controller_command_init) != true)
        abort();;
    if (controller->hasCommand(controller, "hasCommandTest") != true) abort();

    // Remove the Command from the Controller
    if (controller->removeCommand(controller, "hasCommandTest", NULL) != true) abort();;

    // test that hasCommand returns false for hasCommandTest notifications
    if (controller->hasCommand(controller, "hasCommandTest") != false) abort();

    if (puremvc_controller_removeController("ControllerTestKey4", NULL) != true) abort();;
    if (puremvc_view_removeView("ControllerTestKey4", NULL) != true) abort();;
}

void testReregisterAndExecuteCommand() {
    struct ViewMap **viewMap = (struct ViewMap *[]) { &(struct ViewMap){ .view = alloca(puremvc_view_size()) }, NULL };

    struct ObserverMap **observerMap = (struct ObserverMap *[]) {
        &(struct ObserverMap){
            .observers = (struct IObserver *[]){ memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()), NULL },
        },
        NULL
    };

    // Test Factory Method
    struct IView *view = puremvc_view_getInstance(viewMap, "ControllerTestKey5");
    view->initializeView(view, observerMap, NULL);

    struct ControllerMap **controllerMap = (struct ControllerMap *[]) {
        &(struct ControllerMap){ .controller = alloca(puremvc_controller_size()) },
        NULL
    };
    struct CommandMap **commandMap = (struct CommandMap *[]) { &(struct CommandMap){}, NULL };

    // Fetch the controller, register the ControllerTestCommand to handle 'ControllerTest2' notes
    struct IController *controller = puremvc_controller_getInstance(controllerMap, "ControllerTestKey5");
    controller->initializeController(controller, view, commandMap);

    if (controller->registerCommand(controller, "ControllerTest2", test_controller_command2_init) != true)
        abort();

    // Remove the Command from the Controller
    if (controller->removeCommand(controller, "ControllerTest2", NULL) != true) abort();

    // Re-register the Command with the Controller
    if (controller->registerCommand(controller, "ControllerTest2", test_controller_command2_init) != true)
        abort();

    // Create a 'ControllerTest2' note
    struct ControllerTestVO vo = {12, 0};
    const struct INotification *notification = puremvc_notification_init(alloca(puremvc_notification_size()), "ControllerTest2", &vo, NULL);

    // retrieve a reference to the View from the same core.
    const struct IView *view2 = puremvc_view_getInstance(viewMap, "ControllerTestKey5");
    if (view2 == NULL) abort();

    view2->notifyObservers(view2, notification);

    // test assertions
    // if the command is executed once the value will be 24
    if (vo.result != 24) abort();

    // Prove that accumulation works in the VO by sending the notification again
    view->notifyObservers(view, notification);

    // if the command is executed twice the value will be 48
    if (vo.result != 48) abort();

    if (controller->removeCommand(controller, "ControllerTest2", NULL) != true) abort();
    if (puremvc_controller_removeController("ControllerTestKey5", NULL) != true) abort();
    if (puremvc_view_removeView("ControllerTestKey5", NULL) != true) abort();;
}

void testRegisterAndUpdateCommand() {
    struct ViewMap **viewMap = (struct ViewMap *[]) {
        &(struct ViewMap){ .view = alloca(puremvc_view_size()) },
        NULL
    };

    struct ObserverMap **observerMap = (struct ObserverMap *[]) {
        &(struct ObserverMap){
            .observers = (struct IObserver *[]){
                memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()),
                NULL
            },
        },
        NULL
    };

    struct IView *view = puremvc_view_getInstance(viewMap, "ControllerTestKey6");
    view->initializeView(view, observerMap, NULL);

    struct ControllerMap **controllerMap = (struct ControllerMap *[]) {
        &(struct ControllerMap){ .controller = alloca(puremvc_controller_size()) },
        NULL
    };

    struct CommandMap **commandMap = (struct CommandMap *[]) {
        &(struct CommandMap){},
        NULL
    };

    struct IController *controller = puremvc_controller_getInstance(controllerMap, "ControllerTestKey6");
    controller->initializeController(controller, view, commandMap);

    // first registration
    if (controller->registerCommand(controller, "ControllerTest2", test_controller_command_init) != true)
        abort();

    // update command (a warning will be generated as it overrides)
    if (controller->registerCommand(controller, "ControllerTest2", test_controller_command2_init) != true)
        abort();

    struct ControllerTestVO vo = {12, 10};
    struct INotification *notification = puremvc_notification_init(alloca(puremvc_notification_size()), "ControllerTest2", &vo, NULL);
    controller->executeCommand(controller, notification);

    // second command result
    if (vo.result != 34) abort();

    if (controller->removeCommand(controller, "ControllerTest2", NULL) != true) abort();;
    if (puremvc_controller_removeController("ControllerTestKey6", NULL) != true) abort();;
    if (puremvc_view_removeView("ControllerTestKey6", NULL) != true) abort();;
}

void testRemoveController() {
    // NOTE: Successful execution will produce several expected error logs
    struct ControllerMap **controllerMap = (struct ControllerMap *[]) {
        &(struct ControllerMap){ .controller = alloca(puremvc_controller_size()) },
        NULL
    };

    struct CommandMap **commandMap = (struct CommandMap *[]) { &(struct CommandMap){}, NULL };

    // Get a Multiton Controller instance
    struct IController *controller = puremvc_controller_getInstance(controllerMap, "ControllerTestKey6");;
    controller->initializeController(controller, NULL, commandMap);

    if (controller == NULL) abort();

    // Test command registration when view is unavailable — should return false
    if (controller->registerCommand(controller, "ControllerTest2", test_controller_command_init) != false)
        abort();

    // remove the controller
    if (puremvc_controller_removeController("ControllerTestKey6", NULL) != true) abort();

    // trying removing again will return false
    if (puremvc_controller_removeController("ControllerTestKey6", NULL) != false) abort();;

    // view doesn't exist to begin with
    if (puremvc_view_removeView("ControllerTestKey6", NULL) != false) abort();;
}

void testCommandMapShiftLeft() {
    struct ViewMap **viewMap = (struct ViewMap *[]) {
        &(struct ViewMap){ .view = alloca(puremvc_view_size()) },
        NULL
    };

    struct ObserverMap **observerMap = (struct ObserverMap *[]) {
        &(struct ObserverMap){ .observers = (struct IObserver *[]){ memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()), NULL } },
        &(struct ObserverMap){ .observers = (struct IObserver *[]){ memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()), NULL } },
        &(struct ObserverMap){ .observers = (struct IObserver *[]){ memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()), NULL } },
        &(struct ObserverMap){ .observers = (struct IObserver *[]){ memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()), NULL } },
        NULL
    };

    struct IView *view = puremvc_view_getInstance(viewMap, "ControllerTestKey8");
    view->initializeView(view, observerMap, NULL);

    struct ControllerMap **controllerMap = (struct ControllerMap *[]) {
        &(struct ControllerMap){ .controller = alloca(puremvc_controller_size()) },
        NULL
    };

    struct CommandMap **commandMap = (struct CommandMap *[]){
        &(struct CommandMap){},
        &(struct CommandMap){},
        &(struct CommandMap){},
        &(struct CommandMap){},
        NULL
    };

    struct IController *controller = puremvc_controller_getInstance(controllerMap, "ControllerTestKey8");
    controller->initializeController(controller, view, commandMap);

    size_t offset1 = sizeof(struct IView) + sizeof(const char *);
    struct ObserverMap ***ppp1 = (struct ObserverMap ***)((char *) view + offset1);
    struct ObserverMap **obsMap = *ppp1;

    size_t offset2 = sizeof(struct IController) + sizeof(const char *);
    struct CommandMap ***ppp2 = (struct CommandMap ***)((char *) controller + offset2);
    struct CommandMap **cmdMap = *ppp2;

    struct INotification *notification0 = puremvc_notification_init(alloca(puremvc_notification_size()), "command0", NULL, NULL);
    controller->executeCommand(controller, notification0); // crash test
    controller->executeCommand(controller, notification0);

    // Register four commands and verify that each is correctly associated to their dictionaries and observers
    if (controller->registerCommand(controller, "command0", puremvc_simple_command_init) != true) abort();;
    if (strcmp(cmdMap[0]->key, "command0") != 0) abort();
    if(cmdMap[0]->factory != puremvc_simple_command_init) abort();
    if (strcmp(obsMap[0]->key, "command0") != 0) abort();
    if (obsMap[0]->observers[0]->getContext(obsMap[0]->observers[0]) != controller) abort();

    if (controller->registerCommand(controller, "command1", puremvc_simple_command_init) != true) abort();
    if (strcmp(cmdMap[1]->key, "command1") != 0) abort();
    if(cmdMap[1]->factory != puremvc_simple_command_init) abort();
    if (strcmp(obsMap[1]->key, "command1") != 0) abort();
    if (obsMap[1]->observers[0]->getContext(obsMap[1]->observers[0]) != controller) abort();

    if (controller->registerCommand(controller, "command2", puremvc_simple_command_init) != true)

        abort();
    if (strcmp(cmdMap[2]->key, "command2") != 0) abort();
    if(cmdMap[2]->factory != puremvc_simple_command_init) abort();
    if (strcmp(obsMap[2]->key, "command2") != 0) abort();
    if (obsMap[2]->observers[0]->getContext(obsMap[2]->observers[0]) != controller) abort();

    if (controller->registerCommand(controller, "command3", puremvc_simple_command_init) != true) abort();
    if (strcmp(cmdMap[3]->key, "command3") != 0) abort();
    if(cmdMap[3]->factory != puremvc_simple_command_init) abort();
    if (strcmp(obsMap[3]->key, "command3") != 0) abort();
    if (obsMap[3]->observers[0]->getContext(obsMap[3]->observers[0]) != controller) abort();

    // Remove the second command1 (middle) and verify that remaining commands 2, 3 and observers shifted correctly
    if (controller->removeCommand(controller, "command1", NULL) != true) abort();
    if (strcmp(cmdMap[0]->key, "command0") != 0) abort();
    if(cmdMap[0]->factory != puremvc_simple_command_init) abort();
    if (strcmp(obsMap[1]->key, "command2") != 0) abort();
    if (obsMap[1]->observers[0]->getContext(obsMap[1]->observers[0]) != controller) abort();
    if (strcmp(obsMap[2]->key, "command3") != 0) abort();
    if (obsMap[2]->observers[0]->getContext(obsMap[2]->observers[0]) != controller) abort();

    // // Remove the last command3 and verify the remaining command 0, 2 stay in place
    if (controller->removeCommand(controller, "command3", NULL) != true) abort();
    if (strcmp(cmdMap[0]->key, "command0") != 0) abort();
    if(cmdMap[0]->factory != puremvc_simple_command_init) abort();
    if (strcmp(obsMap[1]->key, "command2") != 0) abort();
    if (obsMap[1]->observers[0]->getContext(obsMap[1]->observers[0]) != controller) abort();

    // Remove the first command0 and verify that subsequent command2 shift left
    if (controller->removeCommand(controller, "command0", NULL) != true) abort();
    if (strcmp(cmdMap[0]->key, "command2") != 0) abort();
    if(cmdMap[0]->factory != puremvc_simple_command_init) abort();

    // Remove all remaining mediators and confirm that the dictionary key is cleared
    if (controller->removeCommand(controller, "command2", NULL) != true) abort();
    if (cmdMap[0]->key[0] != '\0') abort();

    if (puremvc_controller_removeController("ControllerTestKey8", NULL) != true) abort();
    if (puremvc_view_removeView("ControllerTestKey8", NULL) != true) abort();
}

void TestControllerMapShiftLeft() {
    struct ViewMap **viewMap = (struct ViewMap *[]) {
        &(struct ViewMap){ .view = alloca(puremvc_view_size() ) },
        &(struct ViewMap){ .view = alloca(puremvc_view_size() ) },
        &(struct ViewMap){ .view = alloca(puremvc_view_size() ) },
        &(struct ViewMap){ .view = alloca(puremvc_view_size() ) },
        NULL
    };

    if (puremvc_view_getInstance(viewMap, "controller0") == NULL) abort(); // Controller dependencies
    if (puremvc_view_getInstance(viewMap, "controller1") == NULL) abort();
    if (puremvc_view_getInstance(viewMap, "controller2") == NULL) abort();
    if (puremvc_view_getInstance(viewMap, "controller3") == NULL) abort();

    struct ControllerMap **instanceMap = (struct ControllerMap *[]) {
        &(struct ControllerMap){ .controller = alloca(puremvc_controller_size() ) },
        &(struct ControllerMap){ .controller = alloca(puremvc_controller_size() ) },
        &(struct ControllerMap){ .controller = alloca(puremvc_controller_size() ) },
        &(struct ControllerMap){ .controller = alloca(puremvc_controller_size() ) },
        NULL
    };

    // create 4 instances
    if (puremvc_controller_getInstance(instanceMap, "controller0") == NULL) abort();
    if (strcmp(instanceMap[0]->key, "controller0") != 0) abort();
    const char **key0 = (const char **)((char *) instanceMap[0]->controller + sizeof(struct IController));
    if (strcmp(*key0, "controller0") != 0) abort();

    if (puremvc_controller_getInstance(instanceMap, "controller1") == NULL) abort();
    if (strcmp(instanceMap[1]->key, "controller1") != 0) abort();
    const char **key1 = (const char **)((char *) instanceMap[1]->controller + sizeof(struct IController));
    if (strcmp(*key1, "controller1") != 0) abort();

    if (puremvc_controller_getInstance(instanceMap, "controller2") == NULL) abort();
    const char **key2 = (const char **)((char *) instanceMap[2]->controller + sizeof(struct IController));
    if (strcmp(*key2, "controller2") != 0) abort();

    if (puremvc_controller_getInstance(instanceMap, "controller3") == NULL) abort();
    const char **key3 = (const char **)((char *) instanceMap[3]->controller + sizeof(struct IController));
    if (strcmp(*key3, "controller3") != 0) abort();

    // remove
    struct IController *controller1 = NULL; // remove middle controller1, remaining 0, 2, 3
    if (puremvc_controller_removeController("controller1", &controller1) != true) abort();
    if (strcmp(instanceMap[0]->key, "controller0") != 0) abort();
    if (strcmp(instanceMap[1]->key, "controller2") != 0) abort();
    if (strcmp(instanceMap[2]->key, "controller3") != 0) abort();
    if (instanceMap[3]->key[0] != '\0') abort();
    if (instanceMap[4] != NULL) abort();

    struct IController *controller3 = NULL; // remove last, remaining 0, 2
    if (puremvc_controller_removeController("controller3", &controller3) != true) abort();
    if (strcmp(instanceMap[0]->key, "controller0") != 0) abort();
    if (strcmp(instanceMap[1]->key, "controller2") != 0) abort();
    if (instanceMap[2]->key[0] != '\0') abort();
    if (instanceMap[3]->key[0] != '\0') abort();
    if (instanceMap[4] != NULL) abort();

    struct IController *controller0 = NULL; // remove first, remaining 2
    if (puremvc_controller_removeController("controller0", &controller0) != true) abort();
    if (strcmp(instanceMap[0]->key, "controller2") != 0) abort();
    if (instanceMap[1]->key[0] != '\0') abort();
    if (instanceMap[2]->key[0] != '\0') abort();
    if (instanceMap[3]->key[0] != '\0') abort();
    if (instanceMap[4] != NULL) abort();

    struct IController *controller2 = NULL; // remove remaining
    if (puremvc_controller_removeController("controller2", &controller2) != true) abort();
    if (instanceMap[0]->key[0] != '\0') abort();
    if (instanceMap[1]->key[0] != '\0') abort();
    if (instanceMap[2]->key[0] != '\0') abort();
    if (instanceMap[3]->key[0] != '\0') abort();
    if (instanceMap[4] != NULL) abort();
}

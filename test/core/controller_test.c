#include "controller_test.h"
#include "controller_test_command.h"
#include "controller_test_command2.h"
#include "controller_test_vo.h"

#include "puremvc/i_view.h"

#include <alloca.h>
#include <assert.h>
#include <string.h>

int main() {
    testGetInstance();
    testRegisterAndExecuteCommand();
    testRegisterAndRemoveCommand();
    testHasCommand();
    testReregisterAndExecuteCommand();
    testRegisterAndUpdateCommand();
    testRemoveController();
    testCommandMapShiftLeft();
    TestControllerMapShiftLeft();
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
    assert(controller != NULL);
    assert(controller == puremvc_controller_getInstance(NULL, "ControllerTestKey1"));

    struct IController *removedController = NULL;
    assert(puremvc_controller_removeController("ControllerTestKey1", &removedController) == true);

    struct IView *removedView = NULL;
    assert(puremvc_view_removeView("ControllerTestKey1", &removedView) == true);
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
    assert(controller != NULL);

    controller->initializeController(controller, view, commandMap);

    assert(controller->registerCommand(controller, "ControllerTest1", test_controller_command_init) == true);

    // Create a 'ControllerTest' note
    struct ControllerTestVO vo = {12, 0};
    struct INotification *notification = puremvc_notification_init(alloca(puremvc_notification_size()), "ControllerTest1", &vo, NULL);

    // Tell the controller to execute the Command associated with the note
    // the ControllerTestCommand invoked will multiply the vo.input value
    // by 2 and set the result on vo.result
    controller->executeCommand(controller, notification);

    // test assertions
    assert(vo.result == 24);

    struct ICommand *(*factory)(void *) = 0;
    assert(controller->removeCommand(controller, "ControllerTest1", &factory) == true);;
    assert(puremvc_controller_removeController("ControllerTestKey2", NULL) == true);;
    assert(puremvc_view_removeView("ControllerTestKey2", NULL) == true);;
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

    assert(controller->registerCommand(controller, "ControllerRemoveTest", test_controller_command_init) == true);;

    // Create a 'ControllerTest' note
    struct ControllerTestVO vo = {12, 0};
    struct INotification *notification = puremvc_notification_init(alloca(puremvc_notification_size()), "ControllerRemoveTest", &vo, NULL);

    // Tell the controller to execute the Command associated with the note
    // the ControllerTestCommand invoked will multiply the vo.input value
    // by 2 and set the result on vo.result
    controller->executeCommand(controller, notification);

    // test assertions
    assert(vo.result == 24);

    // Reset result
    vo.result = 0;

    // Remove the Command from the Controller
    assert(controller->removeCommand(controller, "ControllerRemoveTest", NULL) == true);;

    // Tell the controller to execute the Command associated with the
    // note. This time, it should not be registered, and our vo result
    // will not change
    controller->executeCommand(controller, notification);

    // test assertions
    assert(vo.result == 0);
    assert(puremvc_controller_removeController("ControllerTestKey3", NULL) == true);;
    assert(puremvc_view_removeView("ControllerTestKey3", NULL) == true);
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
    assert(controller->registerCommand(controller, "hasCommandTest", test_controller_command_init) == true);;
    assert(controller->hasCommand(controller, "hasCommandTest") == true);

    // Remove the Command from the Controller
    assert(controller->removeCommand(controller, "hasCommandTest", NULL) == true);;

    // test that hasCommand returns false for hasCommandTest notifications
    assert(controller->hasCommand(controller, "hasCommandTest") == false);

    assert(puremvc_controller_removeController("ControllerTestKey4", NULL) == true);;
    assert(puremvc_view_removeView("ControllerTestKey4", NULL) == true);;
}

void testReregisterAndExecuteCommand() {
    struct ViewMap **viewMap = (struct ViewMap *[]) {
        &(struct ViewMap){ .view = alloca(puremvc_view_size()) },
        NULL
    };

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

    struct CommandMap **commandMap = (struct CommandMap *[]) {
        &(struct CommandMap){},
        NULL
    };

    // Fetch the controller, register the ControllerTestCommand to handle 'ControllerTest2' notes
    struct IController *controller = puremvc_controller_getInstance(controllerMap, "ControllerTestKey5");
    controller->initializeController(controller, view, commandMap);

    assert(controller->registerCommand(controller, "ControllerTest2", test_controller_command2_init) == true);

    // Remove the Command from the Controller
    assert(controller->removeCommand(controller, "ControllerTest2", NULL) == true);

    // Re-register the Command with the Controller
    assert(controller->registerCommand(controller, "ControllerTest2", test_controller_command2_init) == true);

    // Create a 'ControllerTest2' note
    struct ControllerTestVO vo = {12, 0};
    const struct INotification *notification = puremvc_notification_init(alloca(puremvc_notification_size()), "ControllerTest2", &vo, NULL);

    // retrieve a reference to the View from the same core.
    const struct IView *view2 = puremvc_view_getInstance(NULL, "ControllerTestKey5");
    view2->notifyObservers(view2, notification);

    // test assertions
    // if the command is executed once the value will be 24
    assert(vo.result == 24);

    // Prove that accumulation works in the VO by sending the notification again
    view->notifyObservers(view, notification);

    // if the command is executed twice the value will be 48
    assert(vo.result == 48);

    assert(controller->removeCommand(controller, "ControllerTest2", NULL) == true);
    assert(puremvc_controller_removeController("ControllerTestKey5", NULL) == true);
    assert(puremvc_view_removeView("ControllerTestKey5", NULL) == true);;
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
    assert(controller->registerCommand(controller, "ControllerTest2", test_controller_command_init) == true);

    // update command (a warning will be generated as it overrides)
    assert(controller->registerCommand(controller, "ControllerTest2", test_controller_command2_init) == true);

    struct ControllerTestVO vo = {12, 10};
    struct INotification *notification = puremvc_notification_init(alloca(puremvc_notification_size()), "ControllerTest2", &vo, NULL);
    controller->executeCommand(controller, notification);

    // second command result
    assert(vo.result == 34);

    assert(controller->removeCommand(controller, "ControllerTest2", NULL) == true);;
    assert(puremvc_controller_removeController("ControllerTestKey6", NULL) == true);;
    assert(puremvc_view_removeView("ControllerTestKey6", NULL) == true);;
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

    assert(controller != NULL);

    // Test command registration when view is unavailable — should return false
    assert(controller->registerCommand(controller, "ControllerTest2", test_controller_command_init) == false);

    // remove the controller
    assert(puremvc_controller_removeController("ControllerTestKey6", NULL) == true);

    // trying removing again will return false
    assert(puremvc_controller_removeController("ControllerTestKey6", NULL) == false);;

    // view doesn't exist to begin with
    assert(puremvc_view_removeView("ControllerTestKey6", NULL) == false);;
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
    assert(controller->registerCommand(controller, "command0", puremvc_simple_command_init) == true);;
    assert(strcmp(cmdMap[0]->key, "command0") == 0);
    assert(cmdMap[0]->factory == puremvc_simple_command_init);
    assert(strcmp(obsMap[0]->key, "command0") == 0);
    assert(obsMap[0]->observers[0]->getContext(obsMap[0]->observers[0]) == controller);

    assert(controller->registerCommand(controller, "command1", puremvc_simple_command_init) == true);
    assert(strcmp(cmdMap[1]->key, "command1") == 0);
    assert(cmdMap[1]->factory == puremvc_simple_command_init);
    assert(strcmp(obsMap[1]->key, "command1") == 0);
    assert(obsMap[1]->observers[0]->getContext(obsMap[1]->observers[0]) == controller);

    assert(controller->registerCommand(controller, "command2", puremvc_simple_command_init) == true);
    assert(strcmp(cmdMap[2]->key, "command2") == 0);
    assert(cmdMap[2]->factory == puremvc_simple_command_init);
    assert(strcmp(obsMap[2]->key, "command2") == 0);
    assert(obsMap[2]->observers[0]->getContext(obsMap[2]->observers[0]) == controller);

    assert(controller->registerCommand(controller, "command3", puremvc_simple_command_init) == true);
    assert(strcmp(cmdMap[3]->key, "command3") == 0);
    assert(cmdMap[3]->factory == puremvc_simple_command_init);
    assert(strcmp(obsMap[3]->key, "command3") == 0);
    assert(obsMap[3]->observers[0]->getContext(obsMap[3]->observers[0]) == controller);

    // Remove the second command1 (middle) and verify that remaining commands 2, 3 and observers shifted correctly
    assert(controller->removeCommand(controller, "command1", NULL) == true);
    assert(strcmp(cmdMap[0]->key, "command0") == 0);
    assert(cmdMap[0]->factory == puremvc_simple_command_init);
    assert(strcmp(obsMap[1]->key, "command2") == 0);
    assert(obsMap[1]->observers[0]->getContext(obsMap[1]->observers[0]) == controller);
    assert(strcmp(obsMap[2]->key, "command3") == 0);
    assert(obsMap[2]->observers[0]->getContext(obsMap[2]->observers[0]) == controller);

    // // Remove the last command3 and verify the remaining command 0, 2 stay in place
    assert(controller->removeCommand(controller, "command3", NULL) == true);
    assert(strcmp(cmdMap[0]->key, "command0") == 0);
    assert(cmdMap[0]->factory == puremvc_simple_command_init);
    assert(strcmp(obsMap[1]->key, "command2") == 0);
    assert(obsMap[1]->observers[0]->getContext(obsMap[1]->observers[0]) == controller);

    // Remove the first command0 and verify that subsequent command2 shift left
    assert(controller->removeCommand(controller, "command0", NULL) == true);
    assert(strcmp(cmdMap[0]->key, "command2") == 0);
    assert(cmdMap[0]->factory == puremvc_simple_command_init);

    // Remove all remaining mediators and confirm that the dictionary key is cleared
    assert(controller->removeCommand(controller, "command2", NULL) == true);
    assert(cmdMap[0]->key == NULL);

    assert(puremvc_controller_removeController("ControllerTestKey8", NULL) == true);
    assert(puremvc_view_removeView("ControllerTestKey8", NULL) == true);
}

void TestControllerMapShiftLeft() {
    struct ViewMap **viewMap = (struct ViewMap *[]) {
        &(struct ViewMap){ .view = alloca(puremvc_view_size() ) },
        &(struct ViewMap){ .view = alloca(puremvc_view_size() ) },
        &(struct ViewMap){ .view = alloca(puremvc_view_size() ) },
        &(struct ViewMap){ .view = alloca(puremvc_view_size() ) },
        NULL
    };

    assert(puremvc_view_getInstance(viewMap, "controller0") != NULL); // Controller dependencies
    assert(puremvc_view_getInstance(viewMap, "controller1") != NULL);
    assert(puremvc_view_getInstance(viewMap, "controller2") != NULL);
    assert(puremvc_view_getInstance(viewMap, "controller3") != NULL);

    struct ControllerMap **instanceMap = (struct ControllerMap *[]) {
        &(struct ControllerMap){ .controller = alloca(puremvc_controller_size() ) },
        &(struct ControllerMap){ .controller = alloca(puremvc_controller_size() ) },
        &(struct ControllerMap){ .controller = alloca(puremvc_controller_size() ) },
        &(struct ControllerMap){ .controller = alloca(puremvc_controller_size() ) },
        NULL
    };

    // create 4 instances
    assert(puremvc_controller_getInstance(instanceMap, "controller0") != NULL);
    assert(strcmp(instanceMap[0]->key, "controller0") == 0);
    const char **key0 = (const char **)((char *) instanceMap[0]->controller + sizeof(struct IController));
    assert(strcmp(*key0, "controller0") == 0);

    assert(puremvc_controller_getInstance(instanceMap, "controller1") != NULL);
    assert(strcmp(instanceMap[1]->key, "controller1") == 0);
    const char **key1 = (const char **)((char *) instanceMap[1]->controller + sizeof(struct IController));
    assert(strcmp(*key1, "controller1") == 0);

    assert(puremvc_controller_getInstance(instanceMap, "controller2") != NULL);
    const char **key2 = (const char **)((char *) instanceMap[2]->controller + sizeof(struct IController));
    assert(strcmp(*key2, "controller2") == 0);

    assert(puremvc_controller_getInstance(instanceMap, "controller3") != NULL);
    const char **key3 = (const char **)((char *) instanceMap[3]->controller + sizeof(struct IController));
    assert(strcmp(*key3, "controller3") == 0);

    // remove
    struct IController *controller1 = NULL; // remove middle controller1, remaining 0, 2, 3
    assert(puremvc_controller_removeController("controller1", &controller1) == true);
    assert(strcmp(instanceMap[0]->key, "controller0") == 0);
    assert(strcmp(instanceMap[1]->key, "controller2") == 0);
    assert(strcmp(instanceMap[2]->key, "controller3") == 0);
    assert(instanceMap[3]->key == NULL);
    assert(instanceMap[4] == NULL);

    struct IController *controller3 = NULL; // remove last, remaining 0, 2
    assert(puremvc_controller_removeController("controller3", &controller3) == true);
    assert(strcmp(instanceMap[0]->key, "controller0") == 0);
    assert(strcmp(instanceMap[1]->key, "controller2") == 0);
    assert(instanceMap[2]->key == NULL);
    assert(instanceMap[3]->key == NULL);
    assert(instanceMap[4] == NULL);

    struct IController *controller0 = NULL; // remove first, remaining 2
    assert(puremvc_controller_removeController("controller0", &controller0) == true);
    assert(strcmp(instanceMap[0]->key, "controller2") == 0);
    assert(instanceMap[1]->key == NULL);
    assert(instanceMap[2]->key == NULL);
    assert(instanceMap[3]->key == NULL);
    assert(instanceMap[4] == NULL);

    struct IController *controller2 = NULL; // remove remaining
    assert(puremvc_controller_removeController("controller2", &controller2) == true);
    assert(instanceMap[0]->key == NULL);
    assert(instanceMap[1]->key == NULL);
    assert(instanceMap[2]->key == NULL);
    assert(instanceMap[3]->key == NULL);
    assert(instanceMap[4] == NULL);
}

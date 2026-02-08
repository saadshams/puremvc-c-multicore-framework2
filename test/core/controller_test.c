#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "puremvc/controller.h"
#include "puremvc/view.h"

#include "controller_test.h"
#include "controller_test_command.h"
#include "controller_test_command2.h"
#include "controller_test_vo.h"

int main() {
    testGetInstance();
    testRegisterAndExecuteCommand();
    testRegisterAndRemoveCommand();
    testHasCommand();
    testReregisterAndExecuteCommand();
    testRegisterAndUpdateCommand();
    testRemoveController();
    testRegisterAndRemoveMultipleCommands();
    TestControllerShiftLeft();
    return 0;
}

void testGetInstance() {
    struct ViewMap *viewMap[] = { &(struct ViewMap) {
        .view = (struct IView *) &(struct View) { .multitonKey = "", .observerMap = {}, .mediatorMap = {} },
    }, NULL };

    // Test Factory Method
   assert(puremvc_view_getInstance(viewMap, "ControllerTestKey1") != NULL); // pre-initialize View for the Controller

    struct ControllerMap *controllerMap[] = { &(struct ControllerMap) {
        .controller = (struct IController *) &(struct Controller) { .commandMap = (struct CommandMap *[]){} },
    }, NULL };

    // Test Factory Method
    const struct IController *controller = puremvc_controller_getInstance(controllerMap, "ControllerTestKey1");

    // test assertions
    assert(controller != NULL);
    assert(controller == puremvc_controller_getInstance(controllerMap, "ControllerTestKey1"));

    struct IController *removedController = NULL;
    assert(puremvc_controller_removeController("ControllerTestKey1", &removedController) == true);;
    assert(strcmp(((struct Controller *) removedController)->multitonKey, "ControllerTestKey1") == 0);

    struct IView *removedView = NULL;
    assert(puremvc_view_removeView("ControllerTestKey1", &removedView) == true);
    assert(strcmp(((struct View *) removedView)->multitonKey, "ControllerTestKey1") == 0);
}

void testRegisterAndExecuteCommand() {
    struct ViewMap *viewMap[] = { &(struct ViewMap) {
        .view = (struct IView *) &(struct View) {
            .observerMap = (struct ObserverMap *[]) { &(struct ObserverMap) { // key = ControllerTest1, observer = command
                .observers = (struct IObserver *[]) { (struct IObserver *) &(struct Observer){0}, NULL }
            }, NULL }
        },
    }, NULL };

    puremvc_view_getInstance(viewMap, "ControllerTestKey2");

    struct ControllerMap *controllerMap[] = { &(struct ControllerMap) {
        .controller = (struct IController *) &(struct Controller) {
            .commandMap = (struct CommandMap *[]){ &(struct CommandMap){}, NULL }
        },
    }, NULL };

    // Create the controller, register the ControllerTestCommand to handle 'ControllerTest' notes
    struct IController *controller = puremvc_controller_getInstance(controllerMap, "ControllerTestKey2");
    assert(controller->registerCommand(controller, "ControllerTest1", test_controller_command_init) == true);

    // Create a 'ControllerTest' note
    struct ControllerTestVO vo = {12, 0};
    struct INotification *notification = puremvc_notification_init((struct INotification *) &(struct Notification){0}, "ControllerTest1", &vo, NULL);

    // Tell the controller to execute the Command associated with the note
    // the ControllerTestCommand invoked will multiply the vo.input value
    // by 2 and set the result on vo.result
    controller->executeCommand(controller, notification);

    // test assertions
    assert(vo.result == 24);

    struct ICommand *(**factory)() = 0;
    assert(controller->removeCommand(controller, "ControllerTest1", factory) == true);;
    assert(puremvc_controller_removeController("ControllerTestKey2", NULL) == true);;
    assert(puremvc_view_removeView("ControllerTestKey2", NULL) == true);;
}

void testRegisterAndRemoveCommand() {
   struct ViewMap *viewMap[] = { &(struct ViewMap) {
       .view = (struct IView *) &(struct View) {
           .observerMap = (struct ObserverMap *[]) { &(struct ObserverMap){
                .observers = (struct IObserver *[]) { (struct IObserver *) &(struct Observer){}, NULL }
           }, NULL }
       }
   }, NULL };

    // Initialize View before Controller (same multiton key)
    puremvc_view_getInstance(viewMap, "ControllerTestKey3"); // dependency for the controller

    struct ControllerMap *controllerMap[] = { &(struct ControllerMap) {
        .controller = (struct IController *) &(struct Controller) {
            .commandMap = (struct CommandMap *[]){ &(struct CommandMap){.key = ""}, NULL }
        },
    }, NULL };

    // Create the controller, register the ControllerTestCommand to handle 'ControllerTest' notes
    struct IController *controller = puremvc_controller_getInstance(controllerMap, "ControllerTestKey3");
    assert(controller->registerCommand(controller, "ControllerRemoveTest", test_controller_command_init) == true);;

    // Create a 'ControllerTest' note
    struct ControllerTestVO vo = {12, 0};
    struct INotification *notification = puremvc_notification_init((struct INotification *) &(struct Notification){0}, "ControllerRemoveTest", &vo, NULL);

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
    struct ViewMap *viewMap[] = { &(struct ViewMap) {
        .view = (struct IView *) &(struct View){
            .observerMap = (struct ObserverMap *[]){ &(struct ObserverMap){
                .observers = (struct IObserver *[]) { (struct IObserver *) &(struct Observer){}, NULL }
            }, NULL }
        }
    }, NULL };

    puremvc_view_getInstance(viewMap, "ControllerTestKey4"); // dependency for the Controller

    struct ControllerMap *controllerMap[] = { &(struct ControllerMap) {
        .controller = (struct IController *) &(struct Controller) {
            .commandMap = (struct CommandMap *[]){ &(struct CommandMap){}, NULL }
        },
    }, NULL };

    // register the ControllerTestCommand to handle 'hasCommandTest' notes
    struct IController *controller = puremvc_controller_getInstance(controllerMap, "ControllerTestKey4");

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
    struct ViewMap *viewMap[] = { &(struct ViewMap) {
        .view = (struct IView *) &(struct View) {
            .observerMap = (struct ObserverMap *[]) { &(struct ObserverMap){
                .observers = (struct IObserver *[]) { (struct IObserver *) &(struct Observer){}, NULL }
            }, NULL },
        },
    }, NULL };

    // Test Factory Method
    assert(puremvc_view_getInstance(viewMap, "ControllerTestKey5") != NULL);

    struct ControllerMap *controllerMap[] = { &(struct ControllerMap) {
        .controller = (struct IController *) &(struct Controller) {
            .commandMap = (struct CommandMap *[]){ &(struct CommandMap){}, NULL }
        },
    }, NULL };

    // Fetch the controller, register the ControllerTestCommand to handle 'ControllerTest2' notes
    struct IController *controller = puremvc_controller_getInstance(controllerMap, "ControllerTestKey5");

    assert(controller->registerCommand(controller, "ControllerTest2", test_controller_command2_init) == true);

    // Remove the Command from the Controller
    assert(controller->removeCommand(controller, "ControllerTest2", NULL) == true);

    // Re-register the Command with the Controller
    assert(controller->registerCommand(controller, "ControllerTest2", test_controller_command2_init) == true);

    // Create a 'ControllerTest2' note
    struct ControllerTestVO vo = {12, 0};
    const struct INotification *notification = puremvc_notification_init((struct INotification *) &(struct Notification){0}, "ControllerTest2", &vo, NULL);

    // retrieve a reference to the View from the same core.
    const struct IView *view = puremvc_view_getInstance(NULL, "ControllerTestKey5");
    view->notifyObservers(view, notification);

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
   struct ViewMap *viewMap[] = { &(struct ViewMap){
        .view = (struct IView *) &(struct View) {
            .observerMap = (struct ObserverMap *[]) { &(struct ObserverMap){
                .observers = (struct IObserver *[]) { (struct IObserver *) &(struct Observer){}, NULL }
            }, NULL }
        }
   }, NULL };

    puremvc_view_getInstance(viewMap, "ControllerTestKey6");

    struct ControllerMap *controllerMap[] = { &(struct ControllerMap) {
        .controller = (struct IController *) &(struct Controller) {
            .commandMap = (struct CommandMap *[]){ &(struct CommandMap){}, NULL }
        },
    }, NULL };

    struct IController *controller = puremvc_controller_getInstance(controllerMap, "ControllerTestKey6");

    // first registration
    assert(controller->registerCommand(controller, "ControllerTest2", test_controller_command_init) == true);

    // update command (a warning will be generated as it overrides)
    assert(controller->registerCommand(controller, "ControllerTest2", test_controller_command2_init) == true);

    struct ControllerTestVO vo = {12, 10};
    struct INotification *notification = puremvc_notification_init((struct INotification *) &(struct Notification){0}, "ControllerTest2", &vo, NULL);
    controller->executeCommand(controller, notification);

    // second command result
    assert(vo.result == 34);

    assert(controller->removeCommand(controller, "ControllerTest2", NULL) == true);;
    assert(puremvc_controller_removeController("ControllerTestKey6", NULL) == true);;
    assert(puremvc_view_removeView("ControllerTestKey6", NULL) == true);;
}

void testRemoveController() {
    // NOTE: Successful execution will produce several expected error logs
    struct ControllerMap *controllerMap[] = { &(struct ControllerMap) {
        .controller = (struct IController *) &(struct Controller) {
            .commandMap = (struct CommandMap *[]){ &(struct CommandMap){}, NULL }
        },
    }, NULL };

    // Get a Multiton Controller instance
    struct IController *controller = puremvc_controller_getInstance(controllerMap, "ControllerTestKey6");;

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

void testRegisterAndRemoveMultipleCommands() {
    struct ViewMap *viewMap[] = { &(struct ViewMap){
        .view = (struct IView *) &(struct View) {
            .observerMap = (struct ObserverMap *[]) { // Observer Map of 4 Notification types with its own list of Observers
                &(struct ObserverMap) { .observers = (struct IObserver *[]) { (struct IObserver *) &(struct Observer){}, NULL } },
                &(struct ObserverMap) { .observers = (struct IObserver *[]) { (struct IObserver *) &(struct Observer){}, NULL } },
                &(struct ObserverMap) { .observers = (struct IObserver *[]) { (struct IObserver *) &(struct Observer){}, NULL } },
                &(struct ObserverMap) { .observers = (struct IObserver *[]) { (struct IObserver *) &(struct Observer){}, NULL } },
                NULL
            }
        }
    }, NULL};

    assert(puremvc_view_getInstance(viewMap, "ControllerTestKey8") != NULL);

    struct ControllerMap *controllerMap[] = { &(struct ControllerMap) {
        .controller = (struct IController *) &(struct Controller) {
            .commandMap = (struct CommandMap *[]) { // ControllerMap for 4 Command slots
                &(struct CommandMap){}, &(struct CommandMap){}, &(struct CommandMap){}, &(struct CommandMap){},
                NULL
            }
        },
    }, NULL };

    struct IController *controller = puremvc_controller_getInstance(controllerMap, "ControllerTestKey8");
    struct Controller *concrete = (struct Controller *) controller;
    const struct View *view = (struct View *) concrete->view;

    struct INotification *notification0 = puremvc_notification_init((struct INotification *) &(struct Notification){0}, "command0", NULL, NULL);
    controller->executeCommand(controller, notification0); // crash test
    controller->executeCommand(controller, notification0);

    // Register four commands and verify that each is correctly associated to their dictionaries and observers
    assert(controller->registerCommand(controller, "command0", puremvc_simple_command_init) == true);;
    assert(strcmp(concrete->commandMap[0]->key, "command0") == 0);
    assert(concrete->commandMap[0]->factory == puremvc_simple_command_init);
    assert(strcmp(view->observerMap[0]->key, "command0") == 0);
    assert(view->observerMap[0]->observers[0]->getContext(view->observerMap[0]->observers[0]) == controller);

    assert(controller->registerCommand(controller, "command1", puremvc_simple_command_init) == true);
    assert(strcmp(concrete->commandMap[1]->key, "command1") == 0);
    assert(concrete->commandMap[1]->factory == puremvc_simple_command_init);
    assert(strcmp(view->observerMap[1]->key, "command1") == 0);
    assert(view->observerMap[1]->observers[0]->getContext(view->observerMap[1]->observers[0]) == controller);

    assert(controller->registerCommand(controller, "command2", puremvc_simple_command_init) == true);
    assert(strcmp(concrete->commandMap[2]->key, "command2") == 0);
    assert(concrete->commandMap[2]->factory == puremvc_simple_command_init);
    assert(strcmp(view->observerMap[2]->key, "command2") == 0);
    assert(view->observerMap[2]->observers[0]->getContext(view->observerMap[2]->observers[0]) == controller);

    assert(controller->registerCommand(controller, "command3", puremvc_simple_command_init) == true);
    assert(strcmp(concrete->commandMap[3]->key, "command3") == 0);
    assert(concrete->commandMap[3]->factory == puremvc_simple_command_init);
    assert(strcmp(view->observerMap[3]->key, "command3") == 0);
    assert(view->observerMap[3]->observers[0]->getContext(view->observerMap[3]->observers[0]) == controller);

    // Remove the second command (middle) and verify that remaining commands and observers 3, 4 are shifted correctly
    assert(controller->removeCommand(controller, "command1", NULL) == true);
    assert(strcmp(concrete->commandMap[0]->key, "command0") == 0);
    assert(concrete->commandMap[0]->factory == puremvc_simple_command_init);
    assert(strcmp(view->observerMap[1]->key, "command2") == 0);
    assert(view->observerMap[1]->observers[0]->getContext(view->observerMap[1]->observers[0]) == controller);
    assert(strcmp(view->observerMap[2]->key, "command3") == 0);
    assert(view->observerMap[2]->observers[0]->getContext(view->observerMap[2]->observers[0]) == controller);

    // Remove the last command and verify the remaining command 1, 3 stay in place
    assert(controller->removeCommand(controller, "command3", NULL) == true);
    assert(strcmp(concrete->commandMap[0]->key, "command0") == 0);
    assert(concrete->commandMap[0]->factory == puremvc_simple_command_init);
    assert(strcmp(view->observerMap[1]->key, "command2") == 0);
    assert(view->observerMap[1]->observers[0]->getContext(view->observerMap[1]->observers[0]) == controller);

    // Remove the first command and verify that subsequent command 3 shift left
    assert(controller->removeCommand(controller, "command0", NULL) == true);
    assert(strcmp(concrete->commandMap[0]->key, "command2") == 0);
    assert(concrete->commandMap[0]->factory == puremvc_simple_command_init);

    // Remove all remaining mediators and confirm that the dictionary key is cleared
    assert(controller->removeCommand(controller, "command2", NULL) == true);
    assert(concrete->commandMap[0]->key[0] == '\0');

    assert(puremvc_controller_removeController("ControllerTestKey8", NULL) == true);
    assert(puremvc_view_removeView("ControllerTestKey8", NULL) == true);
}

void TestControllerShiftLeft() {
    struct ViewMap *viewMap[] = {
        &(struct ViewMap) { .view = (struct IView *) &(struct View) {} },
        &(struct ViewMap) { .view = (struct IView *) &(struct View) {} },
        &(struct ViewMap) { .view = (struct IView *) &(struct View) {} },
        &(struct ViewMap) { .view = (struct IView *) &(struct View) {} },
        NULL
    };

    assert(puremvc_view_getInstance(viewMap, "controller1") != NULL); // Controller dependencies
    assert(puremvc_view_getInstance(viewMap, "controller2") != NULL);
    assert(puremvc_view_getInstance(viewMap, "controller3") != NULL);
    assert(puremvc_view_getInstance(viewMap, "controller4") != NULL);

    struct ControllerMap *controllerMap[] = {
        &(struct ControllerMap) { .controller = (struct IController *) &(struct Controller) {} },
        &(struct ControllerMap) { .controller = (struct IController *) &(struct Controller) {} },
        &(struct ControllerMap) { .controller = (struct IController *) &(struct Controller) {} },
        &(struct ControllerMap) { .controller = (struct IController *) &(struct Controller) {} },
        NULL
    };

    assert(puremvc_controller_getInstance(controllerMap, "controller1") != NULL);
    assert(puremvc_controller_getInstance(controllerMap, "controller2") != NULL);
    assert(puremvc_controller_getInstance(controllerMap, "controller3") != NULL);
    assert(puremvc_controller_getInstance(controllerMap, "controller4") != NULL);

    struct IController *controller2 = NULL;
    assert(puremvc_controller_removeController("controller2", &controller2) == true); // remove middle
    assert(strcmp(((struct Controller *) controller2)->multitonKey, "controller2") == 0);

    struct IController *controller4 = NULL;
    assert(puremvc_controller_removeController("controller4", &controller4) == true); // remove last
    assert(strcmp(((struct Controller *) controller4)->multitonKey, "controller4") == 0);

    struct IController *controller1 = NULL;
    assert(puremvc_controller_removeController("controller1", &controller1) == true); // remove first
    assert(strcmp(((struct Controller *) controller1)->multitonKey, "controller1") == 0);

    struct IController *controller3 = NULL;
    assert(puremvc_controller_removeController("controller3", &controller3) == true); // remove remaining
    assert(strcmp(((struct Controller *) controller3)->multitonKey, "controller3") == 0);

    struct IView *view2 = NULL;
    assert(puremvc_view_removeView("controller2", &view2) == true); // remove middle
    assert(strcmp(((struct View *) view2)->multitonKey, "controller2") == 0);

    struct IView *view4 = NULL;
    assert(puremvc_view_removeView("controller4", &view4) == true); // remove last
    assert(strcmp(((struct View *) view4)->multitonKey, "controller4") == 0);

    struct IView *view1 = NULL;
    assert(puremvc_view_removeView("controller1", &view1) == true); // remove first
    assert(strcmp(((struct View *) view1)->multitonKey, "controller1") == 0);

    struct IView *view3 = NULL;
    assert(puremvc_view_removeView("controller3", &view3) == true); // remove remaining
    assert(strcmp(((struct View *) view3)->multitonKey, "controller3") == 0);
}

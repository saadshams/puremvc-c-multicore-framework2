#include "puremvc/macro_command.h"
#include "puremvc/controller.h"
#include "puremvc/notification.h"
#include "puremvc/view.h"
#include "puremvc/observer.h"

#include "macro_command_test.h"
#include "macro_command_test_command.h"
#include "macro_command_test_vo.h"

#include <assert.h>

int main() {
    testMacroCommandExecute();
    testRegisterAndExecuteCommand();
    return 0;
}

void testMacroCommandExecute() {
    struct MacroCommandTestVO vo = { .input = 5, 0, 0, 0};

    struct INotification *notification = puremvc_notification_init((struct INotification *) &(struct Notification){0}, "MacroCommandTest", &vo, NULL);

    const struct ICommand *command = macro_command_test_command((struct ICommand *) &(struct SimpleCommand){0});

    command->getNotifier(command)->initializeNotifier(command->getNotifier(command), "MacroCommandTestkey1");

    command->execute(command, notification);

    assert(vo.result1 == 10);
    assert(vo.result2 == 25);
    assert(vo.result3 == 125);
}

void testRegisterAndExecuteCommand() {
    struct ViewMap *viewMap[] = { &(struct ViewMap){ .view = (struct IView *) &(struct View){
        .observerMap = (struct ObserverMap *[]) { &(struct ObserverMap){
            .observers = (struct IObserver *[]) { (struct IObserver *) &(struct Observer){0}, NULL }
        }, NULL }
    } }, NULL};
    assert(puremvc_view_getInstance(viewMap, "ControllerTestKey1") != NULL); // pre-init view for the controller

    struct ControllerMap *controllerMap[] = { &(struct ControllerMap) { .controller = (struct IController *) &(struct Controller){
        .commandMap = (struct CommandMap *[]) { &(struct CommandMap){}, NULL }
    }}, NULL };
    struct IController *controller = puremvc_controller_getInstance(controllerMap, "ControllerTestKey1");

    assert(controller->registerCommand(controller, "MacroCommandTest", macro_command_test_command) == true);;

    const struct IView *view = puremvc_view_getInstance(NULL, "ControllerTestKey1"); // can pass NULL to retrieve an existing instance

    struct MacroCommandTestVO vo = {.input = 5, 0, 0, 0};
    const struct INotification *notification = puremvc_notification_init((struct INotification *) &(struct Notification){}, "MacroCommandTest", &vo, NULL);

    view->notifyObservers(view, notification);

    // test assertions
    assert(vo.result1 == 10);
    assert(vo.result2 == 25);
    assert(vo.result3 == 125);

    assert(controller->removeCommand(controller, "MacroCommandTest", NULL) == true);
    assert(puremvc_controller_removeController("ControllerTestKey1") == true);;
    assert(puremvc_view_removeView("ControllerTestKey1") == true);
}

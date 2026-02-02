#include <assert.h>

#include "puremvc/macro_command.h"
#include "puremvc/controller.h"
#include "puremvc/notification.h"
#include "puremvc/view.h"

#include "macro_command_test.h"
#include "macro_command_test_command.h"
#include "macro_command_test_vo.h"

int main() {
    testMacroCommandExecute();
    testRegisterAndExecuteCommand();
    return 0;
}

void testMacroCommandExecute() {
    struct MacroCommandTestVO vo = { .input = 5, 0, 0, 0};

    struct Notification notification = puremvc_notification("MacroCommandTest", &vo, NULL);

    struct SimpleCommand command = macro_command_test_command();

    command.notifier.base.initializeNotifier(&command.notifier.base, "MacroCommandTestkey1");

    command.base.execute(&command.base, &notification.base);

    assert(vo.result1 == 10);
    assert(vo.result2 == 25);
    assert(vo.result3 == 125);
}

void testRegisterAndExecuteCommand() {
    struct IController *controller = puremvc_controller_getInstance("ControllerTestKey1", puremvc_controller);

    controller->registerCommand(controller, "MacroCommandTest", macro_command_test_command);

    const struct IView *view = puremvc_view_getInstance("ControllerTestKey1", puremvc_view);

    struct MacroCommandTestVO vo = {.input = 5, 0, 0, 0};
    struct Notification notification = puremvc_notification("MacroCommandTest", &vo, NULL);

    view->notifyObservers(view, &notification.base);

    // test assertions
    assert(vo.result1 == 10);
    assert(vo.result2 == 25);
    assert(vo.result3 == 125);

    controller->removeCommand(controller, "MacroCommandTest");
    puremvc_controller_removeController("ControllerTest1");
    puremvc_view_removeView("ControllerTest1");
    controller = NULL;
}

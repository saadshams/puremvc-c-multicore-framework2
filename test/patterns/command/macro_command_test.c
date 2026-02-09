#include "macro_command_test.h"
#include "macro_command_test_command.h"
#include "macro_command_test_vo.h"

#include "puremvc/i_notification.h"

#include <alloca.h>
#include <assert.h>

int main() {
    testMacroCommandExecute();
    return 0;
}

void testMacroCommandExecute() {
    struct MacroCommandTestVO vo = { .input = 5, 0, 0, 0};

    struct INotification *notification = puremvc_notification_init(alloca(puremvc_notification_size("MacroCommandTest", NULL)), "MacroCommandTest", &vo, NULL);

    const struct ICommand *command = macro_command_test_command(alloca(puremvc_simple_command_size()));

    command->getNotifier(command)->initializeNotifier(command->getNotifier(command), "MacroCommandTestkey1");

    command->execute(command, notification);

    assert(vo.result1 == 10);
    assert(vo.result2 == 25);
    assert(vo.result3 == 125);
}

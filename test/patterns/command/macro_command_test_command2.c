#include "macro_command_test_command2.h"
#include "macro_command_test_sub3_command.h"

static void initializeMacroCommand(struct MacroCommand *self) {
    self->addSubCommand(self, macro_command_test_sub3_command);
}

struct MacroCommand macro_command_test_command2() {
    struct MacroCommand command = puremvc_macro_command();
    command.initializeMacroCommand = initializeMacroCommand;
    return command;
}

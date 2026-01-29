#include "macro_command_test_command.h"
// #include "macro_command_test_sub1_command.h"
// #include "macro_command_test_sub2_command.h"
#include "macro_command_test_command2.h"

static void initializeMacroCommand(struct MacroCommand *self) {
    // self->addSubCommand(self, macro_command_test_sub1_command);
    // self->addSubCommand(self, macro_command_test_sub2_command);
    self->addSubCommand(self, (struct SimpleCommand(*)()) macro_command_test_command2);
}

struct MacroCommand macro_command_test_command() {
    struct MacroCommand command = puremvc_macro_command();
    command.initializeMacroCommand = initializeMacroCommand;
    return command;
}

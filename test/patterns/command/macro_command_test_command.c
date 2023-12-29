#include "macro_command_test_command.h"
#include "macro_command_test_sub1command.h"
#include "macro_command_test_sub2command.h"

static void initializeMacroCommand(struct MacroCommand *self) {
    self->addSubCommand(self, macro_command_test_sub1command_new);
    self->addSubCommand(self, macro_command_test_sub2command_new);
}

struct MacroCommand *macro_command_test_command_new() {
    struct MacroCommand *macroCommand = puremvc_macro_command_new();
    macroCommand->initializeMacroCommand = initializeMacroCommand;
    return macroCommand;
}

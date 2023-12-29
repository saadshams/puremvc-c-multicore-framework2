#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "macro_command.h"
#include "notifier.h"

static void initializeMacroCommand(struct MacroCommand *self) {

}

static void addSubCommand(struct MacroCommand *self, struct SimpleCommand *(*factory)()) {
    self->subCommands->push(self->subCommands, factory);
}

static void execute(struct MacroCommand *self, struct Notification *notification) {
    self->initializeMacroCommand(self);

    while (self->subCommands->size(self->subCommands) > 0) {
        struct SimpleCommand *(*factory)() = self->subCommands->shift(self->subCommands);
        struct SimpleCommand *commandInstance = factory();
        commandInstance->notifier->initializeNotifier(self->simpleCommand.notifier, self->simpleCommand.notifier->multitonKey);
        commandInstance->execute(commandInstance, notification);
        puremvc_simple_command_free(&commandInstance);
    }

    puremvc_array_free(&self->subCommands);
}

struct MacroCommand *puremvc_macro_command_init(struct MacroCommand *self) {
    puremvc_simple_command_init(&self->simpleCommand);
    self->initializeMacroCommand = initializeMacroCommand;
    self->addSubCommand = addSubCommand;
    self->simpleCommand.execute = (void (*)(struct SimpleCommand *, struct Notification *)) execute;
    return self;
}

struct MacroCommand *puremvc_macro_command_alloc() {
    struct MacroCommand *self = malloc(sizeof(struct MacroCommand));
    if (self == NULL) goto exception;
    memset(self, 0, sizeof(*self));

    self->subCommands = puremvc_array_new();
    return self;

    exception:
    fprintf(stderr, "MacroCommand allocation failed.\n");
    return NULL;
}

struct MacroCommand *puremvc_macro_command_new() {
    return puremvc_macro_command_init(puremvc_macro_command_alloc());
}

void puremvc_macro_command_free(struct MacroCommand **self) {
    free(*self);
    *self = NULL;
}

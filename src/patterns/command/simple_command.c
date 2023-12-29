#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "simple_command.h"
#include "notifier.h"

static void execute(struct SimpleCommand *self, struct Notification *notification) {

}

struct SimpleCommand *puremvc_simple_command_init(struct SimpleCommand *self) {
    self->notifier = puremvc_notifier_new();
    self->execute = execute;
    return self;
}

struct SimpleCommand *puremvc_simple_command_alloc() {
    struct SimpleCommand *self = malloc(sizeof(struct SimpleCommand));
    if (self == NULL) goto exception;
    memset(self, 0, sizeof(*self));

    return self;

    exception:
    fprintf(stderr, "SimpleCommand allocation failed.\n");
    return NULL;
}

struct SimpleCommand *puremvc_simple_command_new() {
    return puremvc_simple_command_init(puremvc_simple_command_alloc());
}

void puremvc_simple_command_free(struct SimpleCommand **self) {
    puremvc_notifier_free(&(*self)->notifier);
    free(*self);
    *self = NULL;
}

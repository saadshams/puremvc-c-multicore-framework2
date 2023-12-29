#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "notification.h"

static const char *getName(struct Notification *self) {
    return self->name;
}

static void *getBody(struct Notification *self) {
    return self->body;
}

static void setBody(struct Notification *self, void *body) {
    self->body = body;
}

static char *getType(struct Notification *self) {
    return self->type;
}

static void setType(struct Notification *self, char *type) {
    if (self->type != NULL) free(self->type);
    self->type = strdup(type);
}

static const char *toString(struct Notification *self) {
    char *msg = malloc((strlen(self->name) + (self->type != NULL ? strlen(self->type) : 0) + 1) * sizeof(char));
    if (msg == NULL) goto exception;

    strcat(msg, self->name);
    if (self->type) strcat(msg, self->type);
    return msg;

    exception:
    fprintf(stderr, "Notification: toString allocation failed.\n");
    return NULL;
}

struct Notification *puremvc_notification_init(struct Notification *self) {
    self->getName = getName;
    self->getBody = getBody;
    self->setBody = setBody;
    self->getType = getType;
    self->setType = setType;
    self->toString = toString;
    return self;
}

struct Notification *puremvc_notification_alloc(const char *name, void *body, char *type) {
    struct Notification *self = malloc(sizeof(struct Notification));
    if (self == NULL) goto exception;
    memset(self, 0, sizeof(*self));

    self->name = name != NULL ? strdup(name) : NULL;
    self->body = body;
    self->type = type != NULL ? strdup(type) : NULL;
    return self;

    exception:
    fprintf(stderr, "Notification allocation failed.\n");
    return NULL;
}

struct Notification *puremvc_notification_new(const char *name, void *body, char *type) {
    return puremvc_notification_init(puremvc_notification_alloc(name, body, type));
}

void puremvc_notification_free(struct Notification **self) {
    free((void *) (*self)->name);
    (*self)->name = NULL;
    free((void *) (*self)->type);
    (*self)->type = NULL;
    free(*self);
    *self = NULL;
}

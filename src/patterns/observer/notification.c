/**
* @file notification.c
* @internal
* @brief Notification Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include <stdio.h>

#include "puremvc/notification.h"

static const char *getName(const struct Notification *self) {
    return self->name;
}

static const void *getBody(const struct Notification *self) {
    return self->body;
}

static void setBody(struct Notification *self, void *body) {
    self->body = body;
}

static const char *getType(const struct Notification *self) {
    return self->type;
}

static void setType(struct Notification *self, const char *type) {
    snprintf(self->type, MAX_NAME_LEN, "%s", type);
}

void toString(const struct Notification *self, char *buffer, size_t buffer_size) {
    snprintf(buffer, buffer_size, "%s : %s [body=%p]", self->name, self->type, self->body);
}

struct Notification puremvc_notification(const char *name, void *body, const char *type) {
    struct Notification notification = {0};

    snprintf(notification.name, MAX_NAME_LEN, "%s", name);
    notification.body = body;
    if (type != NULL) snprintf(notification.type, MAX_NAME_LEN, "%s", type);

    notification.getName = getName;
    notification.getBody = getBody;
    notification.setBody = setBody;
    notification.getType = getType;
    notification.setType = setType;
    notification.toString = toString;

    return notification;
}

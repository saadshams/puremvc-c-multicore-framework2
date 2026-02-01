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
    int len = snprintf(self->type, NAME_SIZE, "%s", type);
    if (len >= NAME_SIZE)
        fprintf(stderr, "[PureMVC::Notification::setType] Warning: Type Truncated: '%s' (Original length: %d, Buffer size: %d)\n", type, len, NAME_SIZE);
}

void toString(const struct Notification *self, char *buffer, size_t buffer_size) {
    int len = snprintf(buffer, buffer_size, "%s : %s [body=%p]", self->name, self->type, self->body);
    if (len < 0 || (size_t)len >= buffer_size)
        fprintf(stderr, "[PureMVC::Notification::toString] Warning: Buffer Truncated: '%zu' (Original length: %d, Buffer size: %d)\n", buffer_size, len, NAME_SIZE);
}

struct Notification puremvc_notification(const char *name, void *body, const char *type) {
    struct Notification notification = {
        .body = body,
        .getName = getName,
        .getBody = getBody,
        .setBody = setBody,
        .getType = getType,
        .setType = setType,
        .toString = toString
    };

    int len = snprintf(notification.name, NAME_SIZE, "%s", name);
    if (len >= NAME_SIZE)
        fprintf(stderr, "[PureMVC::Notification] Warning: Name Truncated: '%s' (Original length: %d, Buffer size: %d)\n", name, len, NAME_SIZE);

    if (type != NULL) {
        len = snprintf(notification.type, NAME_SIZE, "%s", type);
        if (len >= NAME_SIZE)
            fprintf(stderr, "[PureMVC::Notification] Warning: Type Truncated: '%s' (Original length: %d, Buffer size: %d)\n", type, len, NAME_SIZE);
    }

    return notification;
}

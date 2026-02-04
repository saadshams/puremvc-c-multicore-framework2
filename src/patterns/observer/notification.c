/**
* @file notification.c
* @internal
* @brief Notification Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include "puremvc/notification.h"

#include <stdio.h>

static const char *getName(const struct INotification *self) {
    const struct Notification *this = (struct Notification *) self;
    return this->name;
}

static const void *getBody(const struct INotification *self) {
    const struct Notification *this = (struct Notification *) self;
    return this->body;
}

static void setBody(struct INotification *self, void *body) {
    struct Notification *this = (struct Notification *) self;
    this->body = body;
}

static const char *getType(const struct INotification *self) {
    const struct Notification *this = (struct Notification *) self;
    return this->type;
}

static void setType(struct INotification *self, const char *type) {
    struct Notification *this = (struct Notification *) self;
    int len = snprintf(this->type, NAME_SIZE, "%s", type);
    if (len >= NAME_SIZE)
        fprintf(stderr, "[PureMVC::Notification::setType] Warning: Type Truncated: '%s' (Original length: %d, Buffer size: %d)\n", type, len, NAME_SIZE);
}

void toString(const struct INotification *self, char *buffer, size_t buffer_size) {
    const struct Notification *this = (struct Notification *) self;
    int len = snprintf(buffer, buffer_size, "%s : %s [body=%p]", this->name, this->type, this->body);
    if (len < 0 || (size_t)len >= buffer_size)
        fprintf(stderr, "[PureMVC::Notification::toString] Warning: Buffer Truncated: '%zu' (Original length: %d, Buffer size: %d)\n", buffer_size, len, NAME_SIZE);
}

struct INotification *puremvc_notification(struct Notification *const notification, const char *name, void *body, const char *type) {
    notification->base.getName = getName;
    notification->base.getBody = getBody;
    notification->base.setBody = setBody;
    notification->base.getType = getType;
    notification->base.setType = setType;
    notification->base.toString = toString;

    notification->body = body;

    int len = snprintf(notification->name, NAME_SIZE, "%s", name);
    if (len >= NAME_SIZE)
        fprintf(stderr, "[PureMVC::Notification] Warning: Name Truncated: '%s' (Original length: %d, Buffer size: %d)\n", name, len, NAME_SIZE);

    if (type != NULL) {
        len = snprintf(notification->type, NAME_SIZE, "%s", type);
        if (len >= NAME_SIZE)
            fprintf(stderr, "[PureMVC::Notification] Warning: Type Truncated: '%s' (Original length: %d, Buffer size: %d)\n", type, len, NAME_SIZE);
    }

    return &notification->base;
}

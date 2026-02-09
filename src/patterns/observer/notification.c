/**
* @file notification.c
* @internal
* @brief Notification Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include "notification.h"

#include <stdio.h>
#include <string.h>

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
    this->type = type;
}

void toString(const struct INotification *self, char *buffer, size_t buffer_size) {
    const struct Notification *this = (struct Notification *) self;
    int len = snprintf(buffer, buffer_size, "%s : %s [body=%p]", this->name, this->type, this->body);
    if (len < 0 || (size_t) len >= buffer_size)
        fprintf(stderr, "\033[0;33m[PureMVC::Notification::toString] Warning: Buffer Truncated: (Required: %d, Available: %zu).\033[0m\n", len, buffer_size);
}

size_t puremvc_notification_size() {
    return (sizeof(struct Notification) + (sizeof(void *) - 1)) & ~(sizeof(void *) - 1);
}

struct INotification *puremvc_notification_init(void *buffer, const char *name, void *body, const char *type) {
    struct Notification *this = (struct Notification *) buffer;

    memset(this, 0, sizeof *this);

    this->base.getName = getName;
    this->base.getBody = getBody;
    this->base.setBody = setBody;
    this->base.getType = getType;
    this->base.setType = setType;
    this->base.toString = toString;

    this->name = name;
    this->body = body;
    this->type = type;

    return (struct INotification *) this;
}

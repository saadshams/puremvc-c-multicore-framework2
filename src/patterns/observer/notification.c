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
    return this->data;
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
    return &this->data[this->name_len];
}

static void setType(struct INotification *self, const char *type) {
    struct Notification *this = (struct Notification *) self;

    const size_t len = strlen(type) + 1;
    if (len <= this->type_len) { // SAFETY CHECK: In a FAM pattern, we cannot exceed the original allocated type_len
        strcpy(&this->data[this->name_len], type);
    } else {
        fprintf(stderr, "\033[0;33m[PureMVC::Notification::setType] Warning: Type Truncated: '%s' (Required: %zu, Available: %zu).\033[0m\n", type, len, this->type_len);
        strncpy(&this->data[this->name_len], type, this->type_len - 1); // Copy only what fits
        this->data[this->name_len + this->type_len - 1] = '\0';
    }
}

void toString(const struct INotification *self, char *buffer, size_t buffer_size) {
    const struct Notification *this = (struct Notification *) self;
    int len = snprintf(buffer, buffer_size, "%s : %s [body=%p]", this->data, &this->data[this->name_len], this->body);
    if (len < 0 || (size_t) len >= buffer_size)
        fprintf(stderr, "\033[0;33m[PureMVC::Notification::toString] Warning: Buffer Truncated: (Required: %d, Available: %zu).\033[0m\n", len, buffer_size);
}

size_t puremvc_notification_size(const char *name, const char *type) {
    size_t name_len = name ? strlen(name) + 1 : 1;
    size_t type_len = type ? strlen(type) + 1 : 1;

    // if (n_len > 1024 || t_len > 1024) return 0; // Check if lengths are suspiciously large for a stack (Optional)

    return (sizeof(struct Notification) + name_len + type_len + (sizeof(void *) - 1)) & ~(sizeof(void *) - 1);
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

    this->body = body;

    this->name_len = name != NULL ? strlen(name) + 1 : 1;
    snprintf(this->data, this->name_len, "%s", name ? name : "");

    this->type_len = type != NULL ? strlen(type) + 1 : 1;
    snprintf(&this->data[this->name_len], this->type_len, "%s", type ? type : "");

    return (struct INotification *) this;
}

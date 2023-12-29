#ifndef PUREMVC_NOTIFICATION_H
#define PUREMVC_NOTIFICATION_H

struct Notification {
    const char *name;
    void *body;
    char *type;

    const char *(*getName)(struct Notification *self);
    void *(*getBody)(struct Notification *self);
    void (*setBody)(struct Notification *self, void *body);
    char *(*getType)(struct Notification *self);
    void (*setType)(struct Notification *self, char *type);
    const char *(*toString)(struct Notification *self);
};

struct Notification *puremvc_notification_init(struct Notification *self);

struct Notification *puremvc_notification_alloc(const char *name, void *body, char *type);

struct Notification *puremvc_notification_new(const char *name, void *body, char *type);

void puremvc_notification_free(struct Notification **self);

#endif //PUREMVC_NOTIFICATION_H

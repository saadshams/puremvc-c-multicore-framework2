#ifndef PUREMVC_PROXY_H
#define PUREMVC_PROXY_H

#define PROXY_NAME "Proxy"

struct Notifier;

struct Proxy {
    struct Notifier *notifier;
    const char *proxyName;
    void *data;
    void (*onRegister)(struct Proxy *self);
    void (*onRemove)(struct Proxy *self);
    const char *(*getProxyName)(const struct Proxy *self);
    void *(*getData)(const struct Proxy *self);
    void (*setData)(struct Proxy *self, void *data);
};

struct Proxy *puremvc_proxy_init(struct Proxy *self);

struct Proxy *puremvc_proxy_alloc(const char *proxyName, void *data);

struct Proxy *puremvc_proxy_new(const char *proxyName, void *data);

void puremvc_proxy_free(struct Proxy **self);

#endif //PUREMVC_PROXY_H

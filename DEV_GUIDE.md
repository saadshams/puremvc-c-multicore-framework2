* User-provided storage (very clean API design)

This is the most honest pattern:

struct FactoryRegistry {
Factory *items;
size_t capacity;
size_t count;
};


Caller decides:

Factory storage[32];
struct FactoryRegistry reg = {
.items = storage,
.capacity = 32
};


Library never guesses.
Library never mallocs.
Caller owns the constraint.

This is how serious C APIs avoid the question entirely.

* Key Truncation Collision
* library is thread-safe because it uses internal mutexes,

### lives for entire program

```c++
struct ViewMap *storage[] = {
  ({
      static struct Observer observers[3]; // lives for entire program
      &(struct ViewMap){
        .view = {
          .mediatorMap = (struct MediatorMap*[]){ &(struct MediatorMap){0}, NULL },
          .observerMap = (struct ObserverMap*[]){
            &(struct ObserverMap){ .observers = (struct IObserver *[]){ &observers[0].base, NULL } },
            &(struct ObserverMap){ .observers = (struct IObserver *[]){ &observers[1].base, NULL } },
            &(struct ObserverMap){ .observers = (struct IObserver *[]){ &observers[2].base, NULL } },
            NULL
          },
        },
      };
  }),
  NULL
};

```

Option 2 — make it static (lives forever)
```c++
static struct Mediator obj;
view->registerMediator(view, view_test_mediator4(&obj, &viewTest)
```

loop

```c++
#define NUM_MEDIATORS 4

// Pre-allocate storage for Mediators
struct Mediator mediatorStorage[NUM_MEDIATORS] = {0};

// Pre-allocate storage for MediatorMap slots
struct MediatorMap mediatorMapStorage[NUM_MEDIATORS] = {0};

// Fill the mediatorMap in a loop
for (int i = 0; i < NUM_MEDIATORS; i++) {
    mediatorMapStorage[i].mediator = &mediatorStorage[i].base;
    mediatorMapStorage[i].key[0] = '\0'; // empty key
}

// Then build the mediatorMap array with NULL terminator
struct MediatorMap *mediatorMapArray[NUM_MEDIATORS + 1];
for (int i = 0; i < NUM_MEDIATORS; i++) {
    mediatorMapArray[i] = &mediatorMapStorage[i];
}
mediatorMapArray[NUM_MEDIATORS] = NULL; // terminate array

// Now you can assign to your View
struct View viewStorage = {
    .mediatorMap = mediatorMapArray,
    .observerMap = NULL // fill separately if needed
};

```

Controller storage

```c++
// Step 1: static storage for empty commandMap
static struct CommandMap initialCommandMap[] = { { .key = "", .factory = NULL } };
static struct CommandMap *commandMapArray[] = { &initialCommandMap[0], NULL };

// Step 2: static storage for controllers
static struct Controller controllerStorage[] = {
    {
        .base = {0},
        .multitonKey = "",
        .view = NULL,
        .commandMapMutex = {0},
        .commandMap = commandMapArray
    }
};
static struct Controller *storage[] = { &controllerStorage[0], NULL };

```

Now this array lives in BSS (like your global) and will exist for the entire program.
```c++
void testGetInstance() {
    static struct ViewMap *viewMap[] = {  // <-- NOTE: static
        &(struct ViewMap) {
            .view = { .multitonKey = "", .observerMap = {}, .mediatorMap = {} },
        }, 
        NULL 
    };

    puremvc_view_getInstance(viewMap, "ControllerTestKey1");
}
```

Allocate on the heap
```c++
void testGetInstance() {
    struct ViewMap **viewMap = calloc(2, sizeof(struct ViewMap*));
    viewMap[0] = malloc(sizeof(struct ViewMap));
    *viewMap[0] = (struct ViewMap){ .view = { .multitonKey = "", .observerMap = {}, .mediatorMap = {} } };

    puremvc_view_getInstance(viewMap, "ControllerTestKey1");
}
```

FATAL   ->  \033[1;31mFATAL\033[0m
ERROR   ->  \033[0;31mERROR\033[0m
WARN    ->  \033[0;33mWARN\033[0m
INFO    ->  \033[0;37mINFO\033[0m
DEBUG   ->  \033[0;32mDEBUG\033[0m
TRACE   ->  \033[0;90mTRACE\033[0m

Comments

```c++
struct ControllerMap *storage[] = { &(struct ControllerMap) {
    .controller = (struct IController *) &(struct Controller) {
        /* Define command dispatch table (Capacity: 4 Commands) */
        .commandMap = (struct CommandMap *[]) { 
            &(struct CommandMap){}, &(struct CommandMap){}, 
            &(struct CommandMap){}, &(struct CommandMap){},
            NULL 
        }
    },
}, NULL };

struct ControllerMap *storage[] = { 
    &(struct ControllerMap) {
        .controller = (struct IController *) &(struct Controller) {
            .commandMap = (struct CommandMap *[]) { 
                &(struct CommandMap){}, // Command 1
                &(struct CommandMap){}, // Command 2
                &(struct CommandMap){}, // Command 3
                &(struct CommandMap){}, // Command 4
                NULL                    // End of Map
            } // end commandMap
        } // end Controller
    }, // end ControllerMap
    NULL // End of storage
};
```

**The "Hidden" Dangers in Embedded**

Even though the language is "good," embedded compilers often implement the C standard in a "freestanding" environment. This means some parts of C11 might be missing or dangerous:

Variable Length Arrays (VLAs): C11 made VLAs optional. In embedded, you should avoid them. They behave like alloca(), but if the size is too big, they crash the stack silently. Your current alloca() approach is actually more "explicit" and often preferred.

The Standard Library: Functions like printf or snprintf (which you used) are often very "heavy" for tiny microcontrollers (they can add 20KB+ to your binary). Most embedded devs use a "mini-printf" library.
https://github.com/mludvig/mini-printf
Multithreading (threads.h): Most embedded compilers (like GCC for ARM) do not implement the C11 threads library. They expect you to use FreeRTOS or POSIX threads instead.

If you are building an embedded system where stack space is tight (e.g., 2KB total stack), go with Option 1 (Double-Buffer FAM).

**Stack Depth (The "Runtime" Cost**

```c++
char buf[512];
snprintf(buf, sizeof(buf), "[SUITE] %s\n", "ControllerTest");
fwrite(buf, 1, strlen(buf), stdout);
```

```c++
#include <sys/resource.h>

void print_memory_usage() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    printf("Max RSS: %ld KB\n", usage.ru_maxrss);
}
```

```c++
    // 1. Explicitly named buffers
    // 1. Use volatile pointers to force the compiler to keep them in memory
    void *volatile model_buffer = alloca(puremvc_model_size());
    void *volatile proxy_buffer = alloca(puremvc_proxy_size());

    // FORCE the compiler to treat this memory as "used" immediately
    // memset(model_buffer, 0, puremvc_model_size());
    // memset(proxy_buffer, 0, puremvc_proxy_size());

    // 2. Named Slots (Do NOT use anonymous compound literals)
    struct ModelMap model_slot = { .model = model_buffer };
    struct ProxyMap proxy_slot = { .proxy = proxy_buffer }; // key set later by register

    // 3. Named Maps
    struct ModelMap *instanceMap[] = { &model_slot, NULL };
    struct ProxyMap *proxyMap[] = { &proxy_slot, NULL };
```

That looks airtight now. You've covered the null parameters, the internal state guards, the debug-only informational logs, and—crucially—the mutex cleanup on all exit paths.

The flow is now logically complete:

1. **Validation** (Fail fast)
2. **Concurrency Lock**
3. **Search & Update** (with early exit)
4. **Capacity Check**
5. **Side Effect** (Registering with the View)
6. **Concurrency Unlock**

### Final Code Checklist for `registerCommand`

* [x] **Null Guards**: `notificationName` and `factory` are safe.
* [x] **Deadlock Prevention**: Mutex is unlocked on all `return` paths after it's acquired.
* [x] **Release Build Compliance**: `printf` is hidden behind `NDEBUG`.
* [x] **State Integrity**: Checks `view` and `commandMap` before attempting logic.

**Ready for the next one? Paste the next function when you're ready to review.**

🔥 Important Insight

You don't need one architecture for everything.
PureMVC was originally designed for GC/heap languages (Java, AS3, etc.).
You're adapting it to:

* C
* No heap
* Deterministic lifetime

So you must split static vs dynamic layers.

That’s not breaking PureMVC — that’s correctly porting it.

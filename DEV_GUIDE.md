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
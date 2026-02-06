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
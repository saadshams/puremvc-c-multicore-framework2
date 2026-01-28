D) User-provided storage (very clean API design)

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
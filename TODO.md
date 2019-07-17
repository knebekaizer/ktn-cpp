TODO:
-----

- extern "C"
- Add common prefix (e.g. $_) to C mangling, to avoid collision with simple C++ types declared in the global namespace
- Hide API with non-POD by-value args and returns. Map POD types to C opaque blob:

```C
    typedef struct {
        char blob[MyPODClass_size];
    } $_MyPODClass;
```
- Forward declaration of proxy types
- include headers ???
- map operator<< etc to C functions (this may work)
- ctors & dtor

Done:
----

- skip duplicates

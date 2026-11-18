# SPString



A secure C string library that's designed with embedded development in mind.

This is yet another implementation of the classic {length, char*} structure which provides much better security than char* alone as it prevents buffer overflows by automating changes in string length. The programmer no longer needs to worry about this.
This particular implementation was designed so it can be used either with stack based preallocated buffers or dynamically allocated and resizable buffers, depending on the hardware requirements. The code is also simple enough to be easily understandable and extensible.

This library provides two types of strings:

1. **String** is a fully dynamic string. It will resize automatically
at runtime, just like Java or C++ strings. It comes with a host of functions that mirror the C string functions, but will additionally resize the target buffer when it's too small to perform the operation.

2. **LString** (Local String) has a fixed buffer size defined at declaration.
It comes with a host of functions that mirror the C string functions (lstringcpy and the like) that can check at runtime that the correct buffer sizes are respected
and will raise an error if it's not the case, instead of silently overflowing. These functions allow to detect and pinpoint buffer overflow bugs during testing phase.

## Build and Test

Build with `make`.

Run the regression suite with `make check` . This will run both with and without
the optional buffer-clearing mode enabled.

Run the demo with `./demo`.

# How to use the library

In order to operate, the actual structure of String and LString is {buffer size, length, char*}, where buffer size is the length of the allocated buffer while length is the length of the string itself (terminal \0 excluded). Both values are maintained automatically and therefore the user should **not** modify the String or LString structures directly, but should instead strive to always use the string functions provided by the library to manipulate them.

One can easily create a String from a C string with 
newString(const char *) 

String.str always returns a null-terminated C chain, and String.len
returns its length.

Most functions mirror standard C functions. You will need to allocate a String with newString and delete it with delString.

Here is a helpful correspondence table between C functions and their counterparts:

|C function        |String                              |LString
|:-----------------|:-----------------------------------|:-----------------------
|malloc            |newString                           |localString
|free              |delString                           |- (stack allocated)
|strlen            |stringlen                           |lstringlen
|strcpy            |stringcpy, stringchcpy              |lstringcpy, lstringchcpy
|strncpy           |stringcpy, stringchcpy              |lstringcpy, lstringchcpy
|strcat            |stringcat, stringchcat              |lstringcat, lstringchcat
|strcmp            |stringcmp, stringchcmp              |lstringcmp, lstringchcmp
|strdup            |stringdup                           |lstringcpy *
|truncation        |stringtrunc                         |lstringtrunc
|printf            |stringprintf                        |-

* Why is there no lstringdup? While stringdup allocates a buffer dynamically, for lstringdup, one has to allocate it statically beforehand, so it doesn't do anything else than lstringcpy.

The source of a copy or concatenation, and the arguments of `stringprintf`, may point inside the destination buffer (for instance `stringchcat(s, s->str)`): the library handles the overlap and the reallocation.

Use `lstringtrunc` to truncate a fixed-buffer `LString`; both truncation functions retain
up to the requested number of characters.

Additionally, the functions can internally raise an error, but because the functions try to mimic the standard C functions interfaces, the error code is not always returned.
One has to call `get_stringerr()` to retrieve and reset the error code. Apart from allocation failures (memory full), checking for errors should mainly be needed during testing or debugging.

|Error type                 |Value   |Signification
|:--------------------------|:-------|:-----------------------------------------
|ST_NO_ERR                  |-1000   |No error
|ST_OVERWRITE_ERR           |-1001   |Buffer overwrite detected
|ST_ALLOC_ERR               |-1002   |Allocation failed
|ST_NULLPARAM_ERR           |-1003   |One of the function parameters was NULL
|ST_NULLSTR_ERR             |-1004   |The String/LString buffer is NULL
|ST_INCONSISTENTSZ_ERR      |-1005   |Inconsistent buffer size detected (corruption)
|ST_INCONSISTENTLEN_ERR     |-1006   |Inconsistent length detected (corruption)

The error state is thread-local (one per thread, like `errno`) when the compiler supports it: an error raised in one thread is never seen or reset by another. On a compiler not recognized by the implementation, it falls back to an ordinary static variable and is not thread-safe. Define `SPSTRING_NO_THREADS` to explicitly use that fallback on targets without thread-local storage support. The error state is only touched when an error occurs or when `get_stringerr()` is called.

The comparison functions always return a valid ordering, even when given NULL: a NULL `String` (or a NULL buffer) sorts before any string and two NULLs compare equal, so they can safely be used with `qsort`. The NULL parameter is reported through `get_stringerr()`.

Finally, `stringcheck` and `lstringcheck` perform several verifications on a `String` or `LString` to check that it is not corrupted, and return 0 or the corresponding error code. The `STRINGCHECK(s)` and `LSTRINGCHECK(s)` macros also print a `file:line:` diagnostic on stderr when a problem is found; pass `NULL` as file to the functions for a silent check. They can be useful in debugging sessions.

## Preprocessor flags

- `TIDY` keeps the unused buffer space zeroed. Turned off by default.
- `SPSTRING_NO_THREADS` makes the error state a plain static variable instead of a thread-local one, for targets without thread-local storage support (bare metal, some RTOS). Turned off by default.

The test suite uses POSIX threads and must be linked with `-pthread` (the Makefile does it).


Published under MIT License
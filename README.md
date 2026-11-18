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

Note that the comparison functions return a negative error code (e.g. `ST_NULLPARAM_ERR`) when a parameter is NULL, which cannot be told apart from a regular "less than" result: call `get_stringerr()` if NULL parameters are possible. The error state is a single global variable, so it is not thread-safe.

Finally, `stringcheck` performs several verifications on a dynamic `String` to check that it is not corrupted. It can be useful in debugging sessions.

## Preprocessor flags

The optional `TIDY` flag keeps the unused buffer space zeroed. `stringcheck` can be called explicitly to verify a dynamic string's internal consistency.

`TIDY` is turned off by default.

Published under MIT License

## Tests

Run the regression suite with `make test` (also included in `make check`). Run it with
the optional buffer-clearing mode enabled using `make test-tidy`.

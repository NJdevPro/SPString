# SPString



A secure C string library that's designed with embedded development in mind.

This is yet another implementation of the classic {length, char*} structure which provides much better security than char* alone as it prevents buffer overflows by automating changes in string length. The programmer no longer needs to worry about this.
This particular implementation was designed so it can be used either with stack based preallocated buffers or dynamically allocated and resizable buffers, depending on the hardware requirements. The code is also simple enough to be easily understandable and extensible.

This library provides two types of strings:

1. **String** is a fully dynamic string. It will resize automatically
at runtime, just like Java or C++ strings. It comes with a host of functions that mirror the C string functions, but will additionnally resize the target buffer when it's too small to perform the operation.

2. **LString** (Local String) has a fixed buffer size defined at declaration.
It comes with a host of functions that mirror the C string functions (lstringcpy and the like) that can check at runtime that the correct buffer sizes are respected
and will raise an error if it's not the case, instead of silently overflowing. These functions allow to detect and pinpoint buffer overflow bugs during testing phase.


# How to use the library

In order to operate, the actual structure of String and LString is {length, buffer size, char*}, where buffer size is the length of the allocated buffer while length is the length of the string itself (terminal \0 excluded). Both values are maintained automatically and therefore the user should **not** modify the String or LString structures directly, but should instead strive to always use the string functions provided by the library to manipulate them.

One can easily create a String from a C string with 
newString(const char *) 

String.str always returns a null-terminated C chain, and String.len
returns its length.

Most functions mirror standard C functions. You will need to allocate a String with newString and delete it with delString.

Here is a helpful correspondance table between C functions and their counterparts:

|C function        |String                              |LString
|:-----------------|:-----------------------------------|:-----------------------
|malloc            |newString                           |localString
|free              |delString                           |- (stack allocated)
|strlen            |stringlen                           |stringlen
|strcpy            |stringcpy, stringchcpy              |lstringcpy, lstringchpy
|strncpy           |stringcpy, stringchcpy              |lstringcpy, lstringchpy
|strcat            |stringcat, stringchcat              |lstringcat, ltringchcat
|strcmp            |stringcmp, stringchcmp              |lstringcmp, lstringchcmp
|strdup            |stringdup                           |lstringdup
|truncation        |stringtrunc                         |stringtrunc
|printf            |stringprintf                        |stringprintf


Additionnally, the functions can internally raise an error, but because the functions try to mimic the standard C functions interfaces, the error code is not always returned.
One has to call get_error() to know the value of an error code. However, apart from allocation failures (memory full), the need to check fo error should only arise at testing or debugging time.

|Error type                 |Value   |Signification
|:--------------------------|:-------|:-----------------------------------------
|ST_NO_ERR                  |-1000   |No error
|ST_OVERWRITE_ERR           |-1001   |Buffer overwrite detected
|ST_ALLOC_ERR               |-1002   |Allocation failed
|ST_NULLPARAM_ERR           |-1003   |One of the function parameters was NULL
|ST_NULLSTR_ERR             |-1004   |The String/LString buffer is NULL
|ST_INCONSISTENTSZ_ERR      |-1005   |Inconsistent buffer size detected (corruption)
|ST_INCONSISTENTLEN_ERR     |-1006   |Inconsistent length detected (corruption)

Finally, stringcheck performs several verifications on a String or a LString in order to check that it is not corrupted. It can be useful in debugging sessions.

Published under MIT License

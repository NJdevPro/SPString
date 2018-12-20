# SPString
A C string library that's designed for embedded development in mind.

This is yet another implementation of the classic {length, char pointer} structure.
It was designed so it could be used either with stack based preallocated buffers or dynamically
allocated and resizable buffers, depending on the hardware requirements.

It's quite simple and does nothing really fancy so you can easily understand what it
does, but when I wrote it in 2009, it was fully tested for embedded hardware with small
memory requirements. It passes Valgrind and has been successfully used in production.
Unfortunaly I no longer have the tests suite so you'll have to write your own before 
using this library.

Enjoy !

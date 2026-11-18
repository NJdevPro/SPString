/*
*  C Implementation: SPString
*
* Description:
*    This class provides a safe and convenient, albeit slower, alternative
*    to C character chains.
*
*    One can easily create a String from a C string with newString(char *).
*    String.str always returns a null-terminated C chain, and String.len
*    returns its length.
*    One should always avoid to manipulate the String structure members
*    directly in order to prevent corruption (i.e incorrect sz and len values).
*
*    Most functions mirror standard C functions.
*    stringcpy and stringcat can increase their buffer size if necessary in
*    order to prevent buffer overflow.
*
* Author: Nicolas Janin <>, (C) 2009
*
* Published under MIT License
*
*/
 
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include "spstring.h"
 
/* private (internal mechanics) */
/* Error state: one per thread, like errno.
   Define SPSTRING_NO_THREADS on targets without thread-local storage
   support (bare metal, some RTOS); ST_ERR is then a plain static. */
#if defined(SPSTRING_NO_THREADS)
#  define SP_THREAD_LOCAL
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#  define SP_THREAD_LOCAL _Thread_local
#elif defined(__GNUC__)
#  define SP_THREAD_LOCAL __thread
#elif defined(_MSC_VER)
#  define SP_THREAD_LOCAL __declspec(thread)
#else
#  define SP_THREAD_LOCAL   /* unknown compiler: not thread-safe */
#endif

static SP_THREAD_LOCAL int ST_ERR = ST_NO_ERR;
static int nullorder ( const void *a, const void *b );
static int incsz ( String *string, size_t len );
static int inbuffer ( const String *string, const char *p );

/* --- Dynamic allocation
 
 String can dynamically increase in size as necessary. */
String * newString ( const char *chars )
{
        String *newS = NULL;
 
        if ( chars != NULL )
        {
                size_t len = strlen ( chars );
                newS = ( String * ) malloc ( sizeof ( String ) );
                if ( newS != NULL )
                {
                        newS->len = len;
                        newS->sz = len + 1;
                        newS->str = malloc ( len + 1 );
                        if ( newS->str == NULL )
                        {
                                ST_ERR = ST_ALLOC_ERR;
                                free ( newS );
                                newS = NULL;
                        }
                        else
                        {
                                memcpy ( newS->str, chars, len + 1 );
                        }
                }
                else
                {
                        ST_ERR = ST_ALLOC_ERR;
                }
        }
        else
        {
                ST_ERR = ST_NULLPARAM_ERR;
        }
        return newS;
}
 
void delString ( String *string )
{
        if ( string != NULL )
        {
                if ( string->str != NULL )
                {
                        free ( string->str );
                        string->str = NULL;
                }
                else
                {
                        ST_ERR = ST_NULLSTR_ERR;
                }
                free ( string );
                string = NULL;
        }
        else
        {
                ST_ERR = ST_NULLPARAM_ERR;
        }
}
 
/* Copy - Will increase dst allocation if src is too large */
size_t stringcpy ( String *dst, const String *src )
{
        if ( dst == NULL || src == NULL || dst->str == NULL || src->str == NULL )
        {
                ST_ERR = dst == NULL || src == NULL ? ST_NULLPARAM_ERR : ST_NULLSTR_ERR;
                return ( size_t ) ST_ERR;
        }
        size_t len = src->len;
        if ( len == SIZE_MAX )
        {
                ST_ERR = ST_ALLOC_ERR;
                return ( size_t ) ST_ERR;
        }
        if ( dst == src )
                return len;
        if ( dst->sz <= len )
        {
                if ( incsz ( dst, len + 1 ) == ST_ALLOC_ERR )
                {
                        return ST_ALLOC_ERR;
                }
        }
 
        dst->len = len;
        memmove ( dst->str, src->str, len + 1 );
#ifdef TIDY
        memset ( dst->str + len + 1, 0, dst->sz - len - 1 );
#endif
        return len;
}
 
/* Copy - Will increase dst allocation if src is too large */
size_t stringchcpy ( String *dst, const char *src )
{
        if ( dst == NULL || src == NULL )
        {
                ST_ERR = ST_NULLPARAM_ERR;
                return ST_NULLPARAM_ERR;
        }
        if ( dst->str == NULL )
        {
                ST_ERR = ST_NULLSTR_ERR;
                return ( size_t ) ST_ERR;
        }
        size_t len = strlen ( src );
        /* src may point inside dst->str: keep its offset across realloc */
        int alias = inbuffer ( dst, src );
        size_t offset = alias ? ( size_t ) ( src - dst->str ) : 0;
        if ( dst->sz <= len )
        {
                if ( incsz ( dst, len + 1 ) == ST_ALLOC_ERR )
                {
                        return ST_ALLOC_ERR;
                }
                if ( alias )
                        src = dst->str + offset;
        }
 
        dst->len = len;
        memmove ( dst->str, src, len + 1 );
#ifdef TIDY
        memset ( dst->str + len + 1, 0, dst->sz - len - 1 );
#endif
        return len;
}
 
/* Concatenation - Will increase dst allocation when necessary */
size_t stringcat ( String *dst, const String *src )
{
        if ( dst == NULL || src == NULL || dst->str == NULL || src->str == NULL )
        {
                ST_ERR = dst == NULL || src == NULL ? ST_NULLPARAM_ERR : ST_NULLSTR_ERR;
                return ( size_t ) ST_ERR;
        }
        size_t oldlen = dst->len;
        size_t len = src->len;
        if ( oldlen == SIZE_MAX || len >= SIZE_MAX - oldlen )
        {
                ST_ERR = ST_ALLOC_ERR;
                return ( size_t ) ST_ERR;
        }
        size_t finallen = oldlen + len;
        if ( dst->sz <= finallen )
        {
                if ( incsz ( dst, finallen + 1 ) == ST_ALLOC_ERR )
                {
                        return ST_ALLOC_ERR;
                }
        }
 
        memmove ( dst->str + oldlen, src->str, len + 1 );
        dst->len = finallen;
#ifdef TIDY
        size_t l = dst->len;
        char * end =  dst->str + l;
        for ( ; l < dst->sz; l++ ) *end++ = 0;
#endif
        return len;
}
 
/* Concatenation - Will increase dst allocation when necessary */
size_t stringchcat ( String *dst, const char *src )
{
        if ( dst == NULL || src == NULL )
        {
                ST_ERR = ST_NULLPARAM_ERR;
                return ST_NULLPARAM_ERR;
        }
        if ( dst->str == NULL )
        {
                ST_ERR = ST_NULLSTR_ERR;
                return ( size_t ) ST_ERR;
        }
        size_t oldlen = dst->len;
        size_t len = strlen ( src );
        if ( oldlen == SIZE_MAX || len >= SIZE_MAX - oldlen )
        {
                ST_ERR = ST_ALLOC_ERR;
                return ( size_t ) ST_ERR;
        }
        /* src may point inside dst->str: keep its offset across realloc */
        int alias = inbuffer ( dst, src );
        size_t offset = alias ? ( size_t ) ( src - dst->str ) : 0;
        size_t finallen = oldlen + len;
        if ( dst->sz <= finallen )
        {
                if ( incsz ( dst, finallen + 1 ) == ST_ALLOC_ERR )
                {
                        return ST_ALLOC_ERR;
                }
                if ( alias )
                        src = dst->str + offset;
        }
 
        memmove ( dst->str + oldlen, src, len + 1 );
        dst->len = finallen;
#ifdef TIDY
        size_t l = dst->len;
        char * end =  dst->str + l;
        for ( ; l < dst->sz; l++ ) *end++ = 0;
#endif
 
        return len;
}
 
/* Duplication - allocate a new String */
String * stringdup ( const String *src )
{
        String *string = NULL;
        if ( src != NULL && src->str != NULL )
        {
                string = newString ( src->str );
        }
        else
        {
                ST_ERR = src == NULL ? ST_NULLPARAM_ERR : ST_NULLSTR_ERR;
        }
        return string;
}
 
 
/* ---- Allocation on the stack (non dynamic) */
 
/*
    Allocation on the stack
    Here, szMax is a maximum size of the buffer, which is allocated
    once and for all, and cannot be increased at runtime.
    Furthermore, the chars array must not be deallocated before the release
    of the LString (else mangling pointer).
    If you need fully extensible Strings, use dynamic allocation instead.
*/
LString localString ( char *chars, size_t szMax )
{
        LString newS;
        if ( chars != NULL && szMax > 0 )
        {
                size_t len = 0;
                while ( len < szMax && chars[len] != '\0' )
                        ++len;
                if ( len == szMax )
                {
                        chars[szMax - 1] = '\0';
                        len = szMax - 1;
                }
                newS = ( LString ) {szMax, len, chars};
#ifdef TIDY
                size_t l = newS.len;
                char * end =  chars + l;
                for ( ; l < szMax; l++ ) *end++ = 0;
#endif
        }
        else
        {
                ST_ERR = chars == NULL ? ST_NULLPARAM_ERR : ST_OVERWRITE_ERR;
                newS = ( LString ) {0, 0, NULL};
        }
        return newS;
}
 
/* Copy of LString - checks that there is no buffer overflow */
size_t lstringcpy ( LString *dst, const LString *src )
{
        if ( dst == NULL || src == NULL || dst->str == NULL || src->str == NULL )
        {
                ST_ERR = dst == NULL || src == NULL ? ST_NULLPARAM_ERR : ST_NULLSTR_ERR;
                return ( size_t ) ST_ERR;
        }
        size_t len = src->len;
        if ( dst->sz <= len )
        {
                ST_ERR = ST_OVERWRITE_ERR;
                return ST_OVERWRITE_ERR;
        }
        dst->len = len;
        memmove ( dst->str, src->str, len + 1 );
#ifdef TIDY
        memset ( dst->str + len + 1, 0, dst->sz - len - 1 );
#endif
        return len;
}
 
/* Copy of a fixed character chain in a LString
 * checks that there is no buffer overflow
 */
size_t lstringchcpy ( LString *dst, const char *src )
{
        if ( dst == NULL || src == NULL )
        {
                ST_ERR = ST_NULLPARAM_ERR;
                return ST_NULLPARAM_ERR;
        }
        if ( dst->str == NULL )
        {
                ST_ERR = ST_NULLSTR_ERR;
                return ( size_t ) ST_ERR;
        }
        size_t len = strlen ( src );
        if ( dst->sz <= len )
        {
                ST_ERR = ST_OVERWRITE_ERR;
                return ST_OVERWRITE_ERR;
        }
        dst->len = len;
        memmove ( dst->str, src, len + 1 );
#ifdef TIDY
        memset ( dst->str + len + 1, 0, dst->sz - len - 1 );
#endif
        return len;
}
 
size_t lstringcat ( LString *dst, const LString *src )
{
        if ( dst == NULL || src == NULL || dst->str == NULL || src->str == NULL )
        {
                ST_ERR = dst == NULL || src == NULL ? ST_NULLPARAM_ERR : ST_NULLSTR_ERR;
                return ( size_t ) ST_ERR;
        }
        size_t oldlen = dst->len;
        size_t len = src->len;
        if ( oldlen == SIZE_MAX || len >= SIZE_MAX - oldlen )
        {
                ST_ERR = ST_OVERWRITE_ERR;
                return ( size_t ) ST_ERR;
        }
        size_t finallen = oldlen + len;
        if ( finallen >= dst->sz )
        {
                ST_ERR = ST_OVERWRITE_ERR;
                return ST_OVERWRITE_ERR;
        }
        memmove ( dst->str + oldlen, src->str, len + 1 );
        dst->len = finallen;
#ifdef TIDY
        size_t l = dst->len;
        char * end =  dst->str + l;
        for ( ; l < dst->sz; l++ ) *end++ = 0;
#endif
        return len;
}
 
/* Concatenation of a fixed character chain to a LString */
size_t lstringchcat ( LString *dst, const char *src )
{
        if ( dst == NULL || src == NULL )
        {
                ST_ERR = ST_NULLPARAM_ERR;
                return ST_NULLPARAM_ERR;
        }
        if ( dst->str == NULL )
        {
                ST_ERR = ST_NULLSTR_ERR;
                return ( size_t ) ST_ERR;
        }
        size_t oldlen = dst->len;
        size_t len = strlen ( src );
        if ( oldlen == SIZE_MAX || len >= SIZE_MAX - oldlen )
        {
                ST_ERR = ST_OVERWRITE_ERR;
                return ( size_t ) ST_ERR;
        }
        size_t finallen = oldlen + len;
        if ( finallen >= dst->sz )
        {
                ST_ERR = ST_OVERWRITE_ERR;
                return ST_OVERWRITE_ERR;
        }
        memmove ( dst->str + oldlen, src, len + 1 );
        dst->len = finallen;
#ifdef TIDY
        size_t l = dst->len;
        char * end =  dst->str + l;
        for ( ; l < dst->sz; l++ ) *end++ = 0;
#endif
        return len;
}
 
 
/* The following functions apply both on String and LString */
 
size_t stringlen ( const String *string )
{
        if ( string == NULL )
        {
                ST_ERR = ST_NULLPARAM_ERR;
                return ( size_t ) ST_ERR;
        }
        return string->len;
}
 
/*
    Check for an internal inconsistency in a String or LString.
    Returns 0 if consistent, or the ST_xxx_ERR code of the first problem.
    If file is not NULL, a diagnostic "file:line: ..." is also printed on
    stderr when a problem is found (see the STRINGCHECK macros).
    You can use this in a debugging session.
*/
static int checkfields ( const char *str, size_t sz, size_t len,
                         const char *file, int line )
{
        int err = 0;
        const char *what = NULL;
        if ( str == NULL )
        {
                err = ST_NULLSTR_ERR;
                what = "NULL buffer";
        }
        else if ( len >= sz )
        {
                err = ST_INCONSISTENTSZ_ERR;
                what = "length >= buffer size";
        }
        else if ( memchr ( str, '\0', sz ) != str + len )
        {
                err = ST_INCONSISTENTLEN_ERR;
                what = "length does not match the terminating NUL";
        }
        if ( err != 0 && file != NULL )
                fprintf ( stderr, "%s:%d: SPString inconsistency: %s "
                          "(sz=%zu, len=%zu)\n", file, line, what, sz, len );
        return err;
}

int stringcheck ( const String *string, const char *file, int line )
{
        if ( string == NULL )
        {
                if ( file != NULL )
                        fprintf ( stderr, "%s:%d: SPString inconsistency: "
                                  "NULL String\n", file, line );
                return ST_NULLPARAM_ERR;
        }
        return checkfields ( string->str, string->sz, string->len, file, line );
}

int lstringcheck ( const LString *string, const char *file, int line )
{
        if ( string == NULL )
        {
                if ( file != NULL )
                        fprintf ( stderr, "%s:%d: SPString inconsistency: "
                                  "NULL LString\n", file, line );
                return ST_NULLPARAM_ERR;
        }
        return checkfields ( string->str, string->sz, string->len, file, line );
}
 
/* Comparisons
   The result is always a valid ordering, usable e.g. with qsort:
   a NULL String (or NULL buffer) sorts before any string, and two NULLs
   are equal. The error is reported through get_stringerr(). */
int stringcmp ( const String *st1, const String *st2 )
{
        if ( st1 == NULL || st2 == NULL )
        {
                ST_ERR = ST_NULLPARAM_ERR;
                return nullorder ( st1, st2 );
        }
        if ( st1->str == NULL || st2->str == NULL )
        {
                ST_ERR = ST_NULLSTR_ERR;
                return nullorder ( st1->str, st2->str );
        }
        return strcmp ( st1->str, st2->str );
}
 
/* Comparison with a C chain */
int stringchcmp ( const String *st1, const char *st2 )
{
        if ( st1 == NULL || st2 == NULL )
        {
                ST_ERR = ST_NULLPARAM_ERR;
                return nullorder ( st1, st2 );
        }
        if ( st1->str == NULL )
        {
                ST_ERR = ST_NULLSTR_ERR;
                return -1;
        }
        return strcmp ( st1->str, st2 );
}

size_t lstringlen ( const LString *string )
{
        if ( string == NULL )
        {
                ST_ERR = ST_NULLPARAM_ERR;
                return ( size_t ) ST_ERR;
        }
        return string->len;
}

int lstringcmp ( const LString *st1, const LString *st2 )
{
        if ( st1 == NULL || st2 == NULL )
        {
                ST_ERR = ST_NULLPARAM_ERR;
                return nullorder ( st1, st2 );
        }
        if ( st1->str == NULL || st2->str == NULL )
        {
                ST_ERR = ST_NULLSTR_ERR;
                return nullorder ( st1->str, st2->str );
        }
        return strcmp ( st1->str, st2->str );
}

int lstringchcmp ( const LString *st1, const char *st2 )
{
        if ( st1 == NULL || st2 == NULL )
        {
                ST_ERR = ST_NULLPARAM_ERR;
                return nullorder ( st1, st2 );
        }
        if ( st1->str == NULL )
        {
                ST_ERR = ST_NULLSTR_ERR;
                return -1;
        }
        return strcmp ( st1->str, st2 );
}

/* Truncation to at most N characters */
String * stringtrunc ( String *string, size_t N )
{
        if ( string == NULL || string->str == NULL )
        {
                ST_ERR = string == NULL ? ST_NULLPARAM_ERR : ST_NULLSTR_ERR;
                return NULL;
        }
        if ( string->sz == 0 || string->len >= string->sz )
        {
                ST_ERR = ST_INCONSISTENTSZ_ERR;
                return NULL;
        }
        if ( N > string->len )
                N = string->len;
        string->len = N;
        string->str[N] = '\0';
#ifdef TIDY
        memset ( string->str + N + 1, 0, string->sz - N - 1 );
#endif
        return string;
}

LString * lstringtrunc ( LString *string, size_t N )
{
        if ( string == NULL || string->str == NULL )
        {
                ST_ERR = string == NULL ? ST_NULLPARAM_ERR : ST_NULLSTR_ERR;
                return NULL;
        }
        if ( string->sz == 0 || string->len >= string->sz )
        {
                ST_ERR = ST_INCONSISTENTSZ_ERR;
                return NULL;
        }
        if ( N > string->len )
                N = string->len;
        string->len = N;
        string->str[N] = '\0';
#ifdef TIDY
        memset ( string->str + N + 1, 0, string->sz - N - 1 );
#endif
        return string;
}


/* Formatting */
void stringprintf ( String *dst, const char * format, ... )
{
        if ( dst == NULL || format == NULL )
        {
                ST_ERR = ST_NULLPARAM_ERR;
                return;
        }
        if ( dst->str == NULL )
        {
                ST_ERR = ST_NULLSTR_ERR;
                return;
        }
        va_list argp, measure;
        va_start ( argp, format );
        va_copy ( measure, argp );
        int needed = vsnprintf ( NULL, 0, format, measure );
        va_end ( measure );
        if ( needed < 0 )
        {
                va_end ( argp );
                ST_ERR = ST_OVERWRITE_ERR;
                return;
        }
        /* Format into a fresh buffer: the arguments may point into dst->str,
           so writing in place (or reallocating it first) is not safe. */
        size_t newsz = dst->sz > ( size_t ) needed ? dst->sz : ( size_t ) needed + 1;
        char *tmp = malloc ( newsz );
        if ( tmp == NULL )
        {
                va_end ( argp );
                ST_ERR = ST_ALLOC_ERR;
                return;
        }
        int written = vsnprintf ( tmp, newsz, format, argp );
        va_end ( argp );
        if ( written < 0 || written > needed )
        {
                free ( tmp );
                ST_ERR = ST_OVERWRITE_ERR;
                return;
        }
#ifdef TIDY
        memset ( tmp + written + 1, 0, newsz - ( size_t ) written - 1 );
#endif
        free ( dst->str );
        dst->str = tmp;
        dst->sz = newsz;
        dst->len = ( size_t ) written;
}
 
/* Returns error type and resets ST_ERR */
int get_stringerr ( void )
{
        int err = ST_ERR;
        ST_ERR = ST_NO_ERR;
        return err;
}
 
/*************************************************
* PRIVATE
**************************************************/
 
/* Increase string.sz to desired size.
   Returns ST_NO_ERR on success, ST_ALLOC_ERR (and sets ST_ERR) on failure.
   A pending error in ST_ERR is left untouched on success. */
static int incsz ( String *string, size_t size )
{
        if ( string->sz < size )
        {
                /* double as necessary */
                size_t newsz = string->sz;
                if ( newsz == 0 )
                        newsz = 1;
                while ( newsz < size )
                {
                        if ( newsz > SIZE_MAX / 2 )
                        {
                                newsz = size;
                                break;
                        }
                        newsz *= 2;
                }

                char *tmp = realloc ( string->str, newsz );
                if ( tmp == NULL )
                {
                        ST_ERR = ST_ALLOC_ERR;
                        return ST_ALLOC_ERR;
                }
#ifdef TIDY
                memset ( tmp + string->len, 0, newsz - string->len );
#endif
                string->str = tmp;
                string->sz = newsz;
        }
        return ST_NO_ERR;
}

/* Ordering when at least one operand is NULL: NULL < non-NULL */
static int nullorder ( const void *a, const void *b )
{
        return ( a != NULL ) - ( b != NULL );
}

/* Does p point inside the allocated buffer of string ?
   Compared as integers: relational operators on unrelated pointers are
   undefined behaviour in C. */
static int inbuffer ( const String *string, const char *p )
{
        uintptr_t begin = ( uintptr_t ) string->str;
        uintptr_t q = ( uintptr_t ) p;
        return q >= begin && q - begin < string->sz;
}

#undef TIDY

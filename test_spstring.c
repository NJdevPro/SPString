#include <stdio.h>
#include <string.h>
#include "spstring.h"

static int failures;

#define CHECK(condition) do { \
        if (!(condition)) { \
                fprintf(stderr, "%s:%d: check failed: %s\n", \
                        __FILE__, __LINE__, #condition); \
                ++failures; \
        } \
} while (0)

static void check_string(const String *string, const char *expected)
{
        CHECK(string != NULL);
        if (string != NULL)
        {
                CHECK(string->str != NULL);
                if (string->str != NULL)
                {
                        CHECK(strcmp(string->str, expected) == 0);
                        CHECK(string->len == strlen(expected));
                        CHECK(string->sz > string->len);
                }
        }
}

static void test_dynamic_strings(void)
{
        String *source = newString("alpha");
        String *destination = newString("x");
        CHECK(source != NULL);
        CHECK(destination != NULL);
        if (source == NULL || destination == NULL)
                return;

        CHECK(source->len == 5 && source->sz == 6);
        CHECK(stringlen(source) == 5);
        CHECK(stringcpy(destination, source) == 5);
        check_string(destination, "alpha");
        CHECK(destination->sz >= 6);
        CHECK(stringcpy(destination, destination) == 5);

        CHECK(stringchcpy(destination, "a longer C string") == 17);
        check_string(destination, "a longer C string");
        CHECK(stringcat(destination, source) == 5);
        check_string(destination, "a longer C stringalpha");
        CHECK(stringchcat(destination, "!") == 1);
        check_string(destination, "a longer C stringalpha!");
        CHECK(stringcat(destination, destination) == destination->len / 2);
        check_string(destination, "a longer C stringalpha!a longer C stringalpha!");

        String *duplicate = stringdup(source);
        check_string(duplicate, "alpha");
        if (duplicate != NULL)
        {
                CHECK(duplicate != source);
                CHECK(duplicate->str != source->str);
                delString(duplicate);
        }

        CHECK(stringcmp(source, destination) > 0);
        String *z = newString("z");
        String *aa = newString("aa");
        CHECK(z != NULL && aa != NULL);
        if (z != NULL && aa != NULL)
                CHECK(stringcmp(z, aa) > 0);
        CHECK(stringchcmp(source, "alpha") == 0);
        CHECK(stringchcmp(source, "alph") > 0);
        delString(z);
        delString(aa);

        CHECK(stringtrunc(destination, 5) == destination);
        check_string(destination, "a lon");
        CHECK(stringtrunc(destination, 0) == destination);
        check_string(destination, "");
        CHECK(stringtrunc(destination, 100) == destination);
        check_string(destination, "");

        stringprintf(destination, "%s %d", "value", 42);
        check_string(destination, "value 42");
        CHECK(destination->sz >= destination->len + 1);

        delString(source);
        delString(destination);
}

static void test_local_strings(void)
{
        char clipped[12] = "longvalue";
        LString initialized = localString(clipped, 5);
        CHECK(initialized.sz == 5);
        CHECK(initialized.len == 4);
        CHECK(strcmp(initialized.str, "long") == 0);

        char empty_buffer[8] = "";
        char source_buffer[8] = "source";
        char destination_buffer[9] = "x";
        LString empty = localString(empty_buffer, sizeof empty_buffer);
        LString source = localString(source_buffer, sizeof source_buffer);
        LString destination = localString(destination_buffer, sizeof destination_buffer);

        CHECK(empty.len == 0);
        CHECK(lstringlen(&source) == 6);
        CHECK(lstringcmp(&source, &destination) < 0);
        CHECK(lstringchcmp(&source, "source") == 0);
        CHECK(lstringchcmp(&source, "source!") < 0);
        CHECK(lstringcpy(&destination, &source) == 6);
        CHECK(strcmp(destination.str, "source") == 0);
        CHECK(destination.len == 6);
        CHECK(lstringchcpy(&destination, "xy") == 2);
        CHECK(strcmp(destination.str, "xy") == 0);
        CHECK(lstringcat(&destination, &source) == 6);
        CHECK(strcmp(destination.str, "xysource") == 0);
        CHECK(destination.len == 8);

        CHECK(lstringchcat(&destination, "!") == (size_t) ST_OVERWRITE_ERR);
        CHECK(strcmp(destination.str, "xysource") == 0);
        CHECK(destination.len == 8);

        char small_buffer[4] = "a";
        char too_long_buffer[8] = "long";
        LString small = localString(small_buffer, sizeof small_buffer);
        LString too_long = localString(too_long_buffer, sizeof too_long_buffer);
        CHECK(lstringcpy(&small, &too_long) == (size_t) ST_OVERWRITE_ERR);
        CHECK(strcmp(small.str, "a") == 0);
        CHECK(lstringchcpy(&small, "long") == (size_t) ST_OVERWRITE_ERR);
        CHECK(strcmp(small.str, "a") == 0);
        CHECK(lstringcat(&small, &too_long) == (size_t) ST_OVERWRITE_ERR);
        CHECK(strcmp(small.str, "a") == 0);
        CHECK(lstringchcat(&small, "long") == (size_t) ST_OVERWRITE_ERR);
        CHECK(strcmp(small.str, "a") == 0);

        CHECK(lstringtrunc(&destination, 3) == &destination);
        CHECK(strcmp(destination.str, "xys") == 0);
        CHECK(destination.len == 3);
        CHECK(lstringtrunc(&destination, 0) == &destination);
        CHECK(strcmp(destination.str, "") == 0);

        LString invalid = localString(empty_buffer, 0);
        CHECK(invalid.str == NULL);
}

static void test_errors_and_validation(void)
{
        CHECK(newString(NULL) == NULL);
        CHECK(get_stringerr() == ST_NULLPARAM_ERR);
        CHECK(get_stringerr() == ST_NO_ERR);

        delString(NULL);
        CHECK(get_stringerr() == ST_NULLPARAM_ERR);

        CHECK(stringlen(NULL) == (size_t) ST_NULLPARAM_ERR);
        CHECK(get_stringerr() == ST_NULLPARAM_ERR);
        CHECK(stringcpy(NULL, NULL) == (size_t) ST_NULLPARAM_ERR);
        CHECK(get_stringerr() == ST_NULLPARAM_ERR);
        CHECK(stringchcpy(NULL, "x") == (size_t) ST_NULLPARAM_ERR);
        CHECK(get_stringerr() == ST_NULLPARAM_ERR);
        CHECK(stringcat(NULL, NULL) == (size_t) ST_NULLPARAM_ERR);
        CHECK(get_stringerr() == ST_NULLPARAM_ERR);
        CHECK(stringchcat(NULL, "x") == (size_t) ST_NULLPARAM_ERR);
        CHECK(get_stringerr() == ST_NULLPARAM_ERR);
        CHECK(stringdup(NULL) == NULL);
        CHECK(get_stringerr() == ST_NULLPARAM_ERR);
        CHECK(stringcmp(NULL, NULL) == ST_NULLPARAM_ERR);
        CHECK(get_stringerr() == ST_NULLPARAM_ERR);
        CHECK(stringchcmp(NULL, "x") == ST_NULLPARAM_ERR);
        CHECK(get_stringerr() == ST_NULLPARAM_ERR);
        CHECK(stringtrunc(NULL, 0) == NULL);
        CHECK(get_stringerr() == ST_NULLPARAM_ERR);
        CHECK(lstringlen(NULL) == (size_t) ST_NULLPARAM_ERR);
        CHECK(get_stringerr() == ST_NULLPARAM_ERR);
        CHECK(lstringcmp(NULL, NULL) == ST_NULLPARAM_ERR);
        CHECK(get_stringerr() == ST_NULLPARAM_ERR);
        CHECK(lstringchcmp(NULL, "x") == ST_NULLPARAM_ERR);
        CHECK(get_stringerr() == ST_NULLPARAM_ERR);
        CHECK(lstringtrunc(NULL, 0) == NULL);
        CHECK(get_stringerr() == ST_NULLPARAM_ERR);
        stringprintf(NULL, "%s", "x");
        CHECK(get_stringerr() == ST_NULLPARAM_ERR);
        String *format_target = newString("");
        stringprintf(format_target, NULL);
        CHECK(get_stringerr() == ST_NULLPARAM_ERR);
        delString(format_target);

        char buffer[] = "ok";
        String valid = { sizeof buffer, 2, buffer };
        CHECK(stringcheck(&valid, __FILE__, __LINE__) == 0);
        CHECK(stringcheck(NULL, __FILE__, __LINE__) == ST_NULLPARAM_ERR);
        String no_buffer = { 0, 0, NULL };
        CHECK(stringcheck(&no_buffer, __FILE__, __LINE__) == ST_NULLSTR_ERR);
        String bad_size = { 2, 2, buffer };
        CHECK(stringcheck(&bad_size, __FILE__, __LINE__) == ST_INCONSISTENTSZ_ERR);
        String bad_length = { sizeof buffer, 1, buffer };
        CHECK(stringcheck(&bad_length, __FILE__, __LINE__) == ST_INCONSISTENTLEN_ERR);
        CHECK(stringtrunc(&bad_size, 0) == NULL);
        CHECK(get_stringerr() == ST_INCONSISTENTSZ_ERR);
        LString bad_local_size = { 1, 1, buffer };
        CHECK(lstringtrunc(&bad_local_size, 0) == NULL);
        CHECK(get_stringerr() == ST_INCONSISTENTSZ_ERR);

        LString null_local = localString(NULL, 3);
        CHECK(null_local.str == NULL);
        CHECK(get_stringerr() == ST_NULLPARAM_ERR);
}

static void test_aliasing(void)
{
        /* Source pointing inside the destination buffer, with reallocation */
        String *s = newString("abcdef");
        CHECK(s != NULL);
        if (s == NULL)
                return;
        CHECK(stringchcat(s, s->str) == 6);
        check_string(s, "abcdefabcdef");
        CHECK(stringchcat(s, s->str + 6) == 6);
        check_string(s, "abcdefabcdefabcdef");
        CHECK(stringchcpy(s, s->str + 12) == 6);
        check_string(s, "abcdef");

        stringprintf(s, "%s-%s-%s", s->str, s->str, s->str);
        check_string(s, "abcdef-abcdef-abcdef");
        stringprintf(s, "%.3s", s->str + 7);
        check_string(s, "abc");
        delString(s);
}

static void test_error_preserved_by_growth(void)
{
        CHECK(newString(NULL) == NULL);
        String *s = newString("a");
        CHECK(s != NULL);
        if (s != NULL)
        {
                CHECK(stringchcat(s, "bcdefgh") == 7);
                check_string(s, "abcdefgh");
                stringprintf(s, "%s%s", s->str, s->str);
                check_string(s, "abcdefghabcdefgh");
                delString(s);
        }
        CHECK(get_stringerr() == ST_NULLPARAM_ERR);
        CHECK(get_stringerr() == ST_NO_ERR);
}

int main(void)
{
        test_dynamic_strings();
        test_local_strings();
        test_errors_and_validation();
        test_aliasing();
        test_error_preserved_by_growth();

        if (failures != 0)
        {
                fprintf(stderr, "%d test(s) failed\n", failures);
                return 1;
        }
        puts("All SPString tests passed.");
        return 0;
}

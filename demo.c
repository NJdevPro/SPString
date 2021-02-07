#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include "spstring.h"

int main()
{
  String* s0 = newString("This string is very long.");
  String* s1 = newString("Shorter string.");
  printf("[%s] has length %ld and buffer size %ld\n", s0->str, s0->len, s0->sz);
  printf("[%s] has length %ld and buffer size %ld\n", s1->str, s1->len, s1->sz);
  
  stringcat(s0, s1);
  printf("After concatenation, ");
  printf("[%s] has length %ld and buffer size %ld\n", s0->str, s0->len, s0->sz);
  
  String *s2 = stringdup(s1);
  printf("Duplicate of [%s] has length %ld and buffer size %ld\n", s2->str, s2->len, s2->sz);
  
  stringtrunc(s0, 12);
  printf("[%s] has length %ld and buffer size %ld\n", s0->str, s0->len, s0->sz);
  delString(s0);
  delString(s1);
  delString(s2);

  // Allocation on the stack
  char t0[50] = "This LString is very long.";
  char t1[30] = "Shorter LString.";
  LString ls0 = localString(t0, 50);
  LString ls1 = localString(t1, 30);
  
  lstringcat(&ls1, &ls0);
  
  printf("[%s] has length %ld\n", ls0.str, ls0.len);
  printf("[%s] has length %ld\n", ls1.str, ls1.len);
  
  LString ls2 = lstringdup(&ls0);
  printf("[%s] has length %ld and buffer size %ld\n", ls2.str, ls2.len, ls2.sz);
  lstringchcat(&ls2, " This is a C chain.");
  printf("[%s] has length %ld and buffer size %ld\n", ls2.str, ls2.len, ls2.sz);
  
  printf(stringchcmp(&ls1, t1) == 0 ? "The chains are identical" : "The chains are different");
  return 0;
}

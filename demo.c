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
  printf("s0 = [%s] has length %ld and buffer size %ld\n", s0->str, s0->len, s0->sz);
  printf("s1 = [%s] has length %ld and buffer size %ld\n", s1->str, s1->len, s1->sz);
  
  stringcat(s0, s1);
  printf("After concatenation of s1 into s0, ");
  printf("s0 = [%s] has length %ld and buffer size %ld\n", s0->str, s0->len, s0->sz);
    
  stringtrunc(s0, 12);
  printf("Truncated s0 = [%s] has length %ld and buffer size %ld\n", s0->str, s0->len, s0->sz);
    
  String *s2 = stringdup(s1);
  printf("Duplicate s2 = [%s] of s1 has length %ld and buffer size %ld\n", s2->str, s2->len, s2->sz);
  
  stringchcat(s0, ".C null terminated string.");
  printf("After concatenation of a C string, s0 = [%s] has length %ld and buffer size %ld\n", s0->str, s0->len, s0->sz);
    
  stringcpy(s2, s0);
  printf("Copy of s0, s2 = [%s] has length %ld and buffer size %ld\n\n", s2->str, s2->len, s2->sz);
    
  delString(s0);
  delString(s1);
  delString(s2);

  // Allocation on the stack
  // ========================
  static char t0[50] = "This LString is very long.";
  static char t1[30] = "Shorter LString.";
  static char t2[40] = "";
  LString l0 = localString(t0, 50);
  LString l1 = localString(t1, 30);
  printf("l0 = [%s] has length %ld and buffer size %ld\n", l0.str, l0.len, l0.sz);
  printf("l1 = [%s] has length %ld and buffer size %ld\n", l1.str, l1.len, l1.sz);
  
  lstringcat(&l1, &l0);
  printf("After concatenation of l1 into l0, ");
  printf("l0 = [%s] has length %ld and buffer size %ld\n", l0.str, l0.len, l0.sz);
  
  stringtrunc((String*)&l0, 13);
  printf("Truncated l0 = [%s] has length %ld and buffer size %ld\n", l0.str, l0.len, l0.sz);
  
  LString l2 = localString(t2, 40); // remember that localString("", 40) does NOT work !
  lstringcpy(&l2, &l0);
  printf("Copy of l0, l2 = [%s] has length %ld and buffer size %ld\n", l2.str, l2.len, l2.sz);
  
  lstringchcat(&l0, ". This is a C chain.");
  printf("After concatenation of a C chain, l0 = [%s] has length %ld and buffer size %ld\n", l0.str, l0.len, l0.sz);
  
  printf(stringchcmp((String*)&l1, t1) == 0 ? "The chains are identical" : "The chains are different");
  return 0;
}

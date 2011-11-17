/*
   some helper functions
   @rockuw
Sun Oct 16 21:05:55 CST 2011
*/

#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

// print msg in hex form
// arg: buffer, buffer length
// ret: void
void print_msg(uint8_t *buffer, int len, const char *msg);

#endif

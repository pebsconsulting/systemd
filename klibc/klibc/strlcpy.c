/*
 * strlcpy.c
 */

#include <string.h>
#include <klibc/compiler.h>

size_t strlcpy(char *dst, const char *src, size_t size)
{
  size_t bytes = 0;
  char *q = dst;
  const char *p = src;
  char ch;

  while ( (ch = *p++) ) {
    if ( bytes+1 < size )
      *q++ = ch;

    bytes++;
  }

  if (size)
    *q = '\0';
  return bytes;
}



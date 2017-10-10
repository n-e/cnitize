#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cnitize.h"

#define DESTLEN 1024

int main(int argc, char const *argv[]) {
  char * src = NULL;
  size_t srccap = 0;
  ssize_t srclen = 0;

  while((srclen = getline(&src, &srccap, stdin))>0) {
    char dest[DESTLEN];
    if (sanitize(src,dest,DESTLEN) == 0)
      printf("%s",dest);
    else
      fprintf(stderr,"Error: output bigger than DESTLEN");
  }

  free(src);

  return 0;
}

/* INTERNAL FUNCTIONS */

void parse(const char * restrict src, char * restrict attrs) {
  size_t len = strlen(src);

  for (size_t i = 0; i < len; i++) {
    attrs = 0;
  }
}

void compact(char * restrict src, char * restrict attrs)
{
  // TODO IMPLEMENT
}

int tohtml(
  const char * restrict src,
  const char * restrict attrs,
  char * restrict dst,
  size_t dstsize)
{
  *dst = 0;
  return 0;
}

int sanitize(const char * restrict src, char * restrict dst, size_t dstsize)
{
  size_t srclen = strlen(src);
  char *src2 = malloc(srclen+1);
  char *attrs = malloc(srclen+1);

  strcpy(src2,src);
  parse(src2,attrs);
  compact(src2,attrs);
  int ret = tohtml(src2,attrs,dst,dstsize);

  free(src2);
  free(attrs);

  return ret;
}

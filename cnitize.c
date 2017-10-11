#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cnitize.h"

#define DESTLEN 1024

int main(int argc, char const *argv[]) {
  char * src = NULL;
  size_t srccap = 0;
  ssize_t srclen = 0;

  /* Use line buffering even when not in interactive mode so the sanitized
  post is returned instantly */
  setvbuf(stdout, NULL, _IOLBF, BUFSIZ);

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
  char mask = 0;

  for (size_t i = 0; i < len; i++) {
    char tag;

    if ((tag = starts_with_otag(src+i))) {
      attrs[i]   = A_TAG; // TODO: fix when we have 2-character tags
      attrs[i+1] = A_TAG;
      attrs[i+2] = A_TAG;
      i+=2;
      mask |= tag;
    }
    else if ((tag = starts_with_ctag(src+i))) {
      attrs[i]   = A_TAG;
      attrs[i+1] = A_TAG;
      attrs[i+2] = A_TAG;
      attrs[i+3] = A_TAG;
      i+=3;
      mask &= ~tag;
    }
    else {
      attrs[i] = mask;
    }
  }
}

void compact(char * restrict src, char * restrict attrs)
{
  size_t inpos = 0, outpos = 0;
  while(src[inpos] != 0) {
    if (attrs[inpos] == A_TAG)
      inpos++;
    else {
      attrs[outpos] = attrs[inpos];
      src[outpos] = src[inpos];
      inpos++;
      outpos++;
    }
  }
  src[outpos] = 0;
}

int tohtml(
  const char * restrict src,
  const char * restrict attrs,
  char * restrict dst,
  size_t dstsize)
{
  size_t srcpos = 0, dstpos = 0;

  while(dstpos < dstsize-1) { // TODO : check if off by one
    if(
      (srcpos == 0 && attrs[srcpos] == A_UNDERLINE) ||
      (srcpos != 0 && attrs[srcpos] == A_UNDERLINE && attrs[srcpos-1] != A_UNDERLINE)
  ) {
      strncpy(dst+dstpos,"<u>",dstsize - dstpos-1); // TODO : not efficient
      dstpos+=3;
    }
    else if(
      (src[srcpos] == 0 && attrs[srcpos-1] == A_UNDERLINE) ||
      (src[srcpos] != 0 && attrs[srcpos-1] == A_UNDERLINE && attrs[srcpos] != A_UNDERLINE)
  ) {
      strncpy(dst+dstpos,"</u>",dstsize - dstpos-1); // TODO : not efficient
      dstpos+=4;
    }
    dst[dstpos] = src[srcpos];
    dstpos++;
    srcpos++;
    if(!src[srcpos]) break;
  }

  dst[dstpos] = 0;

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

/* UTILITY FUNCTIONS */

// TODO : doesn't work yet for two-character tags
char starts_with_otag(const char * str)
{
  if(str[0] == 0 || str[1] == 0 || str[2] == 0 || str[3] == 0)
    return 0;

  char mask1[]="<\0>\0";

  if(((*(int32_t*)str) & (*(int32_t*) mask1)) == (*(int32_t*) mask1)) {
    const char *tag = str+1;

    char tag_idx = 0;
    for (size_t i = 0; i < A_TAGSSZ; i++) {
      if (*tag == a_tags[i][0])
        tag_idx = i;
    }

    return 1 << tag_idx;
  }

  return 0;
}

// TODO :factorize with starts_with_otag
char starts_with_ctag(const char * str)
{
  if(str[0] == 0 || str[1] == 0 || str[2] == 0 || str[3] == 0)
    return 0;

  char mask1[]="</\0>";

  if(((*(int32_t*)str) & (*(int32_t*) mask1)) == (*(int32_t*) mask1)) {
    const char *tag = str+2;

    char tag_idx = 0;
    for (size_t i = 0; i < A_TAGSSZ; i++) {
      if (*tag == a_tags[i][0])
        tag_idx = i;
    }

    return 1 << tag_idx;
  }

  return 0;
}

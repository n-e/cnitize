#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cnitize.h"

#define DESTLEN 1024

int main(int argc, char const *argv[]) {
  char * src = NULL;
  size_t srccap = 0;
  ssize_t srclen = 0;

  char *attrs = malloc(1024);
  size_t attrlen = 1024;

  while((srclen = getline(&src, &srccap, stdin))>0) {
    char dest[DESTLEN];
    if (srclen > 0)
      src[--srclen] = 0; // strip the \n at the end before passing it to sanitize

    if (srclen > attrlen) {
      attrs = realloc(attrs, srclen);
      attrlen = srclen;
    }
    if (sanitize_fast(src,dest,srclen-1,DESTLEN,attrs) == 0)
      printf("%s\n",dest);
    else
      fprintf(stderr,"Error: output bigger than DESTLEN");
  }

  free(src);
  free(attrs);

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
      if (!strncmp(src+i,"http://",7))
        mask |= A_URL;
      if (src[i] == ' '||src[i] == ','||src[i] == ')'||src[i] == '['||src[i] == '"')
        mask &= ~A_URL;

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

// TODO improve this macro
#define SAFECOPYTODST(str) stpncpy(dst+dstpos, str, ((int)dstsize-(int)dstpos-1 < 0)?0:(dstsize-dstpos))
int tohtml(
  const char * restrict src,
  const char * restrict attrs,
  char * restrict dst,
  size_t dstsize)
{
  size_t srcpos = 0, dstpos = 0;

  while(src[srcpos] && dstpos < dstsize-1) { // TODO : check if off by one


    /* 1) Add the start tags if necessary per the attributes */
    // TODO : group with the closing tags code block
    char prev_attrs = (srcpos == 0) ? 0 : attrs[srcpos-1];
    char cur_attrs = attrs[srcpos];

    char chged_attrs = prev_attrs ^ cur_attrs;
    char smallest_changed_attr = -1;
    for (size_t i = 0; i < A_TAGSSZ; i++) {
      char attr = 1 << i;
      if ((attr|chged_attrs) == chged_attrs ) { smallest_changed_attr = attr; break;}
    }

    for (size_t i = 0; i < A_TAGSSZ; i++) {
      if (
        (!((prev_attrs>>i)&1) && ((cur_attrs>>i)&1)) || // what should be opened
        (((prev_attrs>>i)&1) && (1<<i)>smallest_changed_attr && smallest_changed_attr > 0) // what was closed to be "safe"
      ) {
        SAFECOPYTODST("<"); dstpos++;
        SAFECOPYTODST(a_tags[i]); dstpos++;
        SAFECOPYTODST(">"); dstpos++;
      }
    }

    /* 2) add the url opening and closing tags */
    if (!(prev_attrs&A_URL) && (cur_attrs&A_URL)) {
      SAFECOPYTODST("<a href=\""); dstpos += 9;
    }


    /* 3) copy the characters, applying formatting if needed */
    if ((src[srcpos]>0 && src[srcpos]<32) || src[srcpos]==127)
      SAFECOPYTODST(" ");
    else if (src[srcpos] == '&') {
      SAFECOPYTODST("&amp;");
      dstpos += 4; // +1 done after the loop
    }
    else if (src[srcpos] == '<') {
      SAFECOPYTODST("&lt;");
      dstpos += 3; // +1 done after the loop
    }
    else if (src[srcpos] == '>') {
      SAFECOPYTODST("&gt;");
      dstpos += 3; // +1 done after the loop
    }
    /* Unnecessary since we exclude quotes from url attributes */
    /*else if (src[srcpos] == '"' && (cur_attrs&A_URL)) {
      SAFECOPYTODST("&quot;");
      dstpos += 5; // +1 done after the loop
    } */
    else if(dstpos<dstsize-2) {
      dst[dstpos] = src[srcpos];
    }

    dstpos++;
    srcpos++;


    /* 4) Add the closing tags if necessary per the attributes */
    prev_attrs = attrs[srcpos-1];
    cur_attrs = src[srcpos] ? attrs[srcpos] : 0;

    // this is moved here to work if the input ends with the url end
    if ((prev_attrs&A_URL) && !(cur_attrs&A_URL)) {
      SAFECOPYTODST("\">[url]</a>"); dstpos += 11;
    }

     chged_attrs = prev_attrs ^ cur_attrs;
     smallest_changed_attr = -1;
    for (size_t i = 0; i < A_TAGSSZ; i++) {
      char attr = 1 << i;
      if ((attr|chged_attrs) == chged_attrs ) { smallest_changed_attr = attr; break;}
    }

    for (int i = A_TAGSSZ - 1; i >= 0; i--) {
      char was_closed = 0;
      if (
        ((prev_attrs>>i)&1 && !((cur_attrs>>i)&1)) || // Attr goes from set to unset
        (((prev_attrs>>i)&1) && (1<<i)>smallest_changed_attr && smallest_changed_attr > 0)// Attr was set and comes before the last switching attr
      ) { // TODO : there is a buffer overrun somewhere if the condition is always false
        SAFECOPYTODST("</"); dstpos+=2;
        SAFECOPYTODST(a_tags[i]); dstpos++;
        SAFECOPYTODST(">"); dstpos++;
        was_closed = 1;
      }
    }
  }

  SAFECOPYTODST("\0");
  dst[dstsize-1] = 0; /* The last 0 might not be copied if we are too far ahead */

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


int sanitize_fast(char * restrict src, char * restrict dst, size_t srcsize, size_t dstsize, char * restrict attrs)
{
  parse(src,attrs);
  compact(src,attrs);
  int ret = tohtml(src,attrs,dst,dstsize);

  return ret;
}


/* UTILITY FUNCTIONS */

// TODO : doesn't work yet for two-character tags
char starts_with_otag(const char * str)
{
  if(str[0] == 0 || str[1] == 0 || str[2] == 0 || str[3] == 0)
    return 0;

  char mask1[]="<\0>\0"; // TODO : check there is no bug if we only use the comparison expr as a mask

  if(((*(int32_t*)str) & (*(int32_t*) mask1)) == (*(int32_t*) mask1)) {
    const char *tag = str+1;

    char tag_idx = -1;
    for (size_t i = 0; i < A_TAGSSZ; i++) {
      if (*tag == a_tags[i][0])
        tag_idx = i;
    }

    return tag_idx == -1 ? 0 : (1 << tag_idx);
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

    char tag_idx = -1;
    for (size_t i = 0; i < A_TAGSSZ; i++) {
      if (*tag == a_tags[i][0])
        tag_idx = i;
    }

    return tag_idx == -1 ? 0 : (1 << tag_idx);
  }

  return 0;
}

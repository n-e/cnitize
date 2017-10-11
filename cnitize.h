/* INTERNAL FUNCTIONS */

void parse(const char * restrict src, char * restrict attrs);
void compact(char * restrict src, char * restrict attrs);
int tohtml(
  const char * restrict src,
  const char * restrict attrs,
  char * restrict dst,
  size_t dstsize);

/* UTILITY FUNCTIONS */

/* Checks if the string starts with an opening html tag. Allowed tags are
the ones defined below.

@returns the bitmask of the tag on success, 0 on failure.
*/
char starts_with_otag(const char * str);
char starts_with_ctag(const char * str);


/* PUBLIC INTERFACE */

/* Attributes */
#define A_BOLD 1
#define A_ITALIC 2
#define A_UNDERLINE 4
#define A_STRIKE 8
#define A_TAG 16

#define A_TAGSSZ 4
const char *a_tags[] = {"b","i","u","tt"};
  /* Note: tags are sorted in ascending bitmask value */

/** Sanitize a user-provided string with basic formatting (b, i, u, s tags)
 so it can be inserted in an HTML document without (hopefully) any security
 risks nor breaking the document layout or formatting.

 The destination string can also be used in TSV and XML documents in the same way.

 @src the null-terminated source string. The character encoding must be either
 us-ascii or a superset of us-ascii (utf8, iso8859-1...). The source string
 must be properly character encoded but is otherwise assumed to be adversarial.

 @dst the buffer for the sanitized string.

 @dstsize the size of the output buffer.

 @return 0 success
 @return -1 the output was too big to fit in the output buffer and should be
 discarded as it may contain unmatched tags.
*/
int sanitize(const char * restrict src, char * restrict dst, size_t dstsize);

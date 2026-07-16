/*****************************************************************************
 *
 * pc_stringbuffer_shim.c
 *   The stringbuffer symbols pgPointCloud needs but PostGIS does not export.
 *
 * NOTE (# SOURCE-GAP-ACK): these are internal vendored pgPointCloud utility
 * symbols, not public MEOS API — this file resolves a static-link symbol
 * collision at the MEOS build level; it does not filter any binding surface.
 *
 * pgPointCloud's lib/stringbuffer.c and PostGIS's liblwgeom/stringbuffer.c
 * descend from the same upstream utility tree and define the same stringbuffer
 * symbols over an identical stringbuffer_t layout.  Linking both static objects
 * makes those symbols duplicate — a clash only GNU ld tolerates (via
 * --allow-multiple-definition); Apple ld64 rejects it, breaking every -DALL
 * build on macOS.  MobilityDB therefore omits libpc's stringbuffer.c from the
 * link (meos/src/pointcloud/CMakeLists.txt) and lets PostGIS's copy serve the
 * shared symbols — which is exactly the definition GNU ld already selects by
 * link order.
 *
 * The overlap is only PARTIAL, however: two symbols pgPointCloud references are
 * NOT exported by PostGIS — stringbuffer_release_string (PostGIS has no such
 * function) and stringbuffer_append (PostGIS provides it only as a static
 * inline in its header, so it is not a linkable symbol).  Both are defined here,
 * verbatim from upstream pgPointCloud, so the two libraries together contribute
 * exactly one definition of every stringbuffer symbol and no duplicate-
 * tolerating linker flag is needed on any platform.  The bodies touch only the
 * stringbuffer_t fields, whose layout PostGIS shares, so operating on a buffer
 * produced by PostGIS's shared functions is safe.
 *
 *****************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "stringbuffer.h"

/**
 * If necessary, expand the stringbuffer_t internal buffer to accommodate the
 * specified additional size.  (Static in upstream stringbuffer.c; reproduced
 * here because stringbuffer_append below needs it.)
 */
static inline void
stringbuffer_makeroom(stringbuffer_t *s, size_t size_to_add)
{
  size_t current_size = (s->str_end - s->str_start);
  size_t capacity = s->capacity;
  size_t required_size = current_size + size_to_add;

  if (!capacity)
    capacity = STRINGBUFFER_STARTSIZE;
  else
    while (capacity < required_size)
      capacity *= 2;

  if (capacity > s->capacity)
  {
    s->str_start = realloc(s->str_start, capacity);
    s->capacity = capacity;
    s->str_end = s->str_start + current_size;
  }
}

/**
 * Append the specified string to the stringbuffer_t.  (Parameter names match
 * the stringbuffer.h declaration so static analysis does not read them as a
 * swapped argument order.)
 */
void
stringbuffer_append(stringbuffer_t *sb, const char *s)
{
  int alen = strlen(s); /* Length of string to append */
  int alen0 = alen + 1; /* Length including null terminator */
  stringbuffer_makeroom(sb, alen0);
  memcpy(sb->str_end, s, alen0);
  sb->str_end += alen;
}

/**
 * Transfer ownership of the internal string to the caller, turning this buffer
 * into an empty one.
 */
char *
stringbuffer_release_string(stringbuffer_t *sb)
{
  char *ret = sb->str_start;
  sb->str_start = sb->str_end = NULL;
  sb->capacity = 0;
  return ret;
}

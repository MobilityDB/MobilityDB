/***********************************************************************
 * pc_mem.c
 *
 *  Memory and message management routines.
 *  Allow this library to be used both inside and outside a
 *  PgSQL backend.
 *
 *  PgSQL Pointcloud is free and open source software provided
 *  by the Government of Canada
 *  Copyright (c) 2013 Natural Resources Canada
 *
 ***********************************************************************/

#include "pc_api_internal.h"
#include <stdarg.h>

struct pc_context_t
{
  pc_allocator alloc;
  pc_reallocator realloc;
  pc_deallocator free;
  pc_message_handler err;
  pc_message_handler warn;
  pc_message_handler info;
};

static struct pc_context_t pc_context;

/*
 * Default allocators
 *
 * We include some default allocators that use malloc/free/realloc
 * along with stdout/stderr since this is the most common use case
 *
 */

static void *default_allocator(size_t size) { return malloc(size); }

static void default_freeor(void *mem) { free(mem); }

static void *default_reallocator(void *mem, size_t size)
{
  return realloc(mem, size);
}

static void default_msg_handler(const char *label, const char *fmt, va_list ap)
{
  char newfmt[1024] = {0};
  snprintf(newfmt, 1024, "%s%s\n", label, fmt);
  newfmt[1023] = '\0';
  vprintf(newfmt, ap);
}

static void default_info_handler(const char *fmt, va_list ap)
{
  default_msg_handler("INFO: ", fmt, ap);
}

static void default_warn_handler(const char *fmt, va_list ap)
{
  default_msg_handler("WARNING: ", fmt, ap);
}

static void default_error_handler(const char *fmt, va_list ap)
{
  default_msg_handler("ERROR: ", fmt, ap);
  va_end(ap);
  exit(1);
}

void pc_install_default_handlers(void)
{
  pc_context.alloc = default_allocator;
  pc_context.realloc = default_reallocator;
  pc_context.free = default_freeor;
  pc_context.err = default_error_handler;
  pc_context.info = default_info_handler;
  pc_context.warn = default_warn_handler;
}

void pc_set_handlers(pc_allocator allocator, pc_reallocator reallocator,
                     pc_deallocator deallocator,
                     pc_message_handler error_handler,
                     pc_message_handler info_handler,
                     pc_message_handler warn_handler)
{
  if (!allocator)
    allocator = pc_context.alloc;
  if (!reallocator)
    reallocator = pc_context.realloc;
  if (!deallocator)
    deallocator = pc_context.free;
  if (!error_handler)
    error_handler = pc_context.err;
  if (!warn_handler)
    warn_handler = pc_context.warn;
  if (!info_handler)
    info_handler = pc_context.info;

  pc_context.alloc = allocator;
  pc_context.realloc = reallocator;
  pc_context.free = deallocator;
  pc_context.err = error_handler;
  pc_context.warn = warn_handler;
  pc_context.info = info_handler;
  return;
}

void *pcalloc(size_t size)
{
  void *mem;
  if (!size)
    return NULL;
  mem = pc_context.alloc(size);
  memset(mem, 0, size); /* Always clean memory */
  return mem;
}

char *pcstrdup(const char *str)
{
  size_t len = strlen(str);
  char *newstr = pcalloc(len + 1);
  memcpy(newstr, str, len + 1);
  return newstr;
}

void *pcrealloc(void *mem, size_t size)
{
  return pc_context.realloc(mem, size);
}

void pcfree(void *mem) { pc_context.free(mem); }

void pcerror(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  (*pc_context.err)(fmt, ap);
  va_end(ap);
}

void pcinfo(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  (*pc_context.info)(fmt, ap);
  va_end(ap);
}

void pcwarn(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  (*pc_context.warn)(fmt, ap);
  va_end(ap);
}

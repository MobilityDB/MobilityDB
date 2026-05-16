/*****************************************************************************
 *
 * Vendored verbatim from the Apache Arrow project: the Arrow C Data
 * Interface ABI specification.
 *
 * Source: https://github.com/apache/arrow  (format/abi.h)
 * Licensed to the Apache Software Foundation (ASF) under the
 * Apache License, Version 2.0.
 *
 * This is a stable two-struct ABI (ArrowSchema / ArrowArray plus release
 * callbacks). It is reproduced here so that MEOS can produce and consume
 * Arrow columnar data without linking the Apache Arrow C++ library. No
 * first-party MEOS code belongs in this file; it is third-party and is
 * reviewed by provenance against the upstream specification.
 *
 *****************************************************************************/

#ifndef MEOS_ARROW_C_DATA_INTERFACE_H
#define MEOS_ARROW_C_DATA_INTERFACE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ARROW_FLAG_DICTIONARY_ORDERED 1
#define ARROW_FLAG_NULLABLE           2
#define ARROW_FLAG_MAP_KEYS_SORTED    4

struct ArrowSchema
{
  /* Array type description */
  const char *format;
  const char *name;
  const char *metadata;
  int64_t flags;
  int64_t n_children;
  struct ArrowSchema **children;
  struct ArrowSchema *dictionary;

  /* Release callback */
  void (*release)(struct ArrowSchema *);
  /* Opaque producer-specific data */
  void *private_data;
};

struct ArrowArray
{
  /* Array data description */
  int64_t length;
  int64_t null_count;
  int64_t offset;
  int64_t n_buffers;
  int64_t n_children;
  const void **buffers;
  struct ArrowArray **children;
  struct ArrowArray *dictionary;

  /* Release callback */
  void (*release)(struct ArrowArray *);
  /* Opaque producer-specific data */
  void *private_data;
};

#ifdef __cplusplus
}
#endif

#endif /* MEOS_ARROW_C_DATA_INTERFACE_H */

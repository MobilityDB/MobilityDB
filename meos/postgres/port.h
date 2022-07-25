/*-------------------------------------------------------------------------
 *
 * port.h
 *	  Header for src/port/ compatibility functions.
 *
 * Portions Copyright (c) 1996-2021, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/port.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef PG_PORT_H
#define PG_PORT_H

#include <ctype.h>

/* Portable SQL-like case-independent comparisons and conversions */
extern int	pg_strcasecmp(const char *s1, const char *s2);
extern int	pg_strncasecmp(const char *s1, const char *s2, size_t n);
extern unsigned char pg_toupper(unsigned char ch);
extern unsigned char pg_tolower(unsigned char ch);
extern unsigned char pg_ascii_toupper(unsigned char ch);
extern unsigned char pg_ascii_tolower(unsigned char ch);


extern void pg_qsort(void *base, size_t nel, size_t elsize,
					 int (*cmp) (const void *, const void *));
extern int	pg_qsort_strcmp(const void *a, const void *b);

#define qsort(a,b,c,d) pg_qsort(a,b,c,d)

typedef int (*qsort_arg_comparator) (const void *a, const void *b, void *arg);

extern void qsort_arg(void *base, size_t nel, size_t elsize,
					  qsort_arg_comparator cmp, void *arg);

#endif							/* PG_PORT_H */

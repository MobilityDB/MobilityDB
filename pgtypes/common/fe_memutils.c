/*-------------------------------------------------------------------------
 *
 * fe_memutils.c
 *	  memory management support for frontend code
 *
 * Portions Copyright (c) 1996-2025, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  src/common/fe_memutils.c
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"
#include "common/fe_memutils.h"

#if MEOS
#include "meos.h"

/*
 * Optional allocator hook. When unset the hooks are libc malloc/realloc/free,
 * so the allocation path is byte-identical to a plain libc build; an embedder
 * may install its own allocator via meos_initialize_allocator to make MEOS
 * working-memory visible to its memory manager. SIBLING-FROM the error-handler
 * hook (MEOS_ERROR_HANDLER in meos/src/temporal/error.c), including its atomic
 * publish/consume discipline for thread-safety.
 */
static meos_malloc_fn  MEOS_MALLOC  = &malloc;
static meos_realloc_fn MEOS_REALLOC = &realloc;
static meos_free_fn    MEOS_FREE    = &free;

void
meos_initialize_allocator(meos_malloc_fn malloc_fn,
	meos_realloc_fn realloc_fn, meos_free_fn free_fn)
{
	/* Publish with release semantics so a thread that acquire-loads a hook on
	 * the allocation path sees a fully-published value; a NULL argument resets
	 * that hook to the libc default. Mirror of meos_initialize_error_handler. */
	__atomic_store_n(&MEOS_MALLOC, malloc_fn ? malloc_fn : &malloc,
		__ATOMIC_RELEASE);
	__atomic_store_n(&MEOS_REALLOC, realloc_fn ? realloc_fn : &realloc,
		__ATOMIC_RELEASE);
	__atomic_store_n(&MEOS_FREE, free_fn ? free_fn : &free, __ATOMIC_RELEASE);
	return;
}

static inline void *
meos_hook_malloc(size_t size)
{
	meos_malloc_fn fn = __atomic_load_n(&MEOS_MALLOC, __ATOMIC_ACQUIRE);
	return fn(size);
}

static inline void *
meos_hook_realloc(void *ptr, size_t size)
{
	meos_realloc_fn fn = __atomic_load_n(&MEOS_REALLOC, __ATOMIC_ACQUIRE);
	return fn(ptr, size);
}

static inline void
meos_hook_free(void *ptr)
{
	meos_free_fn fn = __atomic_load_n(&MEOS_FREE, __ATOMIC_ACQUIRE);
	fn(ptr);
}
#endif /* MEOS */

static inline void *
pg_malloc_internal(size_t size, int flags)
{
	void	   *tmp;

	/* Avoid unportable behavior of malloc(0) */
	if (size == 0)
		size = 1;
#if MEOS
	tmp = meos_hook_malloc(size);	/* MEOS SIBLING-FROM error.c */
#else
	tmp = malloc(size);
#endif
	if (tmp == NULL)
	{
		if ((flags & MCXT_ALLOC_NO_OOM) == 0)
		{
			fprintf(stderr, _("out of memory\n"));
			exit(EXIT_FAILURE);
		}
		return NULL;
	}

	if ((flags & MCXT_ALLOC_ZERO) != 0)
		MemSet(tmp, 0, size);
	return tmp;
}

void *
pg_malloc(size_t size)
{
	return pg_malloc_internal(size, 0);
}

void *
pg_malloc0(size_t size)
{
	return pg_malloc_internal(size, MCXT_ALLOC_ZERO);
}

void *
pg_malloc_extended(size_t size, int flags)
{
	return pg_malloc_internal(size, flags);
}

void *
pg_realloc(void *ptr, size_t size)
{
	void	   *tmp;

	/* Avoid unportable behavior of realloc(NULL, 0) */
	if (ptr == NULL && size == 0)
		size = 1;
#if MEOS
	tmp = meos_hook_realloc(ptr, size);	/* MEOS SIBLING-FROM error.c */
#else
	tmp = realloc(ptr, size);
#endif
	if (!tmp)
	{
		fprintf(stderr, _("out of memory\n"));
		exit(EXIT_FAILURE);
	}
	return tmp;
}

/*
 * "Safe" wrapper around strdup().
 */
char *
pg_strdup(const char *in)
{
	char	   *tmp;

	if (!in)
	{
		fprintf(stderr,
				_("cannot duplicate null pointer (internal error)\n"));
		exit(EXIT_FAILURE);
	}
#if MEOS
	/* MEOS SIBLING-FROM error.c: libc strdup would bypass the allocator hook,
	 * so duplicate through the hook explicitly. */
	{
		size_t		len = strlen(in) + 1;

		tmp = meos_hook_malloc(len);
		if (tmp)
			memcpy(tmp, in, len);
	}
#else
	tmp = strdup(in);
#endif
	if (!tmp)
	{
		fprintf(stderr, _("out of memory\n"));
		exit(EXIT_FAILURE);
	}
	return tmp;
}

void
pg_free(void *ptr)
{
#if MEOS
	meos_hook_free(ptr);	/* MEOS SIBLING-FROM error.c */
#else
	free(ptr);
#endif
}

/*
 * Frontend emulation of backend memory management functions.  Useful for
 * programs that compile backend files.
 */
void *
palloc(Size size)
{
	return pg_malloc_internal(size, 0);
}

void *
palloc0(Size size)
{
	return pg_malloc_internal(size, MCXT_ALLOC_ZERO);
}

void *
palloc_extended(Size size, int flags)
{
	return pg_malloc_internal(size, flags);
}

void
pfree(void *pointer)
{
	pg_free(pointer);
}

char *
pstrdup(const char *in)
{
	return pg_strdup(in);
}

char *
pnstrdup(const char *in, Size size)
{
	char	   *tmp;
	int			len;

	if (!in)
	{
		fprintf(stderr,
				_("cannot duplicate null pointer (internal error)\n"));
		exit(EXIT_FAILURE);
	}

	len = strnlen(in, size);
#if MEOS
	tmp = meos_hook_malloc(len + 1);	/* MEOS SIBLING-FROM error.c */
#else
	tmp = malloc(len + 1);
#endif
	if (tmp == NULL)
	{
		fprintf(stderr, _("out of memory\n"));
		exit(EXIT_FAILURE);
	}

	memcpy(tmp, in, len);
	tmp[len] = '\0';

	return tmp;
}

void *
repalloc(void *pointer, Size size)
{
	return pg_realloc(pointer, size);
}

/*-------------------------------------------------------------------------
 *
 * dirent.c
 *    Windows-only opendir/readdir/closedir implementation backed by
 *    Win32 FindFirstFile/FindNextFile.  Vendored from PostgreSQL
 *    upstream src/port/dirent.c (BSD-licensed); the same shim
 *    PostgreSQL uses for its standalone Windows MSVC builds.
 *
 *    Compiled only on WIN32 + non-MinGW (MinGW provides its own
 *    <dirent.h> implementation; this file is excluded there via the
 *    CMakeLists.txt guard).
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#ifdef WIN32
#ifndef __MINGW32__

#include <windows.h>

#include "dirent.h"

struct DIR
{
	char	   *dirname;
	struct dirent ret;			/* returned by readdir */
	HANDLE		handle;
};

DIR *
opendir(const char *dirname)
{
	DWORD		attr;
	DIR		   *d;

	/* Make sure it is a directory. */
	attr = GetFileAttributes(dirname);
	if (attr == INVALID_FILE_ATTRIBUTES)
	{
		errno = ENOENT;
		return NULL;
	}
	if ((attr & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		errno = ENOTDIR;
		return NULL;
	}

	d = malloc(sizeof(DIR));
	if (!d)
	{
		errno = ENOMEM;
		return NULL;
	}
	d->dirname = malloc(strlen(dirname) + 4);	/* +3 for "\*?" + nul */
	if (!d->dirname)
	{
		free(d);
		errno = ENOMEM;
		return NULL;
	}
	strcpy(d->dirname, dirname);
	if (d->dirname[strlen(d->dirname) - 1] != '/' &&
		d->dirname[strlen(d->dirname) - 1] != '\\')
		strcat(d->dirname, "\\");		/* add a backslash */
	strcat(d->dirname, "*");			/* match everything */

	d->handle = INVALID_HANDLE_VALUE;
	d->ret.d_ino = 0;					/* no inodes on Windows */
	d->ret.d_reclen = 0;				/* not used in any current MEOS code */
	d->ret.d_type = DT_UNKNOWN;

	return d;
}

struct dirent *
readdir(DIR *d)
{
	WIN32_FIND_DATA fd;

	if (d->handle == INVALID_HANDLE_VALUE)
	{
		d->handle = FindFirstFile(d->dirname, &fd);
		if (d->handle == INVALID_HANDLE_VALUE)
		{
			errno = ENOENT;
			return NULL;
		}
	}
	else
	{
		if (!FindNextFile(d->handle, &fd))
		{
			if (GetLastError() == ERROR_NO_MORE_FILES)
			{
				/* No error; we are simply done. */
				errno = 0;
				return NULL;
			}

			_dosmaperr(GetLastError());
			return NULL;
		}
	}

	strcpy(d->ret.d_name, fd.cFileName);	/* fd.cFileName is bounded by NAME_MAX+1 */
	d->ret.d_namlen = (unsigned short) strlen(d->ret.d_name);
	d->ret.d_type =
		(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? DT_DIR :
		(fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) ? DT_LNK :
		DT_REG;

	return &d->ret;
}

int
closedir(DIR *d)
{
	int			ret = 0;

	if (d->handle != INVALID_HANDLE_VALUE)
		ret = !FindClose(d->handle);
	free(d->dirname);
	free(d);
	return ret;
}

#endif	/* !__MINGW32__ */
#endif	/* WIN32 */

/*-------------------------------------------------------------------------
 *
 * dirent.h
 *    Windows-only opendir/readdir/closedir + DIR/struct dirent shims
 *    for MEOS code that includes <dirent.h> (pgfnames.c, pgtz.c).
 *
 * Native Windows MSVC has no <dirent.h>; vendored from PostgreSQL upstream
 * src/include/port/dirent.h (BSD-licensed; the same shim PostgreSQL uses
 * for its standalone Windows builds).  MinGW already ships a <dirent.h>
 * implementation, so this header is a no-op there.
 *
 * Linux / macOS pick up the system <dirent.h> at the same include path
 * naturally; this file is only ever consulted on Windows.
 *
 *-------------------------------------------------------------------------
 */
#ifndef MEOS_PORT_DIRENT_H
#define MEOS_PORT_DIRENT_H

/*
 * On Linux / macOS / BSD this header is shadowing the system <dirent.h>
 * (because postgres/port/ is on the include path).  Fall through to the
 * system header via #include_next so MEOS code that #include <dirent.h>
 * gets the real DIR / struct dirent definitions everywhere except WIN32.
 *
 * #include_next is supported by GCC, Clang, and recent MSVC.
 */
#ifndef WIN32
#include_next <dirent.h>
#else

/*
 * MinGW ships a <dirent.h> — defer to it.  MSVC native does not, so
 * declare the minimal POSIX surface.
 */
#ifdef __MINGW32__
#include <../include/dirent.h>		/* MinGW's own */
#else

#include <sys/types.h>

#define NAME_MAX 255

/*
 * Maximum entries returned per readdir(3) call.  PostgreSQL chooses
 * 4 KB; we match.
 */
typedef struct dirent
{
	long		d_ino;
	unsigned short d_reclen;
	unsigned char d_type;
	unsigned short d_namlen;
	char		d_name[NAME_MAX + 1];
} dirent;

typedef struct DIR DIR;

extern DIR *opendir(const char *);
extern struct dirent *readdir(DIR *);
extern int	closedir(DIR *);

/* For d_type — POSIX values, mapped from Win32 file attributes. */
#define DT_UNKNOWN	0
#define DT_FIFO		1
#define DT_CHR		2
#define DT_DIR		4
#define DT_BLK		6
#define DT_REG		8
#define DT_LNK		10
#define DT_SOCK		12

#endif	/* __MINGW32__ */

#endif	/* !WIN32 ... else WIN32 ... */

#endif	/* MEOS_PORT_DIRENT_H */

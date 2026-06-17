dnl **********************************************************************
dnl * configure.ac
dnl *
dnl *  Pointclound build configuration.
dnl *
dnl *  PgSQL Pointcloud is free and open source software provided
dnl *  by the Government of Canada
dnl *  Copyright (c) 2013 Natural Resources Canada
dnl *
dnl ***********************************************************************/

AC_INIT()
AC_CONFIG_MACRO_DIR([macros])
AC_CONFIG_HEADERS([lib/pc_config.h pgsql/sqldefines.h])
AC_LANG([C++])

dnl
dnl Compilers
dnl
AC_PROG_CC

dnl
dnl SQL Preprocessor
dnl
AC_PATH_PROG([CPPBIN], [cpp], [])
if test "x$CPPBIN" != "x"; then
  SQLPP="${CPPBIN} -traditional-cpp -w -P"
else
  AC_PATH_PROG([GPP], [gpp_], [])
  if test "x$GPP" != "x"; then
    SQLPP="${GPP} -C -s \'" dnl Use better string support
  else
    if test "x${CPP}" != "x"; then
      SQLPP="${CPP} -traditional-cpp"
    else
	  AC_MSG_ERROR([Required "cpp" command not found])
    fi
  fi
fi
AC_SUBST([SQLPP])

dnl
dnl Define executable suffix to use for utility programs
dnl
EXESUFFIX="$ac_cv_exeext"
AC_SUBST([EXESUFFIX])

dnl
dnl Search for flex/bison to build the parser
dnl
dnl AC_PROG_LEX
dnl AC_PROG_YACC
dnl AC_SUBST([LEX])
dnl AC_SUBST([YACC])

dnl ===========================================================================
dnl Version Information imported from Version.config and Git
dnl ===========================================================================

AC_CHECK_PROG([GITCMD], [git —version], [yes], [no])
AC_CHECK_FILE([.git], [DOTGITDIR=yes], [DOTGITDIR=no])
if test "x${GITCMD}" = "xyes" -a "x${DOTGITDIR}" = "xyes"; then
    GIT_COMMIT_HASH=" $(git rev-parse --short HEAD)"
else
    GIT_COMMIT_HASH=
fi
POINTCLOUD_VERSION="$(cat Version.config)$GIT_COMMIT_HASH"
AC_SUBST([POINTCLOUD_VERSION])
AC_DEFINE_UNQUOTED([POINTCLOUD_VERSION], ["$POINTCLOUD_VERSION"], [Pointcloud version])

dnl ===========================================================================
dnl Detect ZLib if it is installed
dnl ===========================================================================

ZLIB_LDFLAGS=""
AC_CHECK_HEADER([zlib.h], [
	ZLIB_CPPFLAGS="$CPPFLAGS"
	AC_CHECK_LIB([z],
	  [inflate],
	  [ZLIB_LDFLAGS="$LDFLAGS -lz"],
	  [AC_MSG_ERROR([could not locate zlib])]
	  )
	],
	[
	AC_MSG_ERROR([could not locate zlib])
	])

AC_SUBST([ZLIB_CPPFLAGS])
AC_SUBST([ZLIB_LDFLAGS])


dnl ===========================================================================
dnl Detect CUnit if it is installed
dnl ===========================================================================

AC_ARG_WITH([cunit],
	[AS_HELP_STRING([--with-cunit=DIR], [specify the base cunit install directory])],
	[CUNITDIR="$withval"], [CUNITDIR=""])

if test "x$CUNITDIR" = "xyes"; then
	AC_MSG_ERROR([you must specify a prefix directory to --with-cunit, e.g. --with-cunit=/opt/local])
fi

if test "x$CUNITDIR" != "x"; then

  dnl CUNITDIR was specified, so let's look there!

  dnl Build the linker and include flags
  CUNIT_LDFLAGS="-L${CUNITDIR}/lib"
  CUNIT_CPPFLAGS="-I${CUNITDIR}/include"

  dnl Swap to use provided cflags
  CPPFLAGS_SAVE="$CPPFLAGS"
  CPPFLAGS="$CUNIT_CPPFLAGS"

  dnl Swap to use provided ldflags
  LDFLAGS_SAVE="$LDFLAGS"
  LDFLAGS="$CUNIT_LDFLAGS"

fi

dnl Run the header/link tests
AC_CHECK_HEADER([CUnit/CUnit.h], [
	CUNIT_CPPFLAGS="$CPPFLAGS"
	AC_CHECK_LIB([cunit],
	  [CU_initialize_registry],
	  [CUNIT_LDFLAGS="$LDFLAGS -lcunit" FOUND_CUNIT="YES"],
	  [FOUND_CUNIT="NO"]
	  )
	],
	[
    FOUND_CUNIT="NO"
	])

if test "x$CUNITDIR" != "x"; then
  dnl Swap back to the original flags
  LDFLAGS="${LDFLAGS_SAVE}"
  CPPFLAGS="${CPPFLAGS_SAVE}"
fi

if test "$FOUND_CUNIT" = "YES"; then
  AC_DEFINE([HAVE_CUNIT], [1], [Have CUnit])
  CUNIT_STATUS="enabled"
  if test $CUNITDIR; then
    CUNIT_STATUS="$CUNITDIR"
  fi
else
  CUNIT_LDFLAGS=""
  CUNIT_CPPFLAGS=""
  CUNIT_STATUS="disabled"
fi

AC_SUBST([CUNIT_LDFLAGS])
AC_SUBST([CUNIT_CPPFLAGS])


dnl ===========================================================================
dnl Detect the version of PostgreSQL installed on the system
dnl ===========================================================================

AC_ARG_WITH([pgconfig],
	[AS_HELP_STRING([--with-pgconfig=FILE], [specify an alternative pg_config file])],
	[PG_CONFIG="$withval"], [PG_CONFIG=""])

if test "x$PG_CONFIG" = "x"; then
	dnl PG_CONFIG was not specified, so search within the current path
	AC_PATH_PROG([PG_CONFIG], [pg_config])

	dnl If we couldn't find pg_config, display an error
	if test "x$PG_CONFIG" = "x"; then
		AC_MSG_ERROR([could not find pg_config within the current path. You may need to try re-running configure with a --with-pgconfig parameter.])
	fi
else
	dnl PG_CONFIG was specified; display a message to the user
	if test "x$PG_CONFIG" = "xyes"; then
		AC_MSG_ERROR([you must specify a parameter to --with-pgconfig, e.g. --with-pgconfig=/path/to/pg_config])
	else
		if test -f $PG_CONFIG; then
			AC_MSG_RESULT([Using user-specified pg_config file: $PG_CONFIG])
		else
			AC_MSG_ERROR([the user-specified pg_config file $PG_CONFIG does not exist])
		fi
	fi
fi


dnl ===========================================================================
dnl Ensure that $PG_CONFIG --pgxs points to a valid file. This is because some
dnl distributions such as Debian also include pg_config as part of libpq-dev
dnl packages, but don't install the Makefile it points to unless
dnl the postgresql-server-dev packages are installed :)
dnl ===========================================================================

PGXS=`$PG_CONFIG --pgxs`
if test ! -f $PGXS; then
	AC_MSG_ERROR([the PGXS Makefile $PGXS cannot be found. Please install the PostgreSQL server development packages and re-run configure.])
fi

AC_SUBST([PG_CONFIG])
AC_SUBST([PGXS])

dnl Extract the version information from pg_config
dnl Note: we extract the major & minor separately, ensure they are numeric, and then combine to give
dnl the final version. This is to guard against user error...
PGSQL_FULL_VERSION=`$PG_CONFIG --version`
PGSQL_MAJOR_VERSION=`echo $PGSQL_FULL_VERSION | sed 's/[[^0-9]]*\([[0-9]]*\).*/\1/'`
PGSQL_MINOR_VERSION=`echo $PGSQL_FULL_VERSION | sed 's/[[^0-9]]*\([[0-9]]*\)\.\([[0-9]]*\).*/\2/'`

if test "$PGSQL_MAJOR_VERSION" -ge 9; then
	PGSQL_MINOR_VERSION=0
fi

PGSQL_VERSION="$PGSQL_MAJOR_VERSION$PGSQL_MINOR_VERSION"

AC_SUBST([PGSQL_MAJOR_VERSION])

PGSQL_PKGLIBDIR=`$PG_CONFIG --pkglibdir`
PGSQL_LIBDIR=`$PG_CONFIG --libdir`
PGSQL_SHAREDIR=`$PG_CONFIG --sharedir`

AC_MSG_RESULT([checking PostgreSQL version... $PGSQL_FULL_VERSION])
dnl Ensure that we are using PostgreSQL >= 9.0
if test ! "$PGSQL_MAJOR_VERSION" -ge 9; then
	AC_MSG_ERROR([PointCloud requires PostgreSQL >= 9.0])
fi

dnl Extract the linker and include flags for the frontend (for programs that use libpq)
PGSQL_FE_LDFLAGS=-L`$PG_CONFIG --libdir`" -lpq"
PGSQL_FE_CPPFLAGS=-I`$PG_CONFIG --includedir`

AC_SUBST([PGSQL_FE_LDFLAGS])
AC_SUBST([PGSQL_FE_CPPFLAGS])

dnl Ensure that we can parse libpq-fe.h
CPPFLAGS_SAVE="$CPPFLAGS"
CPPFLAGS="$PGSQL_FE_CPPFLAGS"
AC_CHECK_HEADER([libpq-fe.h], [], [AC_MSG_ERROR([could not find libpq-fe.h])])
CPPFLAGS="$CPPFLAGS_SAVE"

dnl Ensure we can link against libpq
LIBS_SAVE="$LIBS"
LIBS="$PGSQL_FE_LDFLAGS"
AC_CHECK_LIB([pq], [PQserverVersion],
	[],
	[AC_MSG_ERROR([could not find libpq])],
	[])
LIBS="$LIBS_SAVE"

AC_DEFINE_UNQUOTED([PGSQL_VERSION], [$PGSQL_VERSION], [PostgreSQL server version])
AC_SUBST([PGSQL_VERSION])


dnl ===========================================================================
dnl Detect LibXML2
dnl ===========================================================================

AC_ARG_WITH([xml2config],
	[AS_HELP_STRING([--with-xml2config=FILE], [specify an alternative xml2-config file])],
	[XML2CONFIG="$withval"], [XML2CONFIG=""])

if test "x$XML2CONFIG" = "x"; then
	dnl XML2CONFIG was not specified, so search within the current path
	AC_PATH_PROG([XML2CONFIG], [xml2-config])

	dnl If we couldn't find xml2-config, display a warning
	if test "x$XML2CONFIG" = "x"; then
		AC_MSG_ERROR([could not find xml2-config from libxml2 within the current path. You may need to try re-running configure with a --with-xml2config parameter.])
	fi
else
	dnl XML2CONFIG was specified; display a message to the user
	if test "x$XML2CONFIG" = "xyes"; then
		AC_MSG_ERROR([you must specify a parameter to --with-xml2config, e.g. --with-xml2config=/path/to/xml2-config])
	else
		if test -f $XML2CONFIG; then
			AC_MSG_RESULT([Using user-specified xml2-config file: $XML2CONFIG])
		else
			AC_MSG_ERROR([the user-specified xml2-config file $XML2CONFIG does not exist])
		fi
	fi
fi


dnl Extract the linker and include flags
XML2_LDFLAGS=`$XML2CONFIG --libs`
XML2_CPPFLAGS=`$XML2CONFIG --cflags`

dnl Extract the version
LIBXML2_VERSION=`$XML2CONFIG --version`

dnl Check headers file
CPPFLAGS_SAVE="$CPPFLAGS"
CPPFLAGS="$XML2_CPPFLAGS"
AC_CHECK_HEADERS([libxml/tree.h libxml/parser.h libxml/xpath.h libxml/xpathInternals.h],
		 [], [AC_MSG_ERROR([could not find headers include related to libxml2])])
CPPFLAGS="$CPPFLAGS_SAVE"

dnl Ensure we can link against libxml2
LIBS_SAVE="$LIBS"
LIBS="$XML2_LDFLAGS"
AC_CHECK_LIB([xml2], [xmlInitParser], [], [AC_MSG_ERROR([could not find libxml2])], [])
LIBS="$LIBS_SAVE"

AC_DEFINE_UNQUOTED([LIBXML2_VERSION], ["$LIBXML2_VERSION"], [PointCloud libxml2 version])
AC_SUBST([LIBXML2_VERSION])
AC_SUBST([XML2_LDFLAGS])
AC_SUBST([XML2_CPPFLAGS])

dnl ===========================================================================
dnl Detect LazPerf
dnl ===========================================================================

AC_ARG_WITH([lazperf],
	[AS_HELP_STRING([--with-lazperf=DIR], [specify the base lazperf installation directory])],
	[LAZPERFDIR="$withval"], [LAZPERFDIR=""])

if test "x$LAZPERFDIR" = "xyes"; then
	AC_MSG_ERROR([you must specify a parameter to --with-lazperf, e.g. --with-lazperf=/opt/local])
fi

if test "x$LAZPERFDIR" = "x"; then
  dnl LAZPERFDIR was not specified, so search in usual system places
  AC_CHECK_HEADER([las.hpp],
		  [FOUND_LAZPERF="YES"],
		  [FOUND_LAZPERF="NO"])
elif test "x$LAZPERFDIR" != "xno"; then
  dnl LAZPERFDIR was specified, so let's look there!

  LAZPERF_CPPFLAGS="-I${LAZPERFDIR}/include/"

  dnl Check header file
  CPPFLAGS_SAVE="${CPPFLAGS}"
  CPPFLAGS="${LAZPERF_CPPFLAGS} --std=c++0x"

  AC_CHECK_HEADER([laz-perf/las.hpp],
		  [FOUND_LAZPERF="YES"],
		  [FOUND_LAZPERF="NO"])

  dnl back to the original
  CPPFLAGS="${CPPFLAGS_SAVE}"
fi

if test "x$FOUND_LAZPERF" = "xYES"; then
  AC_DEFINE([HAVE_LAZPERF], [1], [Have LAZ perf])
  LAZPERF_STATUS="enabled"
  if test $LAZPERFDIR; then
    LAZPERF_STATUS="$LAZPERFDIR/include/laz-perf"
  fi
else
  LAZPERF_STATUS="disabled"
fi

AC_SUBST([LAZPERF_STATUS])
AC_SUBST([LAZPERF_CPPFLAGS])

dnl ===========================================================================
dnl Figure out where this script is running

PROJECT_SOURCE_DIR="$( cd "$( dirname $0 )" && pwd )"
AC_DEFINE_UNQUOTED([PROJECT_SOURCE_DIR], ["$PROJECT_SOURCE_DIR"], [Project source dir])


dnl ===========================================================================
dnl Output the relevant files
dnl ===========================================================================

AC_OUTPUT([
   config.mk
   pgsql/Makefile
])

dnl ===========================================================================
dnl Display the configuration status information
dnl ===========================================================================

AC_MSG_RESULT()
AC_MSG_RESULT([  PointCloud is now configured for ${host}])
AC_MSG_RESULT()
AC_MSG_RESULT([ -------------- Compiler Info ------------- ])
AC_MSG_RESULT([  C compiler:           ${CC} ${CFLAGS}])
AC_MSG_RESULT([  SQL preprocessor:     ${SQLPP}])
AC_MSG_RESULT()
AC_MSG_RESULT([ -------------- Dependencies -------------- ])
AC_MSG_RESULT([  PostgreSQL config:    ${PG_CONFIG}])
AC_MSG_RESULT([  PostgreSQL version:   ${PGSQL_FULL_VERSION} (${PGSQL_VERSION})])
AC_MSG_RESULT([  Libxml2 config:       ${XML2CONFIG}])
AC_MSG_RESULT([  Libxml2 version:      ${LIBXML2_VERSION}])
AC_MSG_RESULT([  LazPerf status:       ${LAZPERF_STATUS}])
AC_MSG_RESULT([  CUnit status:         ${CUNIT_STATUS}])
AC_MSG_RESULT()

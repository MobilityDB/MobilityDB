set(PORT_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/chklocale.c
  ${CMAKE_CURRENT_SOURCE_DIR}/pg_bitutils.c
  ${CMAKE_CURRENT_SOURCE_DIR}/pg_localeconv_r.c
  ${CMAKE_CURRENT_SOURCE_DIR}/qsort.c
  ${CMAKE_CURRENT_SOURCE_DIR}/qsort_arg.c
  ${CMAKE_CURRENT_SOURCE_DIR}/strerror.c
  )

# Only compile the fallback strlcpy when the platform lacks a native one
# (HAVE_DECL_STRLCPY is detected in the parent CMakeLists.txt).
if(NOT HAS_STRLCPY)
  list(APPEND PORT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/strlcpy.c)
endif()

# pgstrcasecmp.c and snprintf.c duplicate symbols the backend already exports.
# Only compile them outside the Windows/macOS extension build.
if(NOT PGTYPES_EXT_NO_BACKEND_DUPS)
  list(APPEND PORT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/pgstrcasecmp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/snprintf.c
  )
endif()

# Native Windows MSVC has no <dirent.h>; MEOS timezone code (pgtypes/timezone/
# pgtz.c) includes it. Add the vendored shim (BSD-licensed from PostgreSQL
# upstream). MinGW already provides its own <dirent.h>, so the .c body is
# itself guarded against MinGW.
if(WIN32)
  list(APPEND PORT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/dirent.c)
  # The mingw-w64 / UCRT runtime does not export _dosmaperr through its import
  # libraries, and PostgreSQL's win32error.c is normally backend-only. Vendor
  # it so pg_locale.c / dirent.c / fd.c (which call _dosmaperr after a failed
  # Win32 call) link in both the standalone MEOS and the extension builds.
  list(APPEND PORT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/win32error.c)
endif()

set(PGTYPES_SOURCES ${PGTYPES_SOURCES} ${PORT_SOURCES}
  # Mandatory so that the variable update is visible in the parent directory's scope
  PARENT_SCOPE
  )

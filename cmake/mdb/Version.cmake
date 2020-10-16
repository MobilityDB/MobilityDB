#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
# MOBILITYDB version variables
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------

set(MOBILITYDB_LIB_VERSION "${MOBILITYDB_VERSION_MAJOR}.${MOBILITYDB_VERSION_MINOR}")
set(MOBILITYDB_SHORT_VERSION "${MOBILITYDB_VERSION_MAJOR}.${MOBILITYDB_VERSION_MINOR}${MOBILITYDB_VERSION_DEV}")
set(MOBILITYDB_FULL_VERSION "v${MOBILITYDB_VERSION}${MOBILITYDB_VERSION_DEV}")

if (MOBILITYDB_VERSION_DEV)
    set(MOBILITYDB_DOC_LINK "https://docs.pgrouting.org/dev/en")
else()
    set(MOBILITYDB_DOC_LINK "https://docs.pgrouting.org/${MOBILITYDB_LIB_VERSION}/en")
endif()


if (MOBILITYDB_DEBUG)
    message(STATUS "MOBILITYDB_VERSION: ${MOBILITYDB_VERSION}")
    message(STATUS "MOBILITYDB_VERSION_MAJOR: ${MOBILITYDB_VERSION_MAJOR}")
    message(STATUS "MOBILITYDB_VERSION_MINOR: ${MOBILITYDB_VERSION_MINOR}")
    message(STATUS "MOBILITYDB_VERSION_PATCH: ${MOBILITYDB_VERSION_PATCH}")
    message(STATUS "MOBILITYDB_VERSION_TWEAK: ${MOBILITYDB_VERSION_TWEAK}")
    message(STATUS "MOBILITYDB_VERSION_DEV: ${MOBILITYDB_VERSION_DEV}")

    message(STATUS "MOBILITYDB_SHORT_VERSION: ${MOBILITYDB_SHORT_VERSION}")
    message(STATUS "MOBILITYDB_FULL_VERSION: ${MOBILITYDB_FULL_VERSION}")

    message(STATUS "MOBILITYDB_LIB_VERSION: ${MOBILITYDB_LIB_VERSION}")
    message(STATUS "MOBILITYDB_DOC_LINK: ${MOBILITYDB_DOC_LINK}")

endif()

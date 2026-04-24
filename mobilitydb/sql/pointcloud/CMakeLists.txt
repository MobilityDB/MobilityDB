SET(LOCAL_FILES
  400_pcset
  # 401_tpcpoint    # Phase 8G
  # 402_tpcpatch    # Phase 8H
  )

foreach (f ${LOCAL_FILES})
  process_file(${f} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
  configure_file(${CMAKE_CURRENT_BINARY_DIR}/${f}.sql.in ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql @ONLY)
  list(APPEND PACKAGE_SQL_FILES  ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql)
endforeach()

set(PROJECT_SQL_FILES ${PROJECT_SQL_FILES} ${PACKAGE_SQL_FILES} PARENT_SCOPE)

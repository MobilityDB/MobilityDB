set(TESTS
  clustering
  deprecated
  edge
  extension
  hierarchy
  indexing
  inspection
  miscellaneous
  opclass_brin
  opclass_btree
  opclass_hash
  opclass_spgist
  regions
  traversal
  type
  vertex
)

if(PostgreSQL_REGRESS)
  add_test(
    NAME h3_regress
    COMMAND ${PostgreSQL_REGRESS}
      --temp-instance=${CMAKE_BINARY_DIR}/tmp
      --bindir=${PostgreSQL_BIN_DIR}
      --inputdir=${CMAKE_CURRENT_SOURCE_DIR}
      --outputdir=${CMAKE_CURRENT_BINARY_DIR}
      --load-extension h3
      ${TESTS}
  )
endif()

if(PostgreSQL_VALIDATE_EXTUPGRADE)
  add_test(
    NAME h3_validate_extupgrade
    COMMAND pg_validate_extupgrade
      --extname h3
      --from 0.1.0
      --to ${INSTALL_VERSION}
  )
endif()

# @TODO: Figure out how to inline on MacOS
if(NOT APPLE)
  add_test(
    NAME h3_inlined
    COMMAND sh -c "! objdump -D ${PostgreSQL_PKG_LIBRARY_DIR}/h3.so | grep radsToDegs"
    CONFIGURATIONS Release
  )
endif()

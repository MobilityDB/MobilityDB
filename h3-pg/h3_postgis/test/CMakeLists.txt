set(TESTS
  deprecations
  postgis
  rasters
)

if(PostgreSQL_REGRESS)
  add_test(
    NAME "h3_postgis_regress"
    COMMAND ${PostgreSQL_REGRESS}
      --temp-instance=${CMAKE_BINARY_DIR}/tmp
      --bindir=${PostgreSQL_BIN_DIR}
      --inputdir=${CMAKE_CURRENT_SOURCE_DIR}
      --outputdir=${CMAKE_CURRENT_BINARY_DIR}
      --load-extension h3
      --load-extension postgis
      --load-extension postgis_raster
      --load-extension h3_postgis
      ${TESTS}
  )
endif()

if(PostgreSQL_VALIDATE_EXTUPGRADE)
  add_test(
    NAME "h3_postgis_validate_extupgrade"
    COMMAND pg_validate_extupgrade
        --extname h3_postgis
        --from 4.0.0
        --to ${INSTALL_VERSION}
  )
endif()

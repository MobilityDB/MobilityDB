include(AssembleExtensionSql)

# Configure the control file
configure_file(mobilitydb_datagen.in.control ${CMAKE_BINARY_DIR}/mobilitydb_datagen.control)

# Define list of source files. Only the random_* generators belong to the
# extension; the create_test_tables_* files are per-domain test scaffolding.
set(DATAGEN_FILES
  temporal/random_temporal.sql
  geo/random_tpoint.sql
  geo/random_tgeo.sql
  )
if(CBUFFER)
  list(APPEND DATAGEN_FILES cbuffer/random_tcbuffer.sql)
endif()
if(H3)
  list(APPEND DATAGEN_FILES h3/random_th3index.sql)
endif()
# if(JSON)
  # list(APPEND DATAGEN_FILES json/random_tjson.sql)
# endif()
if(NPOINT)
  list(APPEND DATAGEN_FILES npoint/random_tnpoint.sql)
endif()
if(POINTCLOUD)
  list(APPEND DATAGEN_FILES pointcloud/random_tpcpoint.sql)
endif()
if(POSE)
  list(APPEND DATAGEN_FILES pose/random_tpose.sql)
  list(APPEND DATAGEN_FILES posechain/random_posechain.sql)
endif()
if(QUADBIN)
  list(APPEND DATAGEN_FILES quadbin/random_tquadbin.sql)
endif()
if(RASTER)
  list(APPEND DATAGEN_FILES raster/random_raquet.sql)
endif()
if(RGEO)
  list(APPEND DATAGEN_FILES rgeo/random_trgeo.sql)
endif()

list(TRANSFORM DATAGEN_FILES PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)

configure_file(header.in.sql ${CMAKE_CURRENT_BINARY_DIR}/header.sql @ONLY)

assemble_extension_sql(
  OUTPUT ${CMAKE_BINARY_DIR}/${MOBILITYDB_DATAGEN_EXTENSION_FILE}
  HEADER ${CMAKE_CURRENT_BINARY_DIR}/header.sql
  BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
  SOURCES ${DATAGEN_FILES})

add_sql_configure_depends(${DATAGEN_FILES})

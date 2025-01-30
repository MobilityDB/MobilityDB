
SET(LOCAL_FILES
  general/random_temporal.sql
  point/random_tpoint.sql
  )
if(NPOINT)
  SET(LOCAL_FILES
    ${LOCAL_FILES}
    npoint/random_tnpoint.sql
    )
endif()
  
configure_file(mobilitydb_datagen.control ${CMAKE_BINARY_DIR}/mobilitydb_datagen.control)

foreach (f ${LOCAL_FILES})
  file(READ ${CMAKE_SOURCE_DIR}/mobilitydb/datagen/${f} CURR_CONTENTS)
  file(APPEND ${CMAKE_BINARY_DIR}/${MOBILITYDB_DATAGEN_EXTENSION_FILE} "${CURR_CONTENTS}")
endforeach()

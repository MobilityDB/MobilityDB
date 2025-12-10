# Configure the control file
configure_file(mobilitydb_datagen.in.control ${CMAKE_BINARY_DIR}/mobilitydb_datagen.control)

# Configure the header template directly into the extension file
string(TIMESTAMP GENERATION_DATE "%Y-%m-%d %H:%M:%S")
configure_file(
    header.in.sql
    ${CMAKE_BINARY_DIR}/${MOBILITYDB_DATAGEN_EXTENSION_FILE}
    @ONLY
)

# Define list of source files
set(DATAGEN_FILES
  temporal/random_temporal.sql
  geo/random_tpoint.sql
  geo/random_tgeo.sql
  )
if(CBUFFER)
  list(APPEND DATAGEN_FILES cbuffer/random_tcbuffer.sql)
endif()
# if(JSON)
  # list(APPEND DATAGEN_FILES json/random_tjson.sql)
# endif()
if(NPOINT)
  list(APPEND DATAGEN_FILES npoint/random_tnpoint.sql)
endif()
if(POSE)
  list(APPEND DATAGEN_FILES pose/random_tpose.sql)
endif()
if(RGEO)
  list(APPEND DATAGEN_FILES rgeo/random_trgeo.sql)
endif()

# Append each source file with license removed
foreach(f ${DATAGEN_FILES})
  # Read file as a list (one item per line)
  file(READ ${f} CURR_CONTENTS)
  # Append to extension file with small header line
  file(APPEND ${CMAKE_BINARY_DIR}/${MOBILITYDB_DATAGEN_EXTENSION_FILE}
    "\n-- Begin contents of ${f}\n\n${CURR_CONTENTS}\n")
endforeach()

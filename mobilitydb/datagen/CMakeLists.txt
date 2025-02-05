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
set(LOCAL_FILES
  general/random_temporal.sql
  point/random_tpoint.sql
  )
if(CBUFFER)
  list(APPEND LOCAL_FILES cbuffer/random_tcbuffer.sql)
endif()
if(NPOINT)
  list(APPEND LOCAL_FILES npoint/random_tnpoint.sql)
endif()

# Append each source file with license removed
foreach(f ${LOCAL_FILES})
  # Read file as a list (one item per line)
  file(STRINGS ${f} FILE_LINES)
  # Remove first 28 lines (length of the license)
  list(SUBLIST FILE_LINES 29 -1 FILE_LINES)
  # Join remaining lines
  list(JOIN FILE_LINES "\n" FILE_CONTENT)
  # Append to extension file with small header line
  file(APPEND ${CMAKE_BINARY_DIR}/${MOBILITYDB_DATAGEN_EXTENSION_FILE}
    "\n-- Begin contents of ${f}\n\n${FILE_CONTENT}\n")
endforeach()

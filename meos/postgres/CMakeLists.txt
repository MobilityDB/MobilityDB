#---------------------------------------------
# Configure PostgreSQL configuration for MEOS
#---------------------------------------------

include(PostgresConfig)
config_pg()

#------------------------
# Include subdirectories
#------------------------

include_directories("common")
include_directories("lib")
include_directories("port")
include_directories("utils")

add_subdirectory("common")
add_subdirectory("port")
add_subdirectory("timezone")
add_subdirectory("utils")
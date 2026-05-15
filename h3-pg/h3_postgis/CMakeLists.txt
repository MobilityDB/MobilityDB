PostgreSQL_add_extension(postgresql_h3_postgis
  RELOCATABLE
  NAME h3_postgis
  COMMENT "H3 PostGIS integration"
  VERSION ${INSTALL_VERSION}
  COMPONENT ${PROJECT_NAME}
  REQUIRES
    h3
    postgis
    postgis_raster
  SOURCES
    src/init.c
    src/wkb_bbox3.c
    src/wkb_indexing.c
    src/wkb_linked_geo.c
    src/wkb_regions.c
    src/wkb_split.c
    src/wkb_vect3.c
    src/wkb.c
  INSTALLS
    sql/install/01-indexing.sql
    sql/install/03-traversal.sql
    sql/install/05-regions.sql
    sql/install/10-operators.sql
    sql/install/20-casts.sql
    sql/install/30-wkb.sql
    sql/install/40-rasters.sql
    sql/install/99-deprecated.sql
  UPDATES
    sql/updates/h3_postgis--4.0.0.sql
    sql/updates/h3_postgis--4.0.0--4.0.1.sql
    sql/updates/h3_postgis--4.0.1--4.0.2.sql
    sql/updates/h3_postgis--4.0.2--4.0.3.sql
    sql/updates/h3_postgis--4.0.3--4.1.0.sql
    sql/updates/h3_postgis--4.1.0--4.1.1.sql
    sql/updates/h3_postgis--4.1.1--4.1.2.sql
    sql/updates/h3_postgis--4.1.2--4.1.3.sql
    sql/updates/h3_postgis--4.1.3--4.1.4.sql
    sql/updates/h3_postgis--4.1.4--4.2.0.sql
    sql/updates/h3_postgis--4.2.0--4.2.1.sql
    sql/updates/h3_postgis--4.2.1--4.2.2.sql
    sql/updates/h3_postgis--4.2.2--4.2.3.sql
)

# link
target_link_libraries(postgresql_h3_postgis PRIVATE postgresql_h3_shared h3)

# test
if(BUILD_TESTING AND PostgreSQL_PostGIS_FOUND)
  add_subdirectory(test)
endif()

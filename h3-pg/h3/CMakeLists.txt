PostgreSQL_add_extension(postgresql_h3
  RELOCATABLE
  NAME h3
  COMMENT "H3 bindings for PostgreSQL"
  VERSION ${INSTALL_VERSION}
  COMPONENT ${PROJECT_NAME}
  SOURCES
    src/binding/edge.c
    src/binding/hierarchy.c
    src/binding/indexing.c
    src/binding/inspection.c
    src/binding/miscellaneous.c
    src/binding/regions.c
    src/binding/traversal.c
    src/binding/vertex.c
    src/deprecated.c
    src/extension.c
    src/guc.c
    src/init.c
    src/opclass_btree.c
    src/opclass_hash.c
    src/opclass_spgist.c
    src/operators.c
    src/srf.c
    src/type.c
  INSTALLS
    sql/install/00-type.sql
    sql/install/01-indexing.sql
    sql/install/02-inspection.sql
    sql/install/03-traversal.sql
    sql/install/04-hierarchy.sql
    sql/install/05-regions.sql
    sql/install/06-edge.sql
    sql/install/07-vertex.sql
    sql/install/08-miscellaneous.sql
    sql/install/10-operators.sql
    sql/install/11-opclass_btree.sql
    sql/install/12-opclass_hash.sql
    sql/install/13-opclass_brin.sql
    sql/install/14-opclass_spgist.sql
    sql/install/20-casts.sql
    sql/install/30-extension.sql
    sql/install/99-deprecated.sql
  UPDATES
    sql/updates/h3--0.1.0.sql
    sql/updates/h3--0.1.0--0.2.0.sql
    sql/updates/h3--0.2.0--0.3.0.sql
    sql/updates/h3--0.3.0--0.3.1.sql
    sql/updates/h3--0.3.1--0.3.2.sql
    sql/updates/h3--0.3.2--0.4.0.sql
    sql/updates/h3--0.4.0--1.0.0.sql
    sql/updates/h3--1.0.0--1.0.1.sql
    sql/updates/h3--1.0.1--1.0.2.sql
    sql/updates/h3--1.0.2--1.0.3.sql
    sql/updates/h3--1.0.3--1.0.4.sql
    sql/updates/h3--1.0.4--1.0.5.sql
    sql/updates/h3--1.0.5--1.0.6.sql
    sql/updates/h3--1.0.6--3.4.0.sql
    sql/updates/h3--3.4.0--3.4.1.sql
    sql/updates/h3--3.4.1--3.5.0.sql
    sql/updates/h3--3.5.0--3.6.0.sql
    sql/updates/h3--3.6.0--3.6.1.sql
    sql/updates/h3--3.6.1--3.6.2.sql
    sql/updates/h3--3.6.2--3.6.3.sql
    sql/updates/h3--3.6.3--3.6.4.sql
    sql/updates/h3--3.6.4--3.6.5.sql
    sql/updates/h3--3.6.5--3.7.0.sql
    sql/updates/h3--3.7.0--3.7.1.sql
    sql/updates/h3--3.7.1--3.7.2.sql
    sql/updates/h3--3.7.2--4.0.0.sql
    sql/updates/h3--4.0.0--4.0.1.sql
    sql/updates/h3--4.0.1--4.0.2.sql
    sql/updates/h3--4.0.2--4.0.3.sql
    sql/updates/h3--4.0.3--4.1.0.sql
    sql/updates/h3--4.1.0--4.1.1.sql
    sql/updates/h3--4.1.1--4.1.2.sql
    sql/updates/h3--4.1.2--4.1.3.sql
    sql/updates/h3--4.1.3--4.1.4.sql
    sql/updates/h3--4.1.4--4.2.0.sql
    sql/updates/h3--4.2.0--4.2.1.sql
    sql/updates/h3--4.2.1--4.2.2.sql
    sql/updates/h3--4.2.2--4.2.3.sql
)

# configure
configure_file(src/config.h.in src/config.h)

# include
target_include_directories(postgresql_h3 PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}/src
  src
)

# link
target_link_libraries(postgresql_h3 PRIVATE
  postgresql_h3_shared
  h3
)

# test
if(BUILD_TESTING)
  add_subdirectory(test)
endif()

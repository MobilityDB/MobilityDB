#!/bin/sh.exe

#-------------------------
# File used in Jenkis setup
#-------------------------

JENKINS_DEBUG=1

#-----------------
# variables setup
#  Variables recived by jenkins setup
#-----------------
#export OS_BUILD=
#export PG_VER=
#export PGHOST=
#export PGPORT=
#export MOBILITYDB_VER=
#export POSTIGS_VER=
#GCC_TYPE=
#export GIT_COMMIT=


if [ $JENKINS_DEBUG -eq 1 ]
then
    #---------------
    echo
    echo "***************************"
    echo Recived variables
    echo "**************************"
    #---------------
    echo "OS_BUILD ${OS_BUILD}"
    echo "PG_VER ${PG_VER}"
    echo "PGHOST ${PGHOST}"
    echo "PGPORT ${PGPORT}"
    echo "MOBILITYDB_VER ${MOBILITYDB_VER}"
    echo "POSTGIS_VER ${POSTGIS_VER}"
    echo "GCC_TYPE ${GCC_TYPE}"
    echo "GIT_COMMIT ${GIT_COMMIT}"
fi

#---------------
echo
echo "***************************"
echo Deduced variables
echo "***************************"
#---------------

export PGUSER=postgres
export PROJECTS=/projects
export PGPATHEDB=${PROJECTS}/postgresql/rel/pg${PG_VER}w${OS_BUILD}${GCC_TYPE}edb  #this is so winnie know's where to copy the dlls for vc++ edb compiled postgresql testing
export PGPATH=${PROJECTS}/postgresql/rel/pg${PG_VER}w${OS_BUILD}${GCC_TYPE}
export PATHOLD=$PATH
#export PATHOLD="/mingw/bin:/mingw/include:/c/Windows/system32:/c/Windows"
export PGWINVER=${PG_VER}w${OS_BUILD}${GCC_TYPE}edb
export PATH="${PATHOLD}:/usr/bin:${PGPATH}/bin:${PGPATH}/lib:${PGPATH}/include"
export PATH="${PROJECTS}/rel-libiconv-1.15.1w${OS_BUILD}${GCC_TYPE}/include:${PATH}"

#we may later pass in as variable in jenkins but hardcode for now
export GSL_VER=2.6
export GSL_PATH=/projects/gsl/rel-gsl-${GSL_VER}w${OS_BUILD}${GCC_TYPE}
export JSON_VER=0.12
export JSON_PATH=${PROJECTS}/json-c/rel-${JSON_VER}w${OS_BUILD}${GCC_TYPE}
export LIBLWGEOM_PATH=${PROJECTS}/postgis/pg${PG_VER}-liblwgeom-2.5w${OS_BUILD}${GCC_TYPE}
export PROTOBUF_VER=3.2.0
export PROTOBUF_PATH=${PROJECTS}/protobuf/rel-${PROTOBUF_VER}w${OS_BUILD}${GCC_TYPE}
export PROJ_VER=6.3.2
export PROJ_PATH=${PROJECTS}/proj/rel-${PROJ_VER}w${OS_BUILD}${GCC_TYPE}
export GEOS_VER=3.8
export GEOS_PATH=${PROJECTS}/geos/rel-${GEOS_VER}w${OS_BUILD}${GCC_TYPE}

#liblwgeom
export PATH="${PGPATH}/bin:${PGPATH}/lib:${PGPATH}/include:${LIBLWGEOM_PATH}/bin:${LIBLWGEOM_PATH}/lib:${LIBLWGEOM_PATH}/include:${PROJ_PATH}/include:${PROJ_PATH}/lib:${PROJ_PATH}/bin:${PATH}"
export PATH="${GEOS_PATH}/bin:{GEOS_PATH}/lib:${PATH}"


export PATH="${GSL_PATH}/lib:${GSL_PATH}/include:${JSON_PATH}/include:${JSON_PATH}/lib:${PATH}"

export PKG_CONFIG_PATH="${PROJ_PATH}/lib/pkgconfig"

if [ $JENKINS_DEBUG -eq 1 ]
then
    echo "PGUSER ${PGUSER}"
    echo "PROJECTS ${PROJECTS}"
    echo "PGPATHEDB ${PGPATHEDB}"
    echo "PGPATH ${PGPATH}"
    echo "PATHOLD ${PATHOLD}"
    echo "PGWINVER ${PGWINVER}"
    echo "PATH ${PATH}"
fi

if [ $JENKINS_DEBUG -eq 1 ]
then
    echo "ZLIB_VER ${ZLIB_VER}"
fi

#zlib
ZLIB_PATH="${PROJECTS}/zlib/rel-${ZLIB_VER}w${OS_BUILD}${GCC_TYPE}"
PATH="${PATH}:${ZLIB_PATH}/include:${ZLIB_PATH}/lib:${ZLIB_PATH}/bin"

#cmake
export PATH="${PATH}:/cmake/bin"
export PATH="${PATH}:.:/bin:/include"

cmake --version

echo "PATH ${PATH}"

cd "${PROJECTS}/mobilitydb" || exit 1
rm -rf "build${MOBILITYDB_VER}w${OS_BUILD}${GCC_TYPE}"
mkdir "build${MOBILITYDB_VER}w${OS_BUILD}${GCC_TYPE}"
cd "build${MOBILITYDB_VER}w${OS_BUILD}${GCC_TYPE}" || exit 1


#---------------
echo
echo "***************************"
echo "Current contents of PGPATH ${PGPATH}"
echo "***************************"
#---------------
ls "${PGPATH}/lib/libmobilitydb*"
ls "${PGPATH}/share/extension/mobilitydb*"

#---------------
echo
echo "***************************"
echo "Current contents of PGPATHEDB ${PGPATHEDB}"
echo "***************************"
#---------------
ls "${PGPATHEDB}/lib/libmobilitydb*"
ls "${PGPATHEDB}/share/extension/mobilitydb*"

rm "${PGPATH}/lib/libmobilitydb*"
rm "${PGPATH}/share/extension/mobilitydb*"
rm "${PGPATHEDB}/lib/libmobilitydb*"
rm "${PGPATHEDB}/share/extension/mobilitydb*"

#---------------
echo
echo "***************************"
echo "After removing in PGPATH ${PGPATH}"
echo "***************************"
#---------------
ls "${PGPATH}"/lib/libmobilitydb*
ls "${PGPATH}"/share/extension/mobilitydb*

#---------------
echo
echo "***************************"
echo "After removing in PGPATHEDB ${PGPATHEDB}"
echo "***************************"
#---------------
ls "${PGPATHEDB}"/lib/libmobilitydb*
ls "${PGPATHEDB}"/share/extension/mobilitydb*
cmake --version

cmake  -G "MSYS Makefiles" -DCMAKE_VERBOSE_MAKEFILE=ON \
  -DGSL_INCLUDE_DIR:PATH=${GSL_PATH}/include \
  -DLWGEOM_ROOT:PATH=${LIBLWGEOM_PATH} \
  -DCMAKE_BUILD_TYPE=Release \
  "../branches/${MOBILITYDB_VER}"

#---------------
echo
echo "***************************"
echo make
echo "***************************"
#---------------
make

#---------------
echo
echo "***************************"
echo make install
echo "***************************"
#---------------
make install

#---------------
echo
echo "***************************"
echo "Current contents of PGPATH ${PGPATH}"
echo "***************************"
#---------------
ls "${PGPATH}/lib/libmobilitydb*"
ls "${PGPATH}/share/extension/mobilitydb*"

#---------------
echo
echo "***************************"
echo "Current contents of PGPATHEDB ${PGPATHEDB}"
echo Should be empty
echo "***************************"
#---------------
ls "${PGPATHEDB}"/lib/libmobilitydb*
ls "${PGPATHEDB}"/share/extension/mobilitydb*


#we need uninstall and reinstall copy to VC++ EDB instance if we want to test on standard Windows installed versions
#cp *.dll ${PGPATHEDB}/lib/  #TODO remove this once we fix so the .dlls are created in lib folder
cp -r ${PGPATH}/lib/libmobilitydb*.dll ${PGPATHEDB}/lib/
cp -r ${PGPATH}/share/extension/mobilitydb*.sql ${PGPATHEDB}/share/extension/
cp -r ${PGPATH}/share/extension/mobilitydb.control ${PGPATHEDB}/share/extension/

#---------------
echo
echo "***************************"
echo "After copying to PGPATHEDB ${PGPATHEDB}"
echo "***************************"
#---------------
ls ${PGPATHEDB}/lib/libmobilitydb*
ls ${PGPATHEDB}/share/extension/mobilitydb*

cd "${PROJECTS}/mobilitydb/branches/${MOBILITYDB_VER}" || exit 1

# TODO cd to build directory before making the test
make test

cd "${PROJECTS}/mobilitydb/build${MOBILITYDB_VER}w${OS_BUILD}${GCC_TYPE}/lib" || exit 1
strip ./*.dll

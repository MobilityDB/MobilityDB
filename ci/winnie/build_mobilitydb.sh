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

cmake -G "MSYS Makefiles" -DCMAKE_VERBOSE_MAKEFILE=ON \
 -DBoost_USE_STATIC_LIBS=ON \
 -DBoost_USE_MULTITHREADED=ON \
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

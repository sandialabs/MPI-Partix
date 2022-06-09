#!/bin/bash

TYPE=$1
HOSTS=$2

CLEANUP="rm CMakeCache.txt"
HOSTNAME=`hostname | cut -f 1 -d "."`

# Some more info
DATE="echo `date`"
INFO="echo `mpirun -n 2 --host ${hosts} hostname`"


APP10=bench1
run_type="run_over_partsize.sh"

# ===================================================
# ===================================================
# benchmark="qtheads"
# ===================================================
# ===================================================

if [ x"$TYPE" = x1 ];
then

BACKEND="qthreads"

echo "RUNNING: $run_type with $BACKEND x86"
source ../../source_me.sh "$BACKEND"_5
$DATE
$INFO

$CLEANUP
LIB_INSTALL_PATH=/ascldap/users/jciesko/RAndD/PartionedCommunication/software/qthreads_x86/install
cmake .. -DCMAKE_CXX_COMPILER=mpicxx \
 -DCMAKE_LINKER=mpicxx \
 -DQthreads_ROOT=$LIB_INSTALL_PATH\
 -DPartix_ENABLE_QTHREADS=ON
make -j

bash ../scripts/$run_type $APP10 $HOSTS $BACKEND | tee ./result/"$APP10"_"$BACKEND"_"$run_type".res

fi

# ===================================================
# ===================================================
# benchmark="abt"
# ===================================================
# ===================================================

if [ x"$TYPE" = x2 ];
then

BACKEND="abt"

echo "RUNNING: $run_type with $BACKEND x86"
source ../../source_me.sh "$BACKEND"_5
$DATE
$INFO

$CLEANUP
LIB_INSTALL_PATH=/ascldap/users/jciesko/RAndD/PartionedCommunication/software/argobots_x86/install
echo `which mpicxx`
cmake .. -DCMAKE_CXX_COMPILER=mpicxx \
 -DCMAKE_LINKER=mpicxx \
 -DArgobots_ROOT=$LIB_INSTALL_PATH\
 -DPartix_ENABLE_ARGOBOTS=ON
make -j

bash ../scripts/$run_type $APP10 $HOSTS $BACKEND | tee ./result/"$APP10"_"$BACKEND"_"$run_type".res

fi

# ===================================================
# ===================================================
# benchmark="omp"
# ===================================================
# ===================================================

if [ x"$TYPE" = x3 ];
then

BACKEND="pthreads"

echo "RUNNING: $run_type with $BACKEND x86"
source ../../source_me.sh "$BACKEND"
$DATE
$INFO

$CLEANUP
cmake .. -DCMAKE_CXX_COMPILER=mpicxx \
 -DCMAKE_LINKER=mpicxx \
 -DPartix_ENABLE_OPENMP=ON 

make -j
bash ../scripts/$run_type $APP10 $HOSTS "omp"| tee ./result/"$APP10"_"OMP"_"$run_type".res

fi


# ===================================================
# ===================================================
# benchmark="pthreads"
# ===================================================
# ===================================================

if [ x"$TYPE" = x4 ];
then

BACKEND="pthreads"

echo "RUNNING: $run_type with $BACKEND x86"
source ../../source_me.sh "$BACKEND"
$DATE
$INFO

$CLEANUP
cmake .. -DCMAKE_CXX_COMPILER=mpicxx \
 -DCMAKE_LINKER=mpicxx \
 -DPartix_ENABLE_PTHREADS=ON

make -j
bash ../scripts/$run_type $APP10 $HOSTS $BACKEND| tee ./result/"$APP10"_"$BACKEND"_"$run_type".res

fi
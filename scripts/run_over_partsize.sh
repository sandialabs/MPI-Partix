# REPRODUCER SCRIPT

binary=$1
hosts=$2
backend=$3

if [ x"$binary" = x ]; then
  echo "Usage: sh run.sh binary hostlist type"
  echo "hostlist : host1,host2"
  exit 1
fi

start_total_length_kb=16 #16KB total
let total_partlen_elems_start=$start_total_length_kb*1024/8 #MPI_DOUBLE
num_tasks=1
num_threads=$num_tasks
num_part=$num_tasks

if [ x"$backend" = x"qthreads" ]; then
  QTHREADS_FLAGS="-x QTHREAD_AFFINITY=yes"
  RT_FLAGS=$QTHREADS_FLAGS
fi

if [ x"$backend" = x"abt" ]; then
  ABT_FLAGS=""
  RT_FLAGS=$ABT_FLAGS
fi

if [ x"$backend" = x"omp" ]; then
  OMP_FLAGS="-x OMP_NUM_THREADS=$num_threads -x OMP_PROC_BIND=true"
  RT_FLAGS=$OMP_FLAGS
fi

if [ x"$backend" = x"pthreads" ]; then
  PTHREADS_FLAGS=""
  RT_FLAGS=$PTHREADS_FLAGS
fi

MPIRUN_FLAGS="--bind-to numa --rank-by package --map-by ppr:1:node"

# Comment this out if not needed
PRELOAD="-x LD_PRELOAD=/home/projects/x86-64/gcc/10.2.0/lib64/libstdc++.so.6"

# Optionally use ob1 
USE_MCA_OB1=""#"OMPI_MCA_pml=ob1"

echo "tasks,threads,partitions,partsize(KB),totalsize(KB),time,bw(MB)"
for t in {1..9..1}; do 
  total_partlen_elems=$total_partlen_elems_start
  for size in {1..18..1}; do 
    let num_partlen=$total_partlen_elems/$num_tasks
     for reps in {1..3..1}; do
        mpirun -x $USE_MCA_OB1 --host $hosts \
          $MPIRUN_FLAGS $RT_FLAGS $PRELOAD\
          $binary $num_tasks $num_threads $num_part $num_partlen
      done
    let total_partlen_elems=$total_partlen_elems*2
  done
  let num_tasks=$num_tasks*2
  if [[ $num_threads -ge 48 ]] 
  then
    num_threads=48
  else
    num_threads=$num_tasks
  fi
  num_part=$num_tasks
done

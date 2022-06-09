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
  QTHREADS_FLAGS="" #"-x QT_CPUBIND=0,24,1,25,2,26,3,27,4,28,5,29,6,30,7,31,8,32,9,33,10,34,11,35,12,36,12,37,13,38,14,39,15,40,16,41,17,42,18,43,19,44,20,,4521,46,22,47,23"
  RT_FLAGS=$QTHREADS_FLAGS
fi

if [ x"$backend" = x"abt" ]; then
  ABT_FLAGS="-x ABT_SET_AFFINITY=\"{0,1,2,3},{4,5,6,7},{8,9,10,11}"
  RT_FLAGS=$ABT_FLAGS
fi

if [ x"$backend" = x"omp" ]; then
  OMP_FLAGS="-x OMP_NUM_THREADS=$num_threads -x OMP_PROC_BIND=true -x OMP_PLACES=cores"
  RT_FLAGS=$OMP_FLAGS
fi

if [ x"$backend" = x"pthreads" ]; then
  PTHREADS_FLAGS=""
  RT_FLAGS=$PTHREADS_FLAGS
fi

MPIRUN_FLAGS="--bind-to numa --rank-by core --map-by ppr:1:node"

overlap_default=1 #msec
PRELOAD="-x LD_PRELOAD=/home/projects/x86-64/gcc/10.2.0/lib64/libstdc++.so.6"

# Optionally use ob1 
USE_MCA_OB1=""#"OMPI_MCA_pml=ob1"

echo "tasks,threads,partitions,overlap,partsize(KB),totalsize(KB),time,bw(MB)"
#Run over threads
for threads in {1..9..1}; do 
  total_partlen_elems=$total_partlen_elems_start
  #Run over buffer size
  for size in {1..18..1}; do 
    let num_partlen=$total_partlen_elems/$num_tasks
    #Run over overlap duraton
    overlp=$overlap_default
    for overlap in {1..8..1}; do 
      let num_partlen=$total_partlen_elems/$num_tasks
      mpirun -x $USE_MCA_OB1 --host $hosts \
      $MPIRUN_FLAGS $RT_FLAGS $PRELOAD\
      $binary $num_tasks $num_threads $num_part $num_partlen $overlp
      if [[ $overlp -eq 1 ]] 
      then
        overlp=10
      fi
      let overlp=$overlp*2
    done
    let total_partlen_elems=$total_partlen_elems*2
    
  done
  let num_tasks=$num_tasks*2
  num_threads=$num_tasks
  num_part=$num_tasks
done

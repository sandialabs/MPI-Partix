# MPI Partix

MPI Partix is an application test suite for MPI implementations to test support for user-level threading (ULT) and partitioned communication (PC). A generic tasking interface abstracts from several threading libraries.

## Usage

This software suite uses the CMake build tool. Several CMake options exist:
```
- CMAKE_BUILD_TYPE
- CMAKE_INSTALL_PREFIX            
- Partix_ENABLE_ARGOBOTS
- Partix_ENABLE_OPENMP
- Partix_ENABLE_PTHREADS
- Partix_ENABLE_QTHREADS
```
Enabling the Qthreads or Argobots backends requires to set the library installation path using the cmake configure option ```Qthreads_ROOT``` or ```Argobots_ROOT```.

## Performance Insights

Several run scripts are included for reproducibility. Executing the included run scripts on a two-nodes dual-socket 24-core Intel(R) Xeon(R) Platinum 8160 CPU @ 2.10GHz and connected over Intel OmniPath results in the following performance charts. Markers are set at the 10GB/sec on the y-axes and 16KB on the x-axes.  The following charts show performance variations with a variable compute to communication ratio. In these cases, computation time ranges from zero to 160 ms. Compute phases can overlap with communication times is discussed in the next section.

<img src=https://user-images.githubusercontent.com/755191/167178079-111c8093-4af0-4b47-abfe-1c80f844f8a9.png width=100%>

It can be seen that different threading implementations result in comparable performances. This especially applies to executions where performance is determined mainly by access latency. Further, higher degrees of concurrency (line color) show performance degradation. This is subject to further analysis of the polling overhead in the Qtheads and OpenMP Tasking.

### Execution Scenarios
Partitioned communication helps to achieve overlapping computation and communication through partitioning data and by allowing the MPI runtime to consolidate actual transfers as partitioned are marked ready in the user's code. Several overlap scenarios exist which are shown below. Currently, the tested MPI implementation was not able to achieve significant performance improvements in either case. Achieving higher performance in these cases is again subject to further development work.

<img src=https://user-images.githubusercontent.com/755191/162831247-77b473e1-8d47-4d23-9339-08dfbaf10366.png width=55%>



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

Several run scripts are included for reproducibility. Executing the included run scripts on a two-nodes dual-socket 24-core Intel(R) Xeon(R) Platinum 8160 CPU @ 2.10GHz and connected over Intel OmniPath results in the following performance charts. The following charts show performance variations with a variable compute to communication ratio. In these cases, computation time ranges from zero to 160 ms. Compute phases can overlap with communication times as shown in the flow chart further below.

<img src=https://user-images.githubusercontent.com/755191/162831229-52e2f427-d92f-4050-bb31-7e330f80c682.png width=62%>

It can be seen that different threading implementations result in different performances. This especially applies to executions where performance is determined mainly by access latency. Further, higher degrees of concurrency (line color) show performance degradation. This is subject to optimizations of partitioned communication in the MPI runtime.

### Execution Scenarios
Partitioned communication helps to achieve overlapping computation and communication through partitioning data and by allowing the MPI runtime to consolidate actual transfers as partitioned are marked ready in the user's code. Several overlap scenarios exist which are shown below. Currently, the tested MPI implementation was not able to achieve significant performance improvements in either case. Achieving higher performance in these cases is again subject to further development work.
<img src=https://user-images.githubusercontent.com/755191/162831220-059980e3-f26a-4a9a-a9cd-f63ecab7a430.png width=90%>
<img src=https://user-images.githubusercontent.com/755191/162831247-77b473e1-8d47-4d23-9339-08dfbaf10366.png width=55%>



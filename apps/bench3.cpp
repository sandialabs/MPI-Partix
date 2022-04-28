//@HEADER
// ************************************************************************
//
//                        MPI Partix 1.0
//       Copyright 2022 National Technology & Engineering
//                Solutions of Sandia, LLC (NTESS).
// Under the terms of Contract DE-NA0003525 with NTESS,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY NTESS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL NTESS OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact Jan Ciesko (jciesko@sandia.gov)
//
// ************************************************************************
//@HEADER

/*
  Description:
  Similar to bench2 but delays n-1 tasks by a random ammount of time
*/

#include "mpi.h"
#include <cstdio>
#include <partix.h>
#include <stdlib.h>
#include <unistd.h>

#define DEFAULT_ITERS 5
#define DATA_TYPE MPI_DOUBLE
#define USE_PARRIVED

//#define ALL_TASKS_SLEEP_SAME_AMMOUNT

#define DEFAULT_RECV_SEND_PARTITION_RATIO 1

double timer[3] = {0.0, 0.0, 0.0};

/* My task args */
typedef struct {
  MPI_Request *request;
  int partition_id;
} send_task_args_t;

/* My task args */
typedef struct {
  MPI_Request *request;
  int recv_partitions;
  int partition_id;
} recv_task_args_t;

void recv_task(partix_task_args_t *args) {
  recv_task_args_t *task_args = (recv_task_args_t *)args->user_task_args;
  int cond1 = 0, cond2 = 0;
  int partition_id = task_args->partition_id;

  while (cond1 == 0 || cond2 == 0) {
    /* test partition_id #partition_id and #partition_id+1 */
    if (!cond1)
      MPI_Parrived(*task_args->request, partition_id, &cond1);
    if (partition_id + 1 < task_args->recv_partitions) {
      if (!cond2)
        MPI_Parrived(*task_args->request, partition_id + 1, &cond2);
    } else {
      cond2++;
    }
#if defined(OMP)
#pragma omp taskyield
#endif
  }
}

void send_task(partix_task_args_t *args) {
  send_task_args_t *task_args = (send_task_args_t *)args->user_task_args;

  size_t sleep_time_ms;
  sleep_time_ms = global_conf->overlap_duration;
#ifdef ALL_TASKS_SLEEP_SAME_AMMOUNT
  usleep(sleep_time_ms * 1000);
#else
  if (task_args->partition_id != 0) {
    // Sleep a random time in the noise_spread range of the overlap duration
    float sleep_time_ms_range =
        (float)sleep_time_ms * (float)global_conf->noise_spread / 100.0;
    sleep_time_ms = (int)(((rand() % 100 + 1) / 100.0) * sleep_time_ms_range);
  }
  usleep(sleep_time_ms * 1000);
#endif

  MPI_Pready(task_args->partition_id, *task_args->request);
}

int main(int argc, char *argv[]) {
  partix_config_t conf;
  partix_init(argc, argv, &conf);
  partix_library_init();

  int provided;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
  if (provided < MPI_THREAD_MULTIPLE)
    MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);

  MPI_Barrier(MPI_COMM_WORLD);

  int myrank;
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

  size_t iterations = DEFAULT_ITERS;
  size_t num_partitions = conf.num_partitions;
  size_t num_partlength = conf.num_partlength;

  MPI_Request request;
  MPI_Info info = MPI_INFO_NULL;
  MPI_Datatype send_xfer_type;
  MPI_Datatype recv_xfer_type;

  MPI_Count send_partitions = num_partitions;
  MPI_Count send_partlength = num_partlength;

  MPI_Count recv_partitions =
      num_partitions * DEFAULT_RECV_SEND_PARTITION_RATIO;
  MPI_Count recv_partlength =
      num_partlength / DEFAULT_RECV_SEND_PARTITION_RATIO;

  double *message = new double[num_partitions * num_partlength];

  int count = 1, source = 0, dest = 1, tag = 1, flag = 0;

  MPI_Type_contiguous(send_partlength, DATA_TYPE, &send_xfer_type);
  MPI_Type_contiguous(recv_partlength, DATA_TYPE, &recv_xfer_type);
  MPI_Type_commit(&send_xfer_type);
  MPI_Type_commit(&recv_xfer_type);

  for (int i = 0; i < iterations; ++i) {
    // Benchmark iteration
    {
      /* Rank 0 */

      if (myrank == 0) {

        send_task_args_t *send_args = (send_task_args_t *)calloc(
            send_partitions, sizeof(send_task_args_t));

        MPI_Psend_init(message, send_partitions, count, send_xfer_type, dest,
                       tag, MPI_COMM_WORLD, info, &request);

        double start_time = MPI_Wtime();
        MPI_Start(&request);

        // set context
        partix_context_t ctx;

#if defined(OMP)
#pragma omp parallel num_threads(conf.num_threads)
#pragma omp single
#endif
        for (int j = 0; j < send_partitions; ++j) {
          send_args[j].request = &request;
          send_args[j].partition_id = j;
          partix_task(&send_task /*functor*/, &send_args[j] /*capture*/, &ctx);
        }
        partix_taskwait(&ctx);
        while (!flag) {
          /* Do useful work */
          MPI_Test(&request, &flag, MPI_STATUS_IGNORE);
          /* Do useful work */
        }

        double delta_time = MPI_Wtime() - start_time;
        timer[0] += delta_time;

        MPI_Request_free(&request);
        free(send_args);
      } else if (myrank == 1) {

        /* Rank 1 */

        recv_task_args_t *recv_args = (recv_task_args_t *)calloc(
            recv_partitions, sizeof(recv_task_args_t));

        MPI_Precv_init(message, recv_partitions, count, recv_xfer_type, source,
                       tag, MPI_COMM_WORLD, info, &request);
        double start_time = MPI_Wtime();
        MPI_Start(&request);

#if defined(USE_PARRIVED)
        // set context
        partix_context_t ctx;
#if defined(OMP)
#pragma omp parallel num_threads(conf.num_threads)
#pragma omp single
#endif
        for (int j = 0; j < recv_partitions; j += 2) {
          recv_args[j].recv_partitions = recv_partitions;
          recv_args[j].request = &request;
          recv_args[j].partition_id = j;
          partix_task(&recv_task /*functor*/, &recv_args[j] /*capture*/, &ctx);
        }
        partix_taskwait(&ctx);
#else
        while (!flag) {
          /* Do useful work */
          MPI_Test(&request, &flag, MPI_STATUS_IGNORE);
          /* Do useful work */
        }
#endif

        double delta_time = MPI_Wtime() - start_time;
        timer[1] += delta_time;

        MPI_Request_free(&request);
        free(recv_args);
      }
      MPI_Barrier(MPI_COMM_WORLD);
    }
  }

  // Measure perceived BW, that is communication as it were in the critical
  // path, by subtracting overlap
  timer[0] -= iterations * (float)global_conf->overlap_duration / 1000;
  timer[1] -= iterations * (float)global_conf->overlap_duration / 1000;

  timer[0] /= iterations;
  timer[1] /= iterations;

  size_t patition_size_bytes = num_partlength * sizeof(DATA_TYPE);
  size_t total_size_bytes = num_partitions * patition_size_bytes;

  if (myrank == 0) {
    double send_BW = total_size_bytes / timer[0] / 1024 / 1024;
#if true
    printf("%i, %i, %i, %.3f, %.2f, %.2f, %.2f, %.2f\n", conf.num_tasks,
           conf.num_threads, conf.num_partitions,
           (float)global_conf->overlap_duration / 1000.0,
           ((double)patition_size_bytes) / 1024,
           ((double)total_size_bytes) / 1024, timer[0] /*rank0*/, send_BW);
#endif
  } else {
#if false
    double recv_BW = total_size_bytes / timer[1] / 1024 / 1024;
    printf("%i, %i, %i, %.3f, %.2f, %.2f, %.2f, %.2f\n", 
          conf.num_tasks,
          conf.num_threads,
          conf.num_partitions,
          (float)global_conf->overlap_duration / 1000.0,
          ((double)patition_size_bytes) / 1024,
          ((double)total_size_bytes) / 1024,
          timer[1] /*rank1*/,
          recv_BW);
#endif
  }

  MPI_Type_free(&send_xfer_type);
  MPI_Type_free(&recv_xfer_type);
  delete[] message;

  MPI_Finalize();
  partix_library_finalize();
  return 0;
}

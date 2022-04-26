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

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#include <partix.h>

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
  int cond = 0;
  int partition_id = task_args->partition_id;

  while (cond == 0) {
    MPI_Parrived(*task_args->request, partition_id, &cond);
  }
}

void send_task(partix_task_args_t *args) {
  send_task_args_t *task_args = (send_task_args_t *)args->user_task_args;
  MPI_Pready(task_args->partition_id, *task_args->request);
}

int main(int argc, char *argv[]) {
  partix_config_t conf;
  partix_init(argc, argv, &conf);
  partix_library_init();

  MPI_Count send_partitions = conf.num_partitions;
  MPI_Count send_partlength = conf.num_partlength;
  MPI_Count recv_partitions = send_partitions;
  MPI_Count recv_partlength = send_partlength;

  double *message = new double[send_partitions * send_partlength];

  int count = 1, source = 0, dest = 1, tag = 1, flag = 0;
  int myrank;
  int provided;

  MPI_Request request;
  MPI_Info info = MPI_INFO_NULL;
  MPI_Datatype send_xfer_type;
  MPI_Datatype recv_xfer_type;

  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
  if (provided < MPI_THREAD_MULTIPLE)
    MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  MPI_Type_contiguous(send_partlength, MPI_DOUBLE, &send_xfer_type);
  MPI_Type_contiguous(recv_partlength, MPI_DOUBLE, &recv_xfer_type);
  MPI_Type_commit(&send_xfer_type);
  MPI_Type_commit(&recv_xfer_type);

  send_task_args_t *send_args =
      (send_task_args_t *)calloc(send_partitions, sizeof(send_task_args_t));
  recv_task_args_t *recv_args =
      (recv_task_args_t *)calloc(recv_partitions, sizeof(recv_task_args_t));

  if (myrank == 0) {
    MPI_Psend_init(message, send_partitions, count, send_xfer_type, dest, tag,
                   MPI_COMM_WORLD, info, &request);
    MPI_Start(&request);

    // set context
    partix_context_t ctx;

#if defined(OMP)
#pragma omp parallel num_threads(conf.num_threads)
#pragma omp single
#endif
    for (int i = 0; i < send_partitions; ++i) {
      send_args[i].request = &request;
      send_args[i].partition_id = i;
      partix_task(&send_task /*functor*/, &send_args[i] /*capture*/, &ctx);
    }
    partix_taskwait(&ctx);
    while (!flag) {
      /* Do useful work */
      MPI_Test(&request, &flag, MPI_STATUS_IGNORE);
      /* Do useful work */
    }
    MPI_Request_free(&request);
  } else if (myrank == 1) {
    MPI_Precv_init(message, recv_partitions, count, recv_xfer_type, source, tag,
                   MPI_COMM_WORLD, info, &request);
    MPI_Start(&request);

    // set context
    partix_context_t ctx;

#if defined(OMP)
#pragma omp parallel num_threads(conf.num_threads)
#pragma omp single
#endif
    for (int i = 0; i < recv_partitions; ++i) {
      recv_args[i].recv_partitions = recv_partitions;
      recv_args[i].request = &request;
      recv_args[i].partition_id = i;
      partix_task(&recv_task /*functor*/, &recv_args[i] /*capture*/, &ctx);
    }
    partix_taskwait(&ctx);

    while (!flag) {
      /* Do useful work */
      MPI_Test(&request, &flag, MPI_STATUS_IGNORE);
      /* Do useful work */
    }
    MPI_Request_free(&request);
  }

  free(send_args);
  free(recv_args);
  delete[] message;
  MPI_Finalize();
  partix_library_finalize();

  return 0;
}

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

/* Reference code based on examples in the Open MPI 4.0 spec */

#include "mpi.h"
#include <stdlib.h>

#include <partix.h>

int main(int argc, char *argv[]) {
  partix_config_t conf;
  partix_init(argc, argv, &conf);
  partix_library_init();

  MPI_Count partitions = conf.num_partitions;
  MPI_Count partlength = conf.num_partlength;

  double *message = new double[partitions * partlength];
  int source = 0, dest = 1, tag = 1, flag = 0;
  int myrank, i;
  int provided;
  MPI_Request request;

  MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED, &provided);
  if (provided < MPI_THREAD_SERIALIZED)
    MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);

  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

  if (myrank == 0) {
    MPI_Psend_init(message, partitions, partlength, MPI_DOUBLE, dest, tag,
                   MPI_COMM_WORLD, MPI_INFO_NULL, &request);
    MPI_Start(&request);
    for (i = 0; i < partitions; ++i) {
      /* compute and fill partition #i, then mark ready: */
      partix_add_noise();
      MPI_Pready(i, request);
    }
    while (!flag) {
      /* do useful work #1 */
      MPI_Test(&request, &flag, MPI_STATUS_IGNORE);
      /* do useful work #2 */
    }
    MPI_Request_free(&request);
  } else if (myrank == 1) {
    MPI_Precv_init(message, partitions, partlength, MPI_DOUBLE, source, tag,
                   MPI_COMM_WORLD, MPI_INFO_NULL, &request);
    MPI_Start(&request);
    while (!flag) {
      /* do useful work #1 */
      MPI_Test(&request, &flag, MPI_STATUS_IGNORE);
      /* do useful work #2 */
    }
    MPI_Request_free(&request);
  }

  delete[] message;
  MPI_Finalize();
  partix_library_finalize();
  return 0;
}
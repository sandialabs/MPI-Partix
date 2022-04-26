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
#include <cassert>
#include <cstdio>
#include <cstdlib>

#include <partix.h>

#define DEFAULT_VALUE 123

int reduction_var = 0;

/* My task args */
typedef struct {
  int some_data;
  int iters;
} task_args_t;

void task_inner(partix_task_args_t *args) {
  task_args_t *task_args = (task_args_t *)args->user_task_args;
  printf("APITest2: Printing: %i on thread %u.\n", task_args->some_data,
         partix_executor_id());
  partix_mutex_enter();
  reduction_var += task_args->some_data;
  partix_mutex_exit();
}

void task_outer(partix_task_args_t *args) {
  task_args_t *task_args = (task_args_t *)args->user_task_args;

  // set context
  partix_context_t ctx;

  for (int i = 0; i < task_args->iters; ++i) {
    partix_task(&task_inner /*functor*/, task_args /*capture by ref*/, &ctx);
  }
  partix_taskwait(&ctx);
}

int main(int argc, char *argv[]) {
  partix_config_t conf;
  partix_init(argc, argv, &conf);
  partix_library_init();
  task_args_t task_args;
  task_args.some_data = DEFAULT_VALUE;
  task_args.iters = 4;

  // set context
  partix_context_t ctx;

#if defined(OMP)
#pragma omp parallel num_threads(conf.num_threads)
#pragma omp single
#endif

  for (int i = 0; i < conf.num_tasks; ++i) {
    partix_task(&task_outer /*functor*/, &task_args /*capture by ref*/, &ctx);
  }

  partix_taskwait(&ctx);

  assert(reduction_var == DEFAULT_VALUE * conf.num_tasks * task_args.iters);

  partix_library_finalize();
  return 0;
}

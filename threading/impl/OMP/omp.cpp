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

#include <stdlib.h>
#include <thread.h>

partix_mutex_t global_mutex;
partix_config_t *global_conf;

void partix_task(void (*f)(partix_task_args_t *), void *args,
                 partix_context_t *ctx) {
#pragma omp task
  {
    partix_task_args_t partix_args;
    partix_args.user_task_args = args;
    f(&partix_args);
  }
}

void partix_taskwait(partix_context_t *ctx) {
#pragma omp taskwait
}

void partix_mutex_init(partix_mutex_t *m) { omp_init_lock(m); };
void partix_mutex_destroy(partix_mutex_t *m) { omp_destroy_lock(m); };

void partix_mutex_enter(partix_mutex_t *m) { omp_set_lock(m); };

void partix_mutex_exit(partix_mutex_t *m) { omp_unset_lock(m); };

void partix_mutex_init() { omp_init_lock(&global_mutex); };
void partix_mutex_destroy() { omp_destroy_lock(&global_mutex); };

void partix_mutex_enter(void) { omp_set_lock(&global_mutex); };

void partix_mutex_exit(void) { omp_unset_lock(&global_mutex); };

int partix_executor_id(void) { return omp_get_thread_num(); };

void partix_library_init(void) { ; /* Empty. */ }

void partix_library_finalize(void) { ; /* Empty. */ }

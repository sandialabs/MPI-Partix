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

#ifndef __PARTIX_THREAD_H__
#define __PARTIX_THREAD_H__

#include <generic_types.h>
#include <types.h>

#if defined(DEBUG)
#define debug(m) printf("%s\n", m)
#else
#define debug(m)
#endif

void partix_task(void (*)(partix_task_args_t *),
                                           void *, partix_context_t *);
void partix_taskwait(partix_context_t *);

void partix_library_init(void);
void partix_library_finalize(void);

void partix_mutex_enter(void);
void partix_mutex_exit(void);

void partix_mutex_enter(partix_mutex_t *);
void partix_mutex_exit(partix_mutex_t *);

void partix_mutex_init(partix_mutex_t *);
void partix_mutex_destroy(partix_mutex_t *);

int partix_executor_id(void);

/*Non-conexted functions are deprecated*/
__attribute__((noinline)) void partix_task(void (*)(partix_task_args_t *),
                                           void *);
__attribute__((noinline)) void partix_taskwait(void);

#endif /* __PARTIX_THREAD_H__ */

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

#ifndef __PARTIX_H__
#define __PARTIX_H__

#include <stdlib.h>
#include <assert.h>

/* Number of workers in ULT libraries and OpenMP */
#define NUM_THREADS_DEFAULT 2

/* Number of tasks in ULT libraries and OpenMP
   Number of threads with Pthreads */
#define NUM_TASKS_DEFAULT NUM_THREADS_DEFAULT * 8

/* Per default, numer of partitions is the number of tasks */
#define PARTITIONS_DEFAULT NUM_TASKS_DEFAULT

/* Num elements of MPI_TYPE
   For MPI_DOUBLE, this creates a partition of 128 bytes */
#define PARTLENGTH_DEFAULT 16

/* Used to simulate computation and communication overlap */
#define OVERLAP_IN_MSEC_DEFAULT 100

/* Used add task duration divergence as a % of OVERLAP_IN_MSEC_DEFAULT */
#define NOISE_IN_PERCENTAGE_OF_OVERLAP 80

#include <thread.h>

#define DEFAULT_CONF_VAL 1

extern partix_config_t *global_conf;

static void partix_init(int argc, char **argv, partix_config_t *conf) {
  conf->num_tasks = argc > 1 ? atoi(argv[1]) : NUM_TASKS_DEFAULT;
  conf->num_threads = argc > 2 ? atoi(argv[2]) : NUM_THREADS_DEFAULT;
  conf->num_partitions = argc > 3 ? atoi(argv[3]) : PARTITIONS_DEFAULT;
  conf->num_partlength = argc > 4 ? atoi(argv[4]) : PARTLENGTH_DEFAULT;
  conf->overlap_duration = argc > 5 ? atoi(argv[5]) : OVERLAP_IN_MSEC_DEFAULT;
  conf->noise_spread = argc > 6 ? atoi(argv[6]) : NOISE_IN_PERCENTAGE_OF_OVERLAP;
  
  assert(conf->num_partitions>=conf->num_tasks);
  assert(conf->num_partlength>0);

  /* conf object duration should be valid until partix_finalize() */
  global_conf = conf;

  /* Propate process args */
  conf->argc = argc;
  conf->argv = argv;
}

static void partix_finalize() { ; /* Empty. */ }

void partix_add_noise() { ;/*Unused*/ }

#endif /* __PARTIX_H__*/

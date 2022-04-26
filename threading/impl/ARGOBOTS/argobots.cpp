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

#include <assert.h>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <thread.h>

#define SUCCEED(val) assert(val == ABT_SUCCESS)
#define MAX_TASKS_PER_TW 1024

partix_mutex_t global_mutex;
partix_mutex_t context_mutex;
partix_config_t *global_conf;

// Global vars
void create_scheds(int num, ABT_pool *pools, ABT_sched *scheds);
void create_threads(void *arg);

typedef struct {
  uint32_t event_freq;
} sched_data_t;

typedef struct {
  ABT_thread thread;
  partix_task_args_t args;
} thread_handle_t;

/* We need this to implement taskwait*/
typedef struct {
  thread_handle_t threadHandle[MAX_TASKS_PER_TW];
  int context_task_counter;
} partix_handle_t;

typedef struct abt_global_t {
  int num_xstreams;
  ABT_xstream *xstreams;
  ABT_sched *scheds;
  ABT_pool *pools;
  ABT_thread *threads;
  /* ABT_xstream_barrier xstream_barrier;*/
} abt_global_t;
abt_global_t abt_global;

struct barrier_handle_t {
  ABT_barrier barrier;
};

typedef std::map<size_t, partix_handle_t *> partix_context_map_t;
partix_context_map_t context_map;

thread_handle_t *register_task(size_t context) {
  partix_context_map_t::iterator it;
  partix_mutex_enter(&context_mutex);
  it = context_map.find(context);
  if (it != context_map.end()) {
    partix_handle_t *context_handle = it->second;
    debug("register_task, it != context_map.end()");
    if (context_handle->context_task_counter >= MAX_TASKS_PER_TW) {
      debug("Error: context_handle->context_task_counter > MAX_TASKS_PER_TW");
      // TBD: return here an invalid threadHandle and stop generating tasks
      exit(1);
    }
    partix_mutex_exit(&context_mutex);
    return &context_handle
                ->threadHandle[context_handle->context_task_counter++];
  } else {
    partix_handle_t *context_handle =
        (partix_handle_t *)calloc(1, sizeof(partix_handle_t));
    const auto it_insert = context_map.insert(
        std::pair<size_t, partix_handle_t *>(context, context_handle));
    if (it_insert.second) { /*Insert successful*/
    }
    debug("register_task, it == context_map.end()");
    partix_mutex_exit(&context_mutex);
    return &context_handle
                ->threadHandle[context_handle->context_task_counter++];
  }
  assert(false); // DO NOT REACH HERE
}

void partix_sched_run(ABT_sched sched) {
  uint32_t work_count = 0;
  sched_data_t *p_data;
  int num_pools;
  ABT_pool *pools;
  int target;
  ABT_bool stop;
  unsigned seed = time(NULL);

  ABT_sched_get_data(sched, (void **)&p_data);
  ABT_sched_get_num_pools(sched, &num_pools);
  pools = (ABT_pool *)malloc(num_pools * sizeof(ABT_pool));
  ABT_sched_get_pools(sched, num_pools, 0, pools);

  debug("partix_sched_run while(1)");
  while (1) {
    /* Execute one work unit from the scheduler's pool */
    ABT_thread thread;
    ABT_pool_pop_thread(pools[0], &thread);
    if (thread != ABT_THREAD_NULL) {
      /* "thread" is associated with its original pool (pools[0]). */
      ABT_self_schedule(thread, ABT_POOL_NULL);
    } else if (num_pools > 1) {
      /* Steal a work unit from other pools */
      target = (num_pools == 2) ? 1 : (rand_r(&seed) % (num_pools - 1) + 1);
      ABT_pool_pop_thread(pools[target], &thread);
      if (thread != ABT_THREAD_NULL) {
        /* "thread" is associated with its original pool
         * (pools[target]). */
        ABT_self_schedule(thread, pools[target]);
      }
    }

    if (++work_count >= p_data->event_freq) {
      work_count = 0;
      ABT_sched_has_to_stop(sched, &stop);
      if (stop == ABT_TRUE)
        break;
      ABT_xstream_check_events(sched);
    }
  }

  debug("free pools");
  free(pools);
}

int partix_sched_free(ABT_sched sched) {
  sched_data_t *p_data;

  debug("ABT_sched_get_data");
  ABT_sched_get_data(sched, (void **)&p_data);
  free(p_data);

  return ABT_SUCCESS;
}

int partix_sched_init(ABT_sched sched, ABT_sched_config config) {

  sched_data_t *p_data = (sched_data_t *)calloc(1, sizeof(sched_data_t));

  ABT_sched_config_read(config, 1, &p_data->event_freq);
  ABT_sched_set_data(sched, (void *)p_data);

  return ABT_SUCCESS;
}

void create_scheds(int num, ABT_pool *pools, ABT_sched *scheds) {
  ABT_sched_config config;
  ABT_pool *my_pools;
  int i, k;

  ABT_sched_config_var cv_event_freq = {.idx = 0, .type = ABT_SCHED_CONFIG_INT};

  ABT_sched_def sched_def = {.type = ABT_SCHED_TYPE_ULT,
                             .init = partix_sched_init,
                             .run = partix_sched_run,
                             .free = partix_sched_free,
                             .get_migr_pool = NULL};

  /* Create a scheduler config */
  debug("ABT_sched_config_create");
  ABT_sched_config_create(&config, cv_event_freq, 10, ABT_sched_config_var_end);

  my_pools = (ABT_pool *)malloc(num * sizeof(ABT_pool));
  debug("ABT_sched_create");
  for (i = 0; i < num; i++) {
    for (k = 0; k < num; k++) {
      my_pools[k] = pools[(i + k) % num];
    }
    ABT_sched_create(&sched_def, num, my_pools, config, &scheds[i]);
  }
  free(my_pools);
  ABT_sched_config_free(&config);
}

void partix_mutex_enter() {
  debug("partix_mutex_enter");
  int ret = ABT_mutex_lock(global_mutex);
  SUCCEED(ret);
}

void partix_mutex_exit() {
  debug("partix_mutex_exit");
  int ret = ABT_mutex_unlock(global_mutex);
  SUCCEED(ret);
}

void partix_mutex_enter(partix_mutex_t *m) {
  debug("partix_mutex_enter");
  int ret = ABT_mutex_lock(*m);
  SUCCEED(ret);
}

void partix_mutex_exit(partix_mutex_t *m) {
  debug("partix_mutex_exit");
  int ret = ABT_mutex_unlock(*m);
  SUCCEED(ret);
}

void partix_mutex_init(partix_mutex_t *m) {
  debug("partix_mutex_init");
  int ret = ABT_mutex_create(m);
  SUCCEED(ret);
}

void partix_mutex_destroy(partix_mutex_t *m) {
  debug("partix_mutex_destroy");
  int ret = ABT_mutex_free(m);
  SUCCEED(ret);
}

void partix_barrier_init(int num_waiters, barrier_handle_t *p_barrier) {
  int ret;
  debug("partix_barrier_init");
  ret = ABT_barrier_create(num_waiters, &p_barrier->barrier);
  SUCCEED(ret);
}

void partix_barrier_wait(barrier_handle_t *p_barrier) {
  int ret;
  debug("partix_barrier_wait");
  ret = ABT_barrier_wait(p_barrier->barrier);
  SUCCEED(ret);
}

void partix_barrier_destroy(barrier_handle_t *p_barrier) {
  int ret;
  debug("partix_barrier_destroy");
  ret = ABT_barrier_free(&p_barrier->barrier);
  SUCCEED(ret);
}

int partix_executor_id(void) {
  debug("partix_executor_id");
  ABT_thread thread;
  ABT_unit_id thread_id;
  ABT_thread_self(&thread);
  ABT_thread_get_id(thread, &thread_id);
  return (int)thread_id;
};

void partix_library_init(void) {
  int ret;
  ABT_init(0, NULL);

  partix_mutex_init(&global_mutex);
  partix_mutex_init(&context_mutex);

  int num_xstreams;
  if (getenv("ABT_NUM_XSTREAMS")) {
    num_xstreams = atoi(getenv("ABT_NUM_XSTREAMS"));
    if (num_xstreams < 0)
      num_xstreams = 1;
  } else {
    num_xstreams = global_conf->num_threads;
  }

  abt_global.num_xstreams = num_xstreams;

  abt_global.xstreams =
      (ABT_xstream *)malloc(num_xstreams * sizeof(ABT_xstream));
  abt_global.scheds = (ABT_sched *)malloc(num_xstreams * sizeof(ABT_sched));
  abt_global.pools = (ABT_pool *)malloc(num_xstreams * sizeof(ABT_pool));
  abt_global.threads = (ABT_thread *)malloc(num_xstreams * sizeof(ABT_thread));

  /* Create pools */
  debug("ABT_pool_create_basic");
  for (int i = 0; i < num_xstreams; i++) {
    ret = ABT_pool_create_basic(ABT_POOL_FIFO, ABT_POOL_ACCESS_MPMC, ABT_TRUE,
                                &abt_global.pools[i]);
    SUCCEED(ret);
  }

  /* Create schedulers */
  create_scheds(num_xstreams, abt_global.pools, abt_global.scheds);

  /* Create ESs */

  debug("ABT_xstream_create");
  for (int i = 1; i < num_xstreams; i++) {
    ret = ABT_xstream_create(abt_global.scheds[i], &abt_global.xstreams[i]);
    SUCCEED(ret);
  }

  /* Set up a primary execution stream. */
  ret = ABT_xstream_self(&abt_global.xstreams[0]);
  SUCCEED(ret);
  ret =
      ABT_xstream_set_main_sched(abt_global.xstreams[0], abt_global.scheds[0]);
  SUCCEED(ret);

  /* Execute a scheduler once. */
  debug("ABT_self_yield");
  ret = ABT_self_yield();
  SUCCEED(ret);
}

void partix_library_finalize(void) {
  int ret;

  debug("ABT_xstream_join");
  for (int i = 1; i < abt_global.num_xstreams; i++) {
    ABT_xstream_join(abt_global.xstreams[i]);
    ABT_xstream_free(&abt_global.xstreams[i]);
  }

  partix_mutex_destroy(&global_mutex);
  partix_mutex_destroy(&context_mutex);

  debug("ABT_sched_free");
  for (int i = 1; i < abt_global.num_xstreams; i++) {
    ret = ABT_sched_free(&abt_global.scheds[i]);
    SUCCEED(ret);
  }

  free(abt_global.xstreams);
  free(abt_global.scheds);
  free(abt_global.pools);
  free(abt_global.threads);

  debug("ABT_finalize");
  ret = ABT_finalize();
  SUCCEED(ret);
}

void partix_thread_create(void (*f)(partix_task_args_t *), void *args,
                          ABT_thread *handle) {
  int ret, rank;
  debug("ABT_self_get_xstream_rank");
  ret = ABT_self_get_xstream_rank(&rank);
  SUCCEED(ret);
  ABT_pool pool = abt_global.pools[rank];
  debug("ABT_thread_create");
  ret = ABT_thread_create(pool, (void (*)(void *))f, args, ABT_THREAD_ATTR_NULL,
                          handle);
  SUCCEED(ret);
}

void partix_thread_join(ABT_thread handle) {
  int ret;
  debug("partix_thread_join");
  ret = ABT_thread_free(&handle);
  SUCCEED(ret);
}

__attribute__((noinline)) void partix_task(void (*f)(partix_task_args_t *),
                                           void *user_args,
                                           partix_context_t *ctx) {
  size_t context = (size_t)ctx;
  thread_handle_t *threadhandle = register_task(context);
  partix_task_args_t *partix_args = &threadhandle->args;
  partix_args->user_task_args = user_args;
  partix_args->conf = global_conf;
  debug("partix_task");
  partix_thread_create(f, partix_args, &threadhandle->thread);
}

__attribute__((noinline)) void partix_taskwait(partix_context_t *ctx) {
  size_t context = (size_t)ctx;
  partix_context_map_t::iterator it;
  it = context_map.find(context);
  if (it == context_map.end()) {
    debug("partix_taskwait, it == context_map.end()");
    return;
  }
  debug("partix_taskwait, it != context_map.end()");

  partix_handle_t *context_handle = it->second;
  for (int i = 0; i < context_handle->context_task_counter; ++i) {
    debug("partix_taskwait, partix_thread_join");
    partix_thread_join(context_handle->threadHandle[i].thread);
  }
  free(context_handle);
  partix_mutex_enter(&context_mutex);
  context_map.erase(it);
  partix_mutex_exit(&context_mutex);
}
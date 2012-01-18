/*!
 * pThread pool
 *
 * This file is part of the
 * Comprehensive and Sensitive Signature Search (CaSSiS) library.
 *
 * Copyright (C) 2011,2012
 *     Kai Christian Bader <mail@kaibader.de>
 *     Christian Grothoff <christian@grothoff.org>
 *
 * CaSSiS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * CaSSiS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with CaSSiS.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "pool.h"

#include <stdio.h>

#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <cstdlib>

// TODO: Remove usage of INT_MAX as soon as possible.
#ifndef INT_MAX
#define INT_MAX (2147483647)
#endif

/*!
 * Returns the number of (real) processors or, if defined, the number
 * from the 'PTHREADS_NUM_THREADS' environment variable.
 */
int num_processors() {
    // int cpus = sysconf(_SC_NPROCESSORS_ONLN);
    int cpus = sysconf(_SC_NPROCESSORS_CONF);

    const char *num_threads_env = getenv("PTHREADS_NUM_THREADS");
    if (num_threads_env) {
        char *tail;
        int num_threads = strtol(num_threads_env, &tail, 10);
        if (tail == num_threads_env || num_threads == 0 || num_threads > INT_MAX) {
            cpus = num_threads;
        }
    }

    return cpus;
}

/*!
 * Internal state of a semaphore.
 */
typedef struct Semaphore_st {
    int v; // Counter.
    pthread_mutex_t mutex; // Mutex.
    pthread_cond_t cond; // Wrapper for pThread condition variable.
} Semaphore_t;

/*!
 * Function must be called prior to semaphore use.
 * Handles setup and initialization.
 */
static Semaphore_t *semaphore_create(int value) {
    Semaphore_t *s = (Semaphore_t*) malloc(sizeof(Semaphore_t));
    s->v = value;
    pthread_mutex_init(&s->mutex, NULL);
    pthread_cond_init(&s->cond, NULL);
    return s;
}

/*!
 * Semaphore destroy should be called when the semaphore is no longer needed.
 */
static void semaphore_destroy(Semaphore_t *s) {
    pthread_cond_destroy(&s->cond);
    pthread_mutex_destroy(&s->mutex);
    free(s);
}

/*!
 * Count semaphore up.
 */
static int semaphore_up(Semaphore_t *s) {
    int ret = 0;
    pthread_mutex_lock(&s->mutex);
    ret = ++(s->v);
    pthread_cond_signal(&s->cond);
    pthread_mutex_unlock(&s->mutex);
    return ret;
}

/*!
 * Count semaphore down.
 */
static int semaphore_down(Semaphore_t *s) {
    int ret = 0;
    pthread_mutex_lock(&s->mutex);
    while (s->v <= 0)
        pthread_cond_wait(&s->cond, &s->mutex);
    ret = --(s->v);
    pthread_mutex_unlock(&s->mutex);
    return ret;
}

/*!
 * Work structure.
 */
typedef struct Work_st {
    struct Work_st *next;
    WorkFun fun;
    void *arg;
} Work_t;

/*!
 * Thread pool...
 */
int num_threads; // Number of threads.
Work_t *work_head; // Work queue head pointer.
Work_t *work_tail; // Work queue tail pointer.
Semaphore_t *work_pending; // Semaphore indicating pending work.
pthread_mutex_t work_lock; // Lock for adding/fetching work.

/*!
 * Schedule the given work function to be run from the
 */
void pool_run(WorkFun fun, void *arg) {
    Work_t *work = (Work_t *) malloc(sizeof(Work_t));
    work->next = NULL;
    work->fun = fun;
    work->arg = arg;

    pthread_mutex_lock(&(work_lock));
    if (work_head == NULL) {
        work_head = work;
        work_tail = work;
    } else {
        work_tail->next = work;
        work_tail = work;
    }
    pthread_mutex_unlock(&(work_lock));
    semaphore_up(work_pending);
}

static void *runner(void *) {
    while (1) {
        semaphore_down(work_pending);
        pthread_mutex_lock(&(work_lock));
        Work_t *work = work_head;
        if (work)
            work_head = work->next;
        pthread_mutex_unlock(&(work_lock));

        // Exit, if the 'work_pending' semaphore was increased without
        // having added work.
        if (work == NULL) {
            pthread_exit(NULL);
        }

        work->fun(work->arg);
        free(work);
    }
    return NULL; // Should not be reached...
}

struct BarrierInfo {
    Semaphore_t *barrier_pending;
    Semaphore_t *barrier_release;
};
static struct BarrierInfo info;

static void *barrier_task(void *arg) {
    struct BarrierInfo *bi = (struct BarrierInfo*) arg;
    semaphore_up(bi->barrier_pending);
    semaphore_down(bi->barrier_release);
    return NULL;
}

/*!
 * Wait for all work given to the pool to complete.
 */
void pool_barrier() {
    Work_t *barrier_head = NULL;
    Work_t *barrier_tail = NULL;

    for (int i = 0; i < num_threads; i++) {
        barrier_head = (Work_t*) malloc(sizeof(Work_t));
        barrier_head->next = barrier_tail;
        barrier_head->fun = &barrier_task;
        barrier_head->arg = &info;
        barrier_tail = barrier_head;
    }

    // Add barrier tasks...
    pthread_mutex_lock(&(work_lock));
    if (work_head == NULL) {
        work_head = barrier_head;
        work_tail = barrier_tail;
    } else {
        work_tail->next = barrier_head;
        work_tail = barrier_tail;
    }
    pthread_mutex_unlock(&(work_lock));

    // Notify the threads that the barrier tasks were added added...
    for (int i = 0; i < num_threads; i++)
        semaphore_up(work_pending);

    // Wait for all barrier tasks to reach the semaphore...
    for (int i = 0; i < num_threads; i++)
        semaphore_down(info.barrier_pending);

    // No work pending here! Release all barrier tasks...
    for (int i = 0; i < num_threads; i++)
        semaphore_up(info.barrier_release);
}

static pthread_t pt[32]; // TODO: Remove hardcoding...?

void pool_init(int processors) {
    // Use external number of threads, if given...
    if (processors == 0)
        processors = num_processors();
    num_threads = processors;

    work_head = NULL;
    work_tail = NULL;
    pthread_mutex_init(&(work_lock), NULL);
    work_pending = semaphore_create(0);

    for (int i = 0; i < num_threads; i++)
        pthread_create(&pt[i], NULL, &runner, (void*) i);

    info.barrier_pending = semaphore_create(0);
    info.barrier_release = semaphore_create(0);
}

void pool_shutdown() {
    // Increase work_pending semaphore without real work --> pthread_exit();
    for (int i = 0; i < num_threads; i++)
        semaphore_up(work_pending);
    // Collect threads.
    for (int i = 0; i < num_threads; i++)
        pthread_join(pt[i], NULL);

    semaphore_destroy(work_pending);
    pthread_mutex_destroy(&(work_lock));
    semaphore_destroy(info.barrier_pending);
    semaphore_destroy(info.barrier_release);
}

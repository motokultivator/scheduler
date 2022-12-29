/******************************************************************

  Author: Aleksandar Rikalo <arikalo@gmail.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2 of the
  License, or (at your option) any later version.
  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
  02111-1307, USA.
  The GNU General Public License is contained in the file COPYING.
*/

#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include "scheduler.h"

#define VALID     1
#define RUNNING   2

#define WAITING(x) ((x & (VALID | RUNNING)) == VALID)

// Thread which own the task can access/modify the struct.
// All other threads should acquire status_guard, and after that they can read the status.
// If RUNNING bit is set, the thread should unlock the guard asap, otherwise it can modify the struct.
struct task_t {
  void *stack;
  uint64_t id;
  volatile uint32_t status;
  MUTEX status_guard;
  void* ctx;
};

// GLOBALS
uint64_t tids[NUM_THREADS];
struct task_t tasks[MAX_TASKS];
unsigned long page_size;
unsigned long page_mask;
volatile int tasks_size;
volatile uint64_t task_id;
volatile int num_tasks;
void* stacks;
struct sigaction sa_orig;
MUTEX num_tasks_guard;

// TLS
__thread int current; // Task currently owned by thread.

// ASM routines

// Initialize a new context on the given stack.
// Returns a pointer to context inside the stack.
void* init_ctx(void* stack, void* entry, void* entry_arg0, void* entry_arg1, void* entry_arg2);
// Swaps current context with given ctx.
void swap_ctx(void* ctx);
// Restores given ctx and store (32 bits) 0 to status.
void restore_and_unlock(void* ctx, volatile uint32_t* status);

static void segfault_sigaction(int signal, siginfo_t *si, void *arg) {
  // Note: Here we have only SIGSTKSZ (usually 8192) bytes of a stack and have no overflow protection.
  // Anyway, it is far enough to call mprotect().
  if (LIKELY(si->si_addr >= tasks[current].stack && si->si_addr < tasks[current].stack + (MAX_STACK_SIZE & page_mask))) {
    // This is access to stack.
    mprotect((void*)((unsigned long)si->si_addr & page_mask), page_size, PROT_WRITE | PROT_READ);
    // TODO: Detect red zone access and shut down the particular task.
  } else {
    // This is a real segfault.
    exit(EXIT_FAILURE);
    // TODO: It is possible to call the original handler, but we need to swap the stacks first.
    // if (sa_orig.sa_flags & SA_SIGINFO)
    //   (*sa_orig.sa_sigaction)(signal, si, arg);
    // else
    //   (*sa_orig.sa_handler)(signal);
  }
}

void yld() {
  swap_ctx(tasks[current].ctx);
}

static void* sched(void *v) {
  stack_t ss, ss_old;
  // A stack in the stack, I like it.
  ss.ss_sp = alloca(SIGSTKSZ);

  if (UNLIKELY(ss.ss_sp == NULL)) {
    return NULL;
  }

  ss.ss_size = SIGSTKSZ;
  ss.ss_flags = 0;

  if (UNLIKELY(sigaltstack(&ss, &ss_old) == -1)) {
    return NULL;
  }

  for (;;) {
    LOCK(num_tasks_guard);

    if (UNLIKELY(num_tasks == 0)) {
      UNLOCK(num_tasks_guard);
      break;
    }

    UNLOCK(num_tasks_guard);

    for (int i = 0; i < tasks_size; i++) {
      LOCK(tasks[i].status_guard);

      if (UNLIKELY(WAITING(tasks[i].status))) {
        // Found. Valid, not running.
        tasks[i].status |= RUNNING;
        UNLOCK(tasks[i].status_guard);
        current = i;
        swap_ctx(tasks[current].ctx);
        tasks[current].status &= ~RUNNING;
      } else {
        UNLOCK(tasks[i].status_guard);
      }
    }
  }

  sigaltstack(&ss_old, NULL); // Is it sensible?
  return NULL;
}

static void task_wrap(entry_ptr entry, void *arg, struct task_t* task) {
  entry(arg);
  LOCK(num_tasks_guard);
  num_tasks--;
  UNLOCK(num_tasks_guard);
  restore_and_unlock(task->ctx, &task->status);
}

int init() {
  page_size = sysconf(_SC_PAGESIZE);
  page_mask = ~(page_size - 1);
  tasks_size = 0;
  uint64_t stacks_map_size = MAX_TASKS * ((RED_ZONE_STACK_SIZE & page_mask) + (MAX_STACK_SIZE & page_mask));
  // Reserve address space for the stacks.
  stacks = mmap(NULL, stacks_map_size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);

  if (UNLIKELY(stacks == NULL))
    return 1;

  memset(tasks, 0, MAX_TASKS * sizeof(struct task_t));

  for (int i = 0; i < MAX_TASKS; i++)
    INIT(tasks[i].status_guard);

  INIT(num_tasks_guard);
  struct sigaction sa;
  memset(&sa, 0, sizeof(struct sigaction));
  sigemptyset(&sa.sa_mask);
  sa.sa_sigaction = segfault_sigaction;
  sa.sa_flags = SA_SIGINFO | SA_ONSTACK;
  sigaction(SIGSEGV, &sa, &sa_orig);
  return 0;
}

uint64_t task(entry_ptr entry, void *arg) {
  int i;

  for (i = 0; i < MAX_TASKS; i++) {
    LOCK(tasks[i].status_guard);

    if (UNLIKELY((tasks[i].status & VALID) == 0))
      break;

    UNLOCK(tasks[i].status_guard);
  }

  if (UNLIKELY(i == MAX_TASKS))
    return 0;

  LOCK(num_tasks_guard);
  num_tasks++;
  UNLOCK(num_tasks_guard);
  tasks[i].status = VALID | RUNNING;
  UNLOCK(tasks[i].status_guard);

  if (UNLIKELY(tasks[i].stack == NULL)) {
    unsigned long stack_space = (MAX_STACK_SIZE & page_mask) + (RED_ZONE_STACK_SIZE & page_mask);
    tasks[i].stack = stacks + stack_space * i + (RED_ZONE_STACK_SIZE & page_mask);
    mprotect(tasks[i].stack + (MAX_STACK_SIZE & page_mask) - (MIN_STACK_SIZE & page_mask), MIN_STACK_SIZE & page_mask,
             PROT_WRITE | PROT_READ);
    __sync_fetch_and_add(&tasks_size, 1);
  }

  tasks[i].id = __sync_add_and_fetch(&task_id, 1);
  tasks[i].ctx = init_ctx(tasks[i].stack + MAX_STACK_SIZE, task_wrap, entry, arg, tasks + i);
  tasks[i].status &= ~RUNNING;
  return tasks[i].id;
}

void run() {
  for (int i = 1; i < NUM_THREADS; i++)
    pthread_create(tids + i, NULL, sched, NULL);

  sched(NULL);

  for (int i = 1; i < NUM_THREADS; i++)
    pthread_join(tids[i], NULL);
}

void cleanup() {
  uint64_t stacks_map_size = MAX_TASKS * ((RED_ZONE_STACK_SIZE & page_mask) + (MAX_STACK_SIZE & page_mask));
  munmap(stacks, stacks_map_size);
  sigaction(SIGSEGV, &sa_orig, NULL);
}

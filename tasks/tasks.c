#include "tasks.h"
#include "../dump_heap/my_heap.h"
#include <stdint.h>
#include <stdlib.h>
#include <sys/cdefs.h>

#define ALIGN 4

extern uint32_t _estack;

uint8_t my_dump_heap[2048] __attribute__((aligned(8))) = {0};
static int last_id = 0;

my_heap_t my_heap = {
    .start = &my_dump_heap[0],
    .end = &my_dump_heap[0] + sizeof(my_dump_heap),
    .pos = &my_dump_heap[0],
};

void thread_exit(void) {
  while (1) {
    __asm__ volatile("wfi");
  }
}

thread_t *head = (thread_t *)0;
thread_t *current_task = (thread_t *)0;

int create_thread(char *name, void (*thread_function)(void *),
                  uint32_t stack_size, void *params) {

  if (stack_size > 8192 || stack_size <= 0) {
    return -1;
  }

  stack_size = (stack_size + ALIGN - 1) & ~(ALIGN - 1);

  if (head == 0) {
    // define the head
    head = (thread_t *)allocate_dumb(&my_heap, sizeof(thread_t));
    if (head == NULL) {
      return -1;
    }
    head->name = name;
    head->thread_function = thread_function;
    head->id = 0;
    head->stack_top = &_estack - (stack_size / 4);
    head->stack_end = head->stack_top + (stack_size / 4);
    head->stack_size = stack_size;
    head->params = params;

    head->next_task = NULL;

    current_task = head;
  } else {

    thread_t *new_task = (thread_t *)allocate_dumb(&my_heap, sizeof(thread_t));
    if (new_task == NULL) {
      return -1;
    }
    new_task->name = name;
    new_task->thread_function = thread_function;
    new_task->id = current_task->id + 1;
    new_task->stack_top = current_task->stack_top - current_task->stack_size;
    new_task->stack_end = new_task->stack_top - (stack_size / 4);
    new_task->stack_size = stack_size;
    new_task->params = params;

    new_task->next_task = NULL;

    current_task->next_task = new_task;

    current_task = new_task;
    
    last_id = new_task->id;
  }

  // prepare stack
  uint32_t sp_addr = (uint32_t)current_task->stack_top;

  uint32_t *sp = (uint32_t *)sp_addr;

  *(--sp) = (1U << 24);
  *(--sp) = (uint32_t)current_task->thread_function;
  *(--sp) = (uint32_t)thread_exit; // LR
  *(--sp) = 0;                     // R12
  *(--sp) = 0;                     // R3
  *(--sp) = 0;                     // R2
  *(--sp) = 0;                     // R1
  *(--sp) = (uint32_t)params;      // R0: Passes 'params' into function

  for (int i = 0; i < 8; i++) {
    *(--sp) = 0;
  }

  current_task->stack_top = sp;

  return 0;
}

void start_routines() { current_task = head; }

void next_task() {
  if (current_task->id != last_id) {
    current_task = current_task->next_task;
  } else {
    start_routines();
  }
  return;
}

/*
    uint32_t *get_stack_pointer(char *name) {
      thread_t *c = head;
    
      while (c->next_task != NULL) {
        if (strcmp(name, c->name) == 0) {
          return c->stack_top;
        }
        c = c->next_task;
      }
    }
 
 */

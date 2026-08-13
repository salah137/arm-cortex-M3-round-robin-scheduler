#include "dump_heap/my_heap.h"
#include "tasks/tasks.h"
#include <stdint.h>

#define PENDSV_PRIO_LOWEST (0xFFUL)
#define ICSR_PENDSVSET_BIT (1UL << 28)
#define SCB_SHPR3 (*((volatile uint32_t *)0xE000ED20UL))
#define SCB_ICSR (*((volatile uint32_t *)0xE000ED04UL))


extern int create_thread(char *name, void (*thread_function)(void *),
                  uint32_t stack_size, void *params) ;
extern void start_routines();
extern void PendSv_handler(void);
extern uint32_t *get_stack_pointer(char *name);
extern thread_t* head;

void task1(void* param){
    while (1) {

    }
}

void task2(void* param){
    while (1) {

    }
}

void task3(void* param){
    while (1) {

    }
}

void setup(){
    __asm__ volatile("cpsid i                 \n\t" : : : "memory");

    create_thread("task1",task1, 128,(void *) NULL);
    __asm__ volatile("cpsid i                 \n\t" : : : "memory");

    create_thread("task2",task2, 128,(void *) NULL);
    __asm__ volatile("cpsid i                 \n\t" : : : "memory");

    create_thread("task3",task3, 128,(void *) NULL);

    __asm__ volatile("cpsie i                 \n\t" : : : "memory");

}

int app_main(){

    setup();
    start_routines();

    __asm__ volatile(
    //    "svc #0\n\t"
        "mov r0,#0\n\t"
        
        "msr psp,r0\n\t"

        : : :);

    SCB_ICSR = ICSR_PENDSVSET_BIT;


    while (1);

}

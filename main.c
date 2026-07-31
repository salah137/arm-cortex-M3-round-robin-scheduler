#include "dump_heap/my_heap.h"
#include "tasks/tasks.h"
#include <stdint.h>

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
    
    uint32_t* psp = head->stack_top;
    
    __asm__ volatile (
        "msr psp, %0\n\t"
        "mov r0, #2\n\t"        // Set bit 1 (SPSEL = 1, select PSP)
        "msr control, r0\n\t"   // Write r0 to CONTROL register
        "isb\n\t"               // Instruction Synchronization Barrier (flushes pipeline)
        :
        :"r" (psp)
        :"r0", "memory"      // Tell compiler r0 and memory are modified
    );
    start_routines();

    
    return 0;
}


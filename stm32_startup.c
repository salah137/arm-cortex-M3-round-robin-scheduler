#include "stdint.h"
#include <stdint.h>
#include "tasks/tasks.h"
#include "dump_heap/my_heap.h"

extern int app_main();
extern void next_task();
extern thread_t current_task;
extern my_heap_t my_heap;

extern uint32_t _sidata;
extern uint32_t _estack;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

typedef struct {
  uint32_t CTRL;
  uint32_t LOAD;
  uint32_t VAL;
  uint32_t CALIB;
} systemt_t;

#define SysTick_BASE (0xE000E010UL)

#define SysTick_CTRL_ENABLE (1UL << 0)
#define SysTick_CTRL_TICKINT (1UL << 1)
#define SysTick_CTRL_CLK_SOURCE (1UL << 2)

#define CLOCK 1000000 * 16
#define TICKS 16

#define SCB_SHPR3 (*((volatile uint32_t *)0xE000ED20UL))
#define SCB_ICSR (*((volatile uint32_t *)0xE000ED04UL))

#define PENDSV_PRIO_LOWEST (0xFFUL)

#define ICSR_PENDSVSET_BIT (1UL << 28)

void reset_handler(void);
void Default_Handler(void);
void Systick_Handler(void);
void Systick_Init(void);
void PendSv_handler(void);
void SVC_Handler(void);

void NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));
void Hard_fault(void);

__attribute__((section(".vector_table"))) const uint32_t vector_table[] = {
    (uint32_t)&_estack,
    (uint32_t)reset_handler,
    (uint32_t)NMI_Handler,
    (uint32_t)Hard_fault,
    (uint32_t)Default_Handler,
    (uint32_t)Default_Handler,
    (uint32_t)Default_Handler,
    0,
    0,
    0,
    0,
    (uint32_t)SVC_Handler, //dd
    (uint32_t)Default_Handler,
    0,
    (uint32_t)PendSv_handler,
    (uint32_t)Systick_Handler,

};

void reset_handler(void) {
  uint32_t *s_ptr = &_sidata;
  uint32_t *d_ptr = &_sdata;

  while (d_ptr < &_edata) {
    *(d_ptr++) = *(s_ptr++);
  }

  d_ptr = &_sbss;
  while (d_ptr < &_ebss) {
    *(d_ptr++) = 0;
  }
  SCB_SHPR3 |= (PENDSV_PRIO_LOWEST << 16);
  Systick_Init();

  app_main();
}

void Default_Handler(void) {
  while (1) {
  }
}

void Hard_fault(){
    reset_handler();
}

void Systick_Init(void) {
  systemt_t *SysTick = (systemt_t *)SysTick_BASE;
  uint32_t reload = 15999;

  SysTick->VAL = 0;
  SysTick->LOAD = reload;

  SysTick->CTRL =
      SysTick_CTRL_TICKINT | SysTick_CTRL_ENABLE | SysTick_CTRL_CLK_SOURCE;
}

void Systick_Handler(void) { 
    SCB_ICSR = ICSR_PENDSVSET_BIT; 
    return;
}

__attribute__((naked)) void PendSv_handler(void) {
  __asm__ volatile("cpsid i                 \n\t"
                   "mrs r0,psp              \n\t"
                   
                   "cbz r0,1f               \n\t"

                   "stmdb r0!,{r4-r11}      \n\t"

                   "ldr r1,=current_task    \n\t"
                   "ldr r1,[r1]             \n\t"
                   "str r0,[r1,#8]          \n\t" // change if stack placement is changed

                   "1:                      \n\t"
                   "push {lr}               \n\t"
                   "bl next_task            \n\t"
                   "pop {lr}                \n\t"
                   
                   "ldr r1,=current_task    \n\t"
                   "ldr r1,[r1]             \n\t"
                   "ldr r0,[r1,#8]          \n\t"

                   "ldmia r0!,{r4-r11}      \n\t"
                   "msr psp,r0              \n\t"
                   "cpsie i                 \n\t"

                   "ldr lr, =0xFFFFFFFD     \n\t"   
                   "bx lr                   \n\t"                 
                   : : :"memory"
                   );
}


__attribute__((naked)) void SVC_Handler(void){
    __asm__ volatile(
            "cpsid i         \n\t"
            "mov r3, #2      \n\t"
            "msr control, r3 \n\t"
            "isb             \n\t" // CRITICAL: Forces the CPU to map the register change instantly
            "cpsie i         \n\t"
            "bx lr           \n\t" // CRITICAL: You must explicitly exit!
            : : : "r3", "memory"
    );
}
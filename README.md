# arm-cortex-M3-round-robin-scheduler

- this is a lightweight bare metal round roubine scheduler and context switcher for arm cortex M3
- it uses a Systick timer that fires a PendSv handler that perform the contex switching saving the old thread state (`R0–R12`, `LR`, `PC`, `xPSR`), and loading the next thread

## What is a thread

- this project represent threads in this format:

```C
      typedef struct thread {
        char *name;
        uint32_t id;
        uint32_t *stack_top;
        uint32_t *stack_end;
        uint32_t stack_size;
        void *params;
        void (*thread_function)(void *);
        struct thread *next_task;
        } thread_t;
```

- each thread has its own stack that start at stack_top and descend til it reachs the end,
- the stack_top starts at the end of the previos thread's stack,

- threads are aligned in a linked list allocated in static bump heap (nothing fancy way to say a big  array)
```C
    uint8_t my_dump_heap[2048] __attribute__((aligned(8))) = {0};
    ...
    my_heap_t my_heap = {
        .start = &my_dump_heap[0],
        .end = &my_dump_heap[0] + sizeof(my_dump_heap),
        .pos = &my_dump_heap[0],
    };

```

## Prerequisites
  - `qemu-system-arm` for emulating
  - `arm-none-eabi-gcc` for compiling
  - `arm-none-eabi-ld` for linking
  -  `arm-none-eabi-gdb` for gdb debuguing

## runing it
- the programe QEMU arm-emulator, and GDB for debuging

- first, compile and start the GDB server :
``` shell
    make qemu
```

- then listen in separate terminal :
``` shell
  make gdb
```
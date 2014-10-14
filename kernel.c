/* kernel.c: Kernel with pre-emptive scheduler */
/* Do not change this file */

#include "common.h"
#include "interrupt.h"
#include "kernel.h"
#include "queue.h"
#include "scheduler.h"
#include "util.h"
#include "printf.h"

#include "tasks.c"

#define NUM_PCBS      (NUM_TASKS)

pcb_t pcb[NUM_PCBS];

/* This is the system call table, used in interrupt.c */
int (*syscall[NUM_SYSCALLS]) ();

/* Structure describing the contents of an interrupt gate entry.
   (Protected Mode Software Architecture p. 206) */
struct gate {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t count;
    uint8_t access;
    uint16_t offset_high;
} __attribute__ ((packed)) idt[IDT_SIZE];

/* Array of addresses for the exception handlers we defined in
   interrupt.c. Used for initializing the IDT. */
static void (*exception_handler[NUM_EXCEPTIONS]) (void) = {
&exception_0, &exception_1, &exception_2, &exception_3, &exception_4,
        &exception_5, &exception_6, &exception_7, &exception_8,
        &exception_9, &exception_10, &exception_11, &exception_12,
        &exception_13, &exception_14};


static uint32_t *stack_new(void);
static void first_entry(void);
static int invalid_syscall(void);
static void init_syscalls(void);
static void create_gate(struct gate *entry, uint32_t offset,
                        uint16_t selector, char type, char privilege);
static void init_idt(void);
static void init_serial(void);
static void initialize_pcb(pcb_t *p, pid_t pid, struct task_info *ti);


/* This function is the entry point for the kernel.
 * !!!THIS MUST BE THE FIRST FUNCTION DEFINED IN THIS FILE!!!
 * otherwise it will be linked incorrectly, and you will
 * get undefined behavior.
 */
void _start(void)
{
    static pcb_t garbage_registers;
    int i;

    clear_screen(0, 0, 80, 25);

    queue_init(&ready_queue);
	queue_init(&sleep_wait_queue);
    current_running = &garbage_registers;

    /* Transcribe the task[] array (in tasks.c)
     * into our pcb[] array.
     */
    for (i = 0; i < NUM_TASKS; ++i) {
        initialize_pcb( &pcb[i], i, &task[i] );
        enqueue(&ready_queue, (node_t *) &pcb[i]);
    }

    init_syscalls();
    init_idt();
    init_serial();
		/* Initialize random number generator */
	  srand(get_timer()); /* using a random value */

    /* Enable the timer interrupt.  The interrupt flag will be set once we
       start the first process, thus starting scheduling. Refer to the IF
       flag in the EFLAGS register. */
    outb(0x21, 0xfe);

    /* Schedule the first task */
    enter_critical();
    scheduler_entry();
    /* We shouldn't ever get here */
    ASSERT(FALSE);
}


static uint32_t *stack_new()
{
    static uint32_t next_stack = 0x100000;

    next_stack += 0x1000;
    ASSERT(next_stack <= 0x200000);
    return (uint32_t *) next_stack;
}

static void initialize_pcb(pcb_t *p, pid_t pid, struct task_info *ti)
{
    p->entry_point = ti->entry_point;
    p->pid = pid;
    p->task_type = ti->task_type;
    p->priority = 1;
    p->status = FIRST_TIME;
		//p->holding_lock = NULL;
    switch (ti->task_type) {
    case KERNEL_THREAD:
        p->ksp = stack_new();
        p->nested_count = 1;
        break;
    case PROCESS:
        p->ksp = stack_new();
        p->usp = stack_new();
        p->nested_count = 0;
        break;
    default:
        ASSERT(FALSE);
    }
    *--p->ksp = (uint32_t) & first_entry;
}


static void first_entry()
{
    uint32_t *stack, entry_point;

    if (KERNEL_THREAD == current_running->task_type) {
        stack = current_running->ksp;
    } else {
        stack = current_running->usp;
    }
    entry_point = current_running->entry_point;

    // Messing with %esp in C is usually a VERY BAD IDEA
    // It is safe in this case because both variables are
    // loaded into registers before the stack change, and
    // because we jmp before leaving asm()
    asm volatile ("movl %0, %%esp;"
                  "call leave_critical;"
                  "jmp  *%1"
                 :: "r" (stack), "r" (entry_point));

    ASSERT(FALSE);
}


static int invalid_syscall(void)
{
    HALT("Invalid system call");
    /* Never get here */
    return 0;
}


    /* Called by kernel to assign a system call handler to the array of
       system calls. */
static void init_syscalls()
{
    int fn;

    for (fn = 0; fn < NUM_SYSCALLS; ++fn) {
        syscall[fn] = &invalid_syscall;
    }
    syscall[SYSCALL_YIELD] = (int (*)()) &do_yield;
    syscall[SYSCALL_EXIT] = (int (*)()) &do_exit;
    syscall[SYSCALL_GETPID] = &do_getpid;
    syscall[SYSCALL_GETPRIORITY] = &do_getpriority;
    syscall[SYSCALL_SETPRIORITY] = (int (*)()) &do_setpriority;
    syscall[SYSCALL_SLEEP] = (int (*)()) &do_sleep;
    syscall[SYSCALL_SHUTDOWN] = (int (*)()) &do_shutdown;
    syscall[SYSCALL_WRITE_SERIAL] = (int (*)()) &do_write_serial;
}


    /* Initialize the Interrupt Descriptor Table IDT can contain up to 256
       entries. Location (in memory) and size are stored in the IDTR. Each
       entry contains a descriptor. We only use interrupt gate descriptors
       in this project. For details: PMSA chapter 12. In this project all
       processes and threads runs in the same segment, so there is no
       segment switch when a process is interrupted. Every thing runs in
       the kernel mode, so there is no stack switch when an interrupt
       occurs. When either a hardware or software interrupt occurs: 1.  The
       processor reads the interrupt vector supplied either by the
       interrupt controller or instruction operand. 2.  The processor
       multiplies the vector by eight to create the offset into the IDT. 3.
       The processor reads the eight byte descriptor from the IDT entry. 4.
       CS, EIP and EFlags are pushed on the stack. 5. EFlags[IF] is cleared
       to disable hardware interrupts. 6.  The processor fetches the first
       instruction of the interrupt handler * (CS:EIP). 7.  The body of the
       interrupt routine is executed. 8.  If (hardware interrupt) then EOI.
       9.  IRET at end of the routine. This causes the processor to pop
       EFlags, EIP and CS 10. The interrupted program resumes at the point
       of interruption. */
void init_idt(void)
{
    int i;
    /* Structure describing the contents of the idt and gdt registers.
       Used for loading the idt and gdt registers. (Protected Mode
       Software architecture p. 42, ) */
    struct point {
        uint16_t limit;
        uint32_t base;
    } __attribute__ ((packed)) idt_p;

    /* IRQs 0-15 are associated with IDT entries 0-15, but so are also
       some software exceptions. So we remap irq 0-15 to IDT entries 32-48
       (PSMA p.187, and Und. PC p.1009) */

    /* interrupt controller 1 */
    outb(0x20, 0x11);           /* Start init of controller 0, require 4
                                   bytes */
    outb(0x21, IRQ_START);      /* IRQ 0-7 use vectors 0x20-0x27 (32-39) */
    outb(0x21, 0x04);           /* Slave controller on IRQ 2 */
    outb(0x21, 0x01);           /* Normal EOI non-buffered, 80x86 mode */
    outb(0x21, 0xfb);           /* Disable int 0-7, enable int 2 */

    /* interrupt controller 2 */
    outb(0xa0, 0x11);           /* Start init of controller 1, require 4
                                   bytes */
    outb(0xa1, IRQ_START + 8);  /* IRQ 8-15 use vectors 0x28-0x30 (40-48) */
    outb(0xa1, 0x02);           /* Slave controller id, slave on IRQ 2 */
    outb(0xa1, 0x01);           /* Normal EOI non-buffered, 80x86 mode */
    outb(0xa1, 0xff);           /* Disable int 8-15 */

    /* Timer 0 is fed from a fixed frequency 1.1932 MHz clock, regardless
       of the CPU system speed (The Undocumented PC p. 960/978) */

    /* set timer 0 frequency */
    outb(0x40, (unsigned char) PREEMPT_TICKS);
    outb(0x40, PREEMPT_TICKS >> 8);

    /* create default handlers for interrupts/exceptions */
    for (i = 0; i < IDT_SIZE; i++) {
        create_gate(&idt[i],    /* IDT entry */
                    (uint32_t) bogus_interrupt, /* interrupt handler */
                    KERNEL_CS,  /* interrupt handler segment */
                    INTERRUPT_GATE,     /* gate type */
                    0);         /* privilege level 0 */
    }

    /* Create handlers for some exceptions */
    for (i = 0; i < NUM_EXCEPTIONS; i++) {
        create_gate(&(idt[i]),  /* IDT entry */
                    (uint32_t) exception_handler[i],    /* exception
                                                           handler */
                    KERNEL_CS,  /* exception handler segment */
                    INTERRUPT_GATE,     /* gate type */
                    0);         /* privilege level 0 */
    }

    /* create gate for the fake interrupt generated on IRQ line 7 when the
       timer is working at a high frequency */
    create_gate(&(idt[IRQ_START + 7]),
                (uint32_t) fake_irq7_entry, KERNEL_CS, INTERRUPT_GATE, 0);

    /* create gate for the timer interrupt */
    create_gate(&(idt[IRQ_START]),
                (uint32_t) irq0_entry, KERNEL_CS, INTERRUPT_GATE, 0);

    /* create gate for system calls */
    create_gate(&(idt[IDT_SYSCALL_POS]),
                (uint32_t) syscall_entry, KERNEL_CS, INTERRUPT_GATE, 0);

    /* load the idtr with a pointer to our idt */
    idt_p.limit = (IDT_SIZE * 8) - 1;
    idt_p.base = (uint32_t) idt;

    /* load idtr */
    asm volatile ("lidt %0"::"m" (idt_p));
}


    /* General function to make a gate entry.  Refer to chapter 12, page
       203, in PMSA for a description of interrupt gates. */
void create_gate(struct gate *entry,    /* pointer to IDT entry */
                 uint32_t offset,       /* pointer to interrupt handler */
                 uint16_t selector,     /* code segment containing the
                                           interrupt handler */
                 char type,     /* type (interrupt gate, trap gate, or
                                   task gate) */
                 char privilege)
{                               /* privilege level */
    entry->offset_low = (uint16_t) offset;
    entry->selector = (uint16_t) selector;
    /* Byte 4 [0:4] = Reserved Byte 4 [5:7] = 0,0,0 */
    entry->count = 0;
    /* Byte 5 [0:2] = type[0:2] Byte 5 [3] = type[3] (1: 32-bit, 0:
       16-bit) Byte 5 [4] = 0 (indicates system segment) Byte 5 [5:6] =
       privilege Byte 5 [7] = 1 (segment always present) */
    entry->access = type | privilege << 5 | 1 << 7;
    entry->offset_high = (uint16_t) (offset >> 16);
}


    /* Used for debugging */
void print_status(void)
{
    static char *status[] = { "First  ", "Ready", "Blocked", "Exited " };
    int i, base;

    base = 13;
    printf(base - 4, 6, "P R O C E S S   S T A T U S");
    printf(base - 2, 0, "Pid\tType\tPrio\tStatus\tEntries");
    for (i = 0; i < NUM_PCBS && (base + i) < 25; i++) {
        printf(base + i, 0, "%d\t%s\t%d\t%s\t%u", pcb[i].pid,
               pcb[i].task_type == KERNEL_THREAD ? "Thread" : "Process",
               pcb[i].priority, status[pcb[i].status], pcb[i].entry_count);
    }
}

void do_shutdown(void)
{
  /* These numbers will work for bochs
   * provided it was compiled WITH acpi.
   * the default ubuntu 9 version of bochs
   * is NOT compiled with acpi support, though
   * the version in the Friend center lab DO have it.
   * This will probably not work with
   * any real computer.
   */
  outw( 0xB004, 0x0 | 0x2000 );

  /* Failing that... */
  HALT("Shutdown");
}

const int serial_port_base = 0x3f8;

/* Write a byte to the 0-th serial port */
void do_write_serial(int character)
{
  enter_critical();

  // wait until port is free
  int8_t byte;
  do
  {
    byte = inb( serial_port_base + 5 );
  } while( (byte & 0x20) == 0 );

  // send character to port
  outb(serial_port_base, character);

  leave_critical();
}

void init_serial(void)
{
  outb(serial_port_base+1, 0);
  outb(serial_port_base+3, 0x80);
  outb(serial_port_base+0, 3);
  outb(serial_port_base+1, 0);
  outb(serial_port_base+3, 3);
  outb(serial_port_base+2, 0xc7);
  outb(serial_port_base+4, 0x0b);
}



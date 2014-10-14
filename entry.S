/* Author(s): <Your name here>
 * COS 318, Fall 2013: Project 3 Pre-emptive Scheduler
 * entry.S
 */

	/* Hardcoded values. Do not change these!! */
	/* Offset of the kernel context in a PCB */
	.equ  KERNEL, 8
	/* Offset of the user context in a PCB */
	.equ  USER, 48
	/* Size of a context */
	.equ  CONTEXT_SIZE, 40
	/* Offset of %esp in a context */
	.equ  STACK, 36
	/* Offset of field nested_count in a PCB */
	.equ  NESTED_COUNT, 88

/* Do not change any of these macros! */
/* Save registers/flags to the specified offset in the current PCB */
#define SAVE_CONTEXT(offset) \
	movl    %esp, scratch; \
	movl    current_running, %esp; \
	leal    ((offset)+CONTEXT_SIZE)(%esp), %esp; \
	pushl    scratch; \
	pushal; \
	pushfl; \
	movl    scratch, %esp

/* As above, but save only %esp */
#define SAVE_STACK(offset) \
	movl    %eax, scratch; \
	movl    current_running, %eax; \
	movl    %esp, ((offset)+STACK)(%eax); \
	movl    scratch, %eax

/* Restore registers/flags from the specified offset in the current PCB */
#define RESTORE_CONTEXT(offset) \
	movl    current_running, %esp; \
	leal    (offset)(%esp), %esp; \
	popfl; \
	popal; \
	popl    %esp

/* As above, but restore only %esp */
#define RESTORE_STACK(offset) \
	movl    %eax, scratch; \
	movl    current_running, %eax; \
	movl    ((offset)+STACK)(%eax), %esp; \
	movl    scratch, %eax

/* Acknowledge a hardware interrupt */
#define SEND_EOI \
	movl    %eax, scratch; \
	movb    $0x20, %al; \
	outb    %al, $0x20; \
	movl    scratch, %eax

/* Increment disable_count, panicking on a double ENTER
 * Interrupts should be disabled before this macro is used
 */
#define ENTER_CRITICAL \
	movl    %eax, scratch; \
	movl    disable_count, %eax; \
	testl    %eax, %eax; \
	jnz    panic; \
	incl    %eax; \
	movl    %eax, disable_count; \
	movl    scratch, %eax

/* Decrement disable_count, panicking on a double LEAVE
 * Interrupts should be enabled after this macro is used
 */
#define LEAVE_CRITICAL \
	movl    %eax, scratch; \
	movl    disable_count, %eax; \
	decl    %eax; \
	testl    %eax, %eax; \
	jnz    panic; \
	movl    %eax, disable_count; \
	movl    scratch, %eax

/* TODO: Test current_running->nested_count, clobbering only eflags */
#define TEST_NESTED_COUNT \
	
/** TODO end */

	.data
// Temporary storage for 32-bit registers
scratch:
	.long    0
// Temporary storage for syscall return value
return_value:
	.long    0
// Signals status of interrupts
	  .globl    disable_count
disable_count:
	.long    0

	.text
	.code32 // protected mode

	.globl    scheduler_entry	
scheduler_entry:
	SAVE_CONTEXT(KERNEL)
	call    scheduler
	RESTORE_CONTEXT(KERNEL)
	ret

/* System call interrupt handler
 * The system call number is passed in %eax
 * The argument is in %ebx
 * The return value is placed in %eax
 * When an interrupt is called, the eflags is pushed onto the stack along with the return address.
	The IF flag is cleared, which prevents further interrupts (either hardware or software) from being
	recognized until IF is set again.
 */
	.globl    syscall_entry
syscall_entry:
	ENTER_CRITICAL
	SAVE_CONTEXT(USER)
	RESTORE_STACK(KERNEL)
	/* return_value = system_call_helper(%eax, %ebx); */
	pushl    %ebx
	pushl    %eax
	call    system_call_helper
	movl    %eax, return_value
	addl    $8, %esp
	SAVE_STACK(KERNEL)
	RESTORE_CONTEXT(USER)
	/* return return_value; */
	movl    return_value, %eax
	LEAVE_CRITICAL
	iret

/* TODO: Timer interrupt handler */
	.globl    irq0_entry
irq0_entry:
	SEND_EOI // Acknowledge the hardware interrupt
	/* Increment time_elapsed */	

	/* Increment disable_count to reflect the fact that interrupts are off */
	
	/* Test nested_count */
	
	/* nested_count not zero, so do nothing */
	jnz return

	/* do system call */
	
	
return:
	/* check the sleeping processes and wake them up on time */
	
	/* Decrement disable_count in anticipation of iret */
	
	iret

/** TODO end */

/* These functions are for use outside entry.S. Do not modify! */
	.globl    fake_irq7_entry
fake_irq7_entry:
	SEND_EOI
	iret

	.globl    enter_critical	
enter_critical:
	cli
	ENTER_CRITICAL
	ret

	.globl    leave_critical	
leave_critical:
	LEAVE_CRITICAL
	sti
	ret

/* Error Message: Write "PANIC" to the screen and enter an infinite loop */
panic:
	cli
	movl    $0xb800a, %esp
	pushw    $0x4f43
	pushw    $0x4f49
	pushw    $0x4f4e
	pushw    $0x4f41
	pushw    $0x4f50
infinite_loop:
        hlt
	jmp    infinite_loop

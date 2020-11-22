#include "kernel/lib/memory/layout.h"

#include "kernel/misc/gdt.h"
#include "kernel/interrupt/apic.h"
#include "kernel/interrupt/interrupt.h"

#define interrupt_handler_no_error_code(name, num)	\
	.globl name;					\
	.type name, @function;				\
	.align 4;					\
	name:                       \
	pushq $0; /*instead of error code*/		\
	pushq $(num);					\
	jmp interrupt_handler_common

#define interrupt_handler_with_error_code(name, num)	\
	.globl name;					\
	.type name, @function;				\
	.align 4;					\
	name:                       \
	pushq $(num);					\
	jmp interrupt_handler_common

// LAB4-5 Instruction: create interrupt and exception entry point (use macros above)
// Задание №7
interrupt_handler_no_error_code(asm_interrupt_div_by_zero_error, INTERRUPT_VECTOR_DIV_BY_ZERO)
interrupt_handler_no_error_code(asm_interrupt_debug, INTERRUPT_VECTOR_DEBUG)
interrupt_handler_no_error_code(asm_interrupt_nmi, INTERRUPT_VECTOR_NMI)
interrupt_handler_no_error_code(asm_interrupt_breakpoint, INTERRUPT_VECTOR_BREAKPOINT)
interrupt_handler_no_error_code(asm_interrupt_overflow, INTERRUPT_VECTOR_OVERFLOW)
interrupt_handler_no_error_code(asm_interrupt_bound_range, INTERRUPT_VECTOR_BOUND_RANGE)
interrupt_handler_no_error_code(asm_interrupt_invalid_opcode, INTERRUPT_VECTOR_INVALID_OPCODE)
interrupt_handler_no_error_code(asm_interrupt_device_not_available, INTERRUPT_VECTOR_DEVICE_NOT_AVAILABLE)

interrupt_handler_with_error_code(asm_interrupt_double_fault, INTERRUPT_VECTOR_DOUBLE_FAULT)
interrupt_handler_with_error_code(asm_interrupt_invalid_tss, INTERRUPT_VECTOR_INVALID_TSS)
interrupt_handler_with_error_code(asm_interrupt_segment_not_present, INTERRUPT_VECTOR_SEGMENT_NOT_PRESENT)
interrupt_handler_with_error_code(asm_interrupt_stack, INTERRUPT_VECTOR_STACK)
interrupt_handler_with_error_code(asm_interrupt_general_protection, INTERRUPT_VECTOR_GENERAL_PROTECTION)
interrupt_handler_with_error_code(asm_interrupt_page_fault, INTERRUPT_VECTOR_PAGE_FAULT)

interrupt_handler_no_error_code(asm_interrupt_x86_86_fp_instruction, INTERRUPT_VECTOR_X86_FP_INSTRUCTION)

interrupt_handler_with_error_code(asm_interrupt_alignment_check, INTERRUPT_VECTOR_ALIGNMENT_CHECK)

interrupt_handler_no_error_code(asm_interrupt_machine_check, INTERRUPT_VECTOR_MACHINE_CHECK)
interrupt_handler_no_error_code(asm_interrupt_simd_fp, INTERRUPT_VECTOR_SIMD_FP)
interrupt_handler_no_error_code(asm_interrupt_security_exception, INTERRUPT_VECTOR_SECURITY_EXCEPTION)
interrupt_handler_no_error_code(asm_interrupt_timer, INTERRUPT_VECTOR_TIMER)
interrupt_handler_no_error_code(asm_interrupt_keyboard, INTERRUPT_VECTOR_KEYBOARD)
interrupt_handler_no_error_code(asm_interrupt_syscall, INTERRUPT_VECTOR_SYSCALL)

// LAB4 instruction:
// -- save task context on stack
// -- setup kernel segment registers (only data required)
// -- call interrupt handler
// Задание №6
interrupt_handler_common:
    // ds, es, fs, gs в структуре task_context
    pushq $0x0
    movw %gs, 6(%rsp)
    movw %fs, 4(%rsp)
    movw %es, 2(%rsp)
    movw %ds, (%rsp)

    // структура gprs в структуре task_context
    pushq %r15
    pushq %r14
    pushq %r13
    pushq %r12
    pushq %r11
    pushq %r10
    pushq %r9
    pushq %r8
    pushq %rbp
    pushq %rsi
    pushq %rdi
    pushq %rdx
    pushq %rcx
    pushq %rbx
    pushq %rax

    // установка дескриптора сегмента данных ядра
    pushq %rax
    movw $GD_KD, %ax
    movw %ax, %ds
    movw %ax, %es
    popq %rax

    call interrupt_handler

#include <cpuid.h>

#include "stdlib/assert.h"
#include "stdlib/string.h"

#include "kernel/misc/util.h"

#include "kernel/lib/memory/mmu.h"
#include "kernel/lib/memory/map.h"
#include "kernel/lib/memory/layout.h"
#include "kernel/lib/console/terminal.h"

#include "kernel/asm.h"
#include "kernel/task.h"
#include "kernel/syscall.h"
#include "kernel/misc/tss.h"
#include "kernel/misc/gdt.h"
#include "kernel/loader/config.h"
#include "kernel/interrupt/interrupt.h"
#include "kernel/interrupt/pic.h"
#include "kernel/interrupt/apic.h"
#include "kernel/interrupt/ioapic.h"
#include "kernel/interrupt/interrupt.h"
#include "kernel/interrupt/timer.h"
#include "kernel/interrupt/keyboard.h"

static struct descriptor64 idt[256];

static struct tss tss[CPU_MAX_CNT]
	__attribute__((aligned(PAGE_SIZE)));

static const char *interrupt_name[256] = {
	[INTERRUPT_VECTOR_DIV_BY_ZERO] = "divide by zero",
	[INTERRUPT_VECTOR_DEBUG] = "debug",
	[INTERRUPT_VECTOR_NMI] = "nmi",
	[INTERRUPT_VECTOR_BREAKPOINT] = "breakpoint",
	[INTERRUPT_VECTOR_OVERFLOW] = "overflow",
	[INTERRUPT_VECTOR_BOUND_RANGE] = "bound range",
	[INTERRUPT_VECTOR_INVALID_OPCODE] = "invalid opcode",
	[INTERRUPT_VECTOR_DEVICE_NOT_AVAILABLE] = "device not available",
	[INTERRUPT_VECTOR_DOUBLE_FAULT] = "double fault",
	[INTERRUPT_VECTOR_INVALID_TSS] = "invalid tss",
	[INTERRUPT_VECTOR_SEGMENT_NOT_PRESENT] = "segment not present",
	[INTERRUPT_VECTOR_STACK] = "stack",
	[INTERRUPT_VECTOR_GENERAL_PROTECTION] = "general protection",
	[INTERRUPT_VECTOR_PAGE_FAULT] = "page fault",
	[INTERRUPT_VECTOR_X86_FP_INSTRUCTION] = "x86 floating point instruction",
	[INTERRUPT_VECTOR_ALIGNMENT_CHECK] = "alignment check",
	[INTERRUPT_VECTOR_MACHINE_CHECK] = "machine check",
	[INTERRUPT_VECTOR_SIMD_FP] = "simd floating point",
	[INTERRUPT_VECTOR_SECURITY_EXCEPTION] = "security exception",
	[INTERRUPT_VECTOR_TIMER] = "timer",
	[INTERRUPT_VECTOR_KEYBOARD] = "keyboard",
	[INTERRUPT_VECTOR_SYSCALL] = "syscall",
};

extern void asm_interrupt_div_by_zero_error();
extern void asm_interrupt_debug();
extern void asm_interrupt_nmi();
extern void asm_interrupt_breakpoint();
extern void asm_interrupt_overflow();
extern void asm_interrupt_bound_range();
extern void asm_interrupt_invalid_opcode();
extern void asm_interrupt_device_not_available();
extern void asm_interrupt_double_fault();
extern void asm_interrupt_invalid_tss();
extern void asm_interrupt_segment_not_present();
extern void asm_interrupt_stack();
extern void asm_interrupt_general_protection();
extern void asm_interrupt_page_fault();
extern void asm_interrupt_x86_86_fp_instruction();
extern void asm_interrupt_alignment_check();
extern void asm_interrupt_machine_check();
extern void asm_interrupt_simd_fp();
extern void asm_interrupt_security_exception();
//extern void asm_interrupt_timer();
//extern void asm_interrupt_keyboard();
extern void asm_interrupt_syscall();

// LAB5-6 Instruction:
// - find page, this address belongs to
// - if page not found or `r/w' bit is not set -> destroy task (this is invalid app)
// - if page doesn't contain `cow' bit -> destroy task (attempt to write into protected page)
// - copy page:
// -- allocate new page
// -- map it into `KERNEL_TEMP'
// -- copy date from origin page
// -- insert new page instead of origin one
// -- remove `KERNEL_TEMP' mapping
#define PAGE_FAULT_ERROR_CODE_P		(1 << 0)
#define PAGE_FAULT_ERROR_CODE_R_W	(1 << 1)
#define PAGE_FAULT_ERROR_CODE_U_S	(1 << 2)
#define PAGE_FAULT_ERROR_CODE_RSV	(1 << 3)
#define PAGE_FAULT_ERROR_CODE_I_D	(1 << 4)
void page_fault_handler(struct task *task)
{
	uintptr_t va = rcr2();
	pte_t *pte = NULL;

	// Your code goes here
	if (true)
		goto fail;

fail:
	terminal_printf("Page fault at `%lx', operation: %s, accessed by: %s\n", va,
			(task->context.error_code & PAGE_FAULT_ERROR_CODE_R_W) != 0 ? "write" : "read",
			(task->context.error_code & PAGE_FAULT_ERROR_CODE_U_S) != 0 ? "user" : "supervisor");
	if (pte != NULL && (*pte & PTE_COW) != 0)
		terminal_printf("\tpage is copy on write\n");
	if ((task->context.error_code & PAGE_FAULT_ERROR_CODE_P) == 0)
		terminal_printf("\tpage is not present\n");
	if ((task->context.error_code & PAGE_FAULT_ERROR_CODE_RSV) != 0)
		terminal_printf("\tcheck reserved fields inside page tables\n");
	if ((task->context.error_code & PAGE_FAULT_ERROR_CODE_I_D) != 0)
		terminal_printf("\tfault was because of instruction fetch\n");

	task_destroy(task);
	schedule();
}

// Task 5 from Lab 4
//____________________________________________________________________
void div_by_zero_handler(struct task *task) {
    (void) task;
    terminal_printf("WARNING: div by zero interrupt was not handled\n");
    task_run(task);
}

void debug_handler(struct task *task) {
    (void) task;
    terminal_printf("WARNING: debug interrupt was not handled\n");
    task_run(task);
}

void nmi_handler(struct task *task) {
    (void)task;
    terminal_printf("WARNING: nmi interrupt was not handled\n");
    task_run(task);
}

void breakpoint_handler(struct task *task) {
    (void) task;
    terminal_printf("WARNING: breakpoint interrupt was not handled\n");

    return schedule();
}

void overflow_handler(struct task *task) {
    (void) task;
    terminal_printf("WARNING: overflow interrupt was not handled\n");
}

void bound_range_handler(struct task *task) {
    (void) task;
    terminal_printf("WARNING: bound range interrupt was not handled\n");
}

void invalid_opcode_handler(struct task *task) {
    (void) task;
    terminal_printf("WARNING: invalid opcode interrupt was not handled\n");
}

void device_not_available_handler(struct task *task) {
    (void) task;
    terminal_printf("WARNING: device not available interrupt was not handled\n");
}

void double_fault_handler(struct task *task) {
    (void) task;
    terminal_printf("WARNING: double fault interrupt was not handled\n");
}

void invalid_tss_handler(struct task *task) {
    (void) task;
    terminal_printf("WARNING: tss interrupt was not handled\n");
}

void segment_not_present_handler(struct task *task) {
    (void) task;
    terminal_printf("WARNING: segment not present interrupt was not handled\n");
}

void stack_handler(struct task *task) {
    (void) task;
    terminal_printf("WARNING: stack handler interrupt was not handled\n");
}

void general_protection_handler(struct task *task) {
    (void) task;
    terminal_printf("WARNING: general protection interrupt was not handled\n");
}

void x86_fp_instruction_handler(struct task *task) {
    (void) task;
    terminal_printf("WARNING: x86 fp instruction interrupt was not handled\n");
}

void alignment_check_handler(struct task *task) {
    (void) task;
    terminal_printf("WARNING: alignment check interrupt was not handled\n");
}

void machine_check_handler(struct task *task) {
    (void) task;
    terminal_printf("WARNING: machine check interrupt was not handled\n");
}

void simd_fp_handler(struct task *task) {
    (void) task;
    terminal_printf("WARNING: simd fp interrupt was not handled\n");
}

void security_exception_handler(struct task *task) {
    (void) task;
    terminal_printf("WARNING: security exception interrupt was not handled\n");
}

// Lab 5. Задание №11
void syscall_handler(struct task *task) {
    (void) task;
    syscall(task);
}
//_______________________________________________________________________

void interrupt_handler(struct task_context ctx)
{
	struct cpu_context *cpu = cpu_context();

	// TODO: моя строка
	cpu->task = &cpu->self_task;

	// XXX: Interrups are disabled here, think twice before enable it,
	// because they can modify `cpu' value (it may cause a lot of problems)
	cpu->task->context = ctx;
	cpu->task->state = TASK_STATE_READY;

	// LAB4 Instruction:
	// - process known interrupt, and use default handler for all others.
	// - take into account, that kernel uses `int 3' to call `schedule()'
	// Задание №8
	switch (ctx.interrupt_number) {
	    case INTERRUPT_VECTOR_PAGE_FAULT:
		    return page_fault_handler(cpu->task);
	    case INTERRUPT_VECTOR_DIV_BY_ZERO:
	        return div_by_zero_handler(cpu->task);
        case INTERRUPT_VECTOR_DEBUG:
            return debug_handler(cpu->task);
	    case INTERRUPT_VECTOR_NMI:
	        return nmi_handler(cpu->task);
	    case INTERRUPT_VECTOR_BREAKPOINT:
	        return breakpoint_handler(cpu->task);
	    case INTERRUPT_VECTOR_OVERFLOW:
	        return overflow_handler(cpu->task);
	    case INTERRUPT_VECTOR_BOUND_RANGE:
	        return bound_range_handler(cpu->task);
	    case INTERRUPT_VECTOR_INVALID_OPCODE:
	        return invalid_opcode_handler(cpu->task);
	    case INTERRUPT_VECTOR_DEVICE_NOT_AVAILABLE:
	        return device_not_available_handler(cpu->task);
	    case INTERRUPT_VECTOR_DOUBLE_FAULT:
	        return double_fault_handler(cpu->task);
	    case INTERRUPT_VECTOR_INVALID_TSS:
	        return invalid_tss_handler(cpu->task);
	    case INTERRUPT_VECTOR_SEGMENT_NOT_PRESENT:
	        return segment_not_present_handler(cpu->task);
	    case INTERRUPT_VECTOR_STACK:
	        return stack_handler(cpu->task);
	    case INTERRUPT_VECTOR_GENERAL_PROTECTION:
	        return general_protection_handler(cpu->task);
	    case INTERRUPT_VECTOR_X86_FP_INSTRUCTION:
	        return x86_fp_instruction_handler(cpu->task);
	    case INTERRUPT_VECTOR_ALIGNMENT_CHECK:
	        return alignment_check_handler(cpu->task);
	    case INTERRUPT_VECTOR_MACHINE_CHECK:
	        return machine_check_handler(cpu->task);
	    case INTERRUPT_VECTOR_SIMD_FP:
	        return simd_fp_handler(cpu->task);
	    case INTERRUPT_VECTOR_SECURITY_EXCEPTION:
	        return security_exception_handler(cpu->task);
//	    case INTERRUPT_VECTOR_TIMER:
//	        return timer_handler(cpu->task);
//	    case INTERRUPT_VECTOR_KEYBOARD:
//	        return keyboard_handler(cpu->task);
	    case INTERRUPT_VECTOR_SYSCALL:
	        return syscall_handler(cpu->task);
	    default:
		    break;
	}

	terminal_printf("\nunhandled interrupt: %s (%u)\n",
			interrupt_name[ctx.interrupt_number],
			(uint32_t)ctx.interrupt_number);
	terminal_printf("Task dump:\n"
			"\trax: %lx, rbx: %lx, rcx: %lx, rdx: %lx\n"
			"\trdi: %lx, rsi: %lx, rsp: %lx\n"
			"\tr8:  %lx, r9:  %lx, r10: %lx, r11: %lx\n"
			"\tr12: %lx, r13: %lx, r14: %lx, r15: %lx\n"
			"\tcs: %x, ss: %x, ds: %x, es: %x, fs: %x, gs: %x\n"
			"\trip: %lx, rfalgs: %lb\n", ctx.gprs.rax, ctx.gprs.rbx,
			ctx.gprs.rcx, ctx.gprs.rdx, ctx.gprs.rdi, ctx.gprs.rsi,
			ctx.rsp, ctx.gprs.r8, ctx.gprs.r9,
			ctx.gprs.r10, ctx.gprs.r11, ctx.gprs.r12, ctx.gprs.r13,
			ctx.gprs.r14, ctx.gprs.r15, (uint32_t)ctx.cs, (uint32_t)ctx.ss,
			(uint32_t)ctx.ds, (uint32_t)ctx.es, (uint32_t)ctx.fs, (uint32_t)ctx.gs,
			ctx.rip, ctx.rflags);

	task_destroy(cpu->task);
	schedule();
}

int ioapic_init(void)
{
	uint32_t eax, ebx, ecx, edx;

	if (__get_cpuid(1, &eax, &ebx, &ecx, &edx) == 0) {
		terminal_printf("cpuid failed\n");
		return -1;
	}

	uint32_t local_apic_id = ebx;
	terminal_printf("Local APIC ID: %x\n", (local_apic_id >> 24) & 0xFF);

	uint32_t ver = IOAPIC_READ(IOAPICVER);
	terminal_printf("[IOAPIC] Maximum Redirection Entry: %u\n", (ver >> 16) + 1);

	// timer
	IOAPIC_WRITE(IOREDTBL_BASE, INTERRUPT_VECTOR_TIMER);
	IOAPIC_WRITE(IOREDTBL_BASE+1, local_apic_id);

	// keyboard
	IOAPIC_WRITE(IOREDTBL_BASE+2, INTERRUPT_VECTOR_KEYBOARD);
	IOAPIC_WRITE(IOREDTBL_BASE+3, local_apic_id);

	return 0;
}

void apic_enable(void)
{
	// 0x1B - msr of local apic
	// bit 11 - global enable/disable APIC flag
	asm volatile(
		"movl $0x1b, %ecx\n\t"
		"rdmsr\n\t"
		"btsl $11, %eax\n\t"
		"wrmsr"
	);
}

void interrupt_init(void)
{
	struct kernel_config *config = (struct kernel_config *)KERNEL_INFO;
	struct descriptor *gdt = config->gdt.ptr;
	struct cpu_context *cpu = cpu_context();
	struct idtr {
		uint16_t limit;
		void *base;
	} __attribute__((packed)) idtr = {
		sizeof(idt)-1, idt
	};

	// LAB4 Instruction: initialize idt, don't forget that interrupts and
	// exceptions, wich may occur inside kernel space should use IST,
	// to force stack switch
    // Задание №9
    idt[INTERRUPT_VECTOR_DIV_BY_ZERO] = INTERRUPT_GATE(GD_KT, asm_interrupt_div_by_zero_error, 1, IDT_DPL_S);
    idt[INTERRUPT_VECTOR_DEBUG] = INTERRUPT_GATE(GD_KT, asm_interrupt_debug, 1, IDT_DPL_S);
    idt[INTERRUPT_VECTOR_NMI] = INTERRUPT_GATE(GD_KT, asm_interrupt_nmi, 1, IDT_DPL_S);
    idt[INTERRUPT_VECTOR_BREAKPOINT] = INTERRUPT_GATE(GD_KT, asm_interrupt_breakpoint, 1, IDT_DPL_U);
    idt[INTERRUPT_VECTOR_OVERFLOW] = INTERRUPT_GATE(GD_KT, asm_interrupt_overflow, 1, IDT_DPL_S);
    idt[INTERRUPT_VECTOR_BOUND_RANGE] = INTERRUPT_GATE(GD_KT, asm_interrupt_bound_range, 1, IDT_DPL_S);
    idt[INTERRUPT_VECTOR_INVALID_OPCODE] = INTERRUPT_GATE(GD_KT, asm_interrupt_invalid_opcode, 1, IDT_DPL_S);
    idt[INTERRUPT_VECTOR_DEVICE_NOT_AVAILABLE] = INTERRUPT_GATE(GD_KT, asm_interrupt_device_not_available, 1, IDT_DPL_S);
    idt[INTERRUPT_VECTOR_DOUBLE_FAULT] = INTERRUPT_GATE(GD_KT, asm_interrupt_double_fault, 1, IDT_DPL_S);
    idt[INTERRUPT_VECTOR_INVALID_TSS] = INTERRUPT_GATE(GD_KT, asm_interrupt_invalid_tss, 1, IDT_DPL_S);
    idt[INTERRUPT_VECTOR_SEGMENT_NOT_PRESENT] = INTERRUPT_GATE(GD_KT, asm_interrupt_segment_not_present, 1, IDT_DPL_S);
    idt[INTERRUPT_VECTOR_STACK] = INTERRUPT_GATE(GD_KT, asm_interrupt_stack, 1, IDT_DPL_S);
    idt[INTERRUPT_VECTOR_GENERAL_PROTECTION] = INTERRUPT_GATE(GD_KT, asm_interrupt_general_protection, 1, IDT_DPL_S);
    idt[INTERRUPT_VECTOR_PAGE_FAULT] = INTERRUPT_GATE(GD_KT, asm_interrupt_page_fault, 1, IDT_DPL_S);
    idt[INTERRUPT_VECTOR_X86_FP_INSTRUCTION] = INTERRUPT_GATE(GD_KT, asm_interrupt_x86_86_fp_instruction, 1, IDT_DPL_S);
    idt[INTERRUPT_VECTOR_ALIGNMENT_CHECK] = INTERRUPT_GATE(GD_KT, asm_interrupt_alignment_check, 1, IDT_DPL_S);
    idt[INTERRUPT_VECTOR_MACHINE_CHECK] = INTERRUPT_GATE(GD_KT, asm_interrupt_machine_check, 1, IDT_DPL_S);
    idt[INTERRUPT_VECTOR_SIMD_FP] = INTERRUPT_GATE(GD_KT, asm_interrupt_simd_fp, 1, IDT_DPL_S);
    idt[INTERRUPT_VECTOR_SECURITY_EXCEPTION] = INTERRUPT_GATE(GD_KT, asm_interrupt_security_exception, 1, IDT_DPL_S);


    idt[INTERRUPT_VECTOR_SYSCALL] = INTERRUPT_GATE(GD_KT, asm_interrupt_syscall, 1, IDT_DPL_U);

	// Load idt
	asm volatile("lidt %0" :: "m" (idtr));

	// Initialize tss
	for (uint32_t idx = (GD_TSS >> 3), j = 0; j < CPU_MAX_CNT; idx += 2, j++) {
		struct descriptor64 *gdt64_entry = (struct descriptor64 *)&gdt[idx];
		*gdt64_entry = SEGMENT_TSS(&tss[j], sizeof(tss[j])-1, TYPE_AVAILABLE_TSS, TSS_DPL_S);
	}

	// LAB4 Instruction: create and map interrupt stack and exception
	// stack (use cpu->pml4)
	// Задание № 10
	for (uintptr_t addr = INTERRUPT_STACK_TOP - INTERRUPT_STACK_SIZE; addr < INTERRUPT_STACK_TOP; addr += PAGE_SIZE) {
	    struct page *temp = page_alloc();
	    if (page_insert(cpu->pml4, temp, addr, PTE_W) != 0) {
	        assert("ERROR: page allocating for INTERRUPT STACK");
	    }
	}

	for (uintptr_t addr = EXCEPTION_STACK_TOP - EXCEPTION_STACK_SIZE; addr < EXCEPTION_STACK_TOP; addr += PAGE_SIZE) {
	    struct page *temp = page_alloc();
	    if (page_insert(cpu->pml4, temp, addr, PTE_W) != 0) {
	        assert("ERROR: page allocating for EXCEPTION STACK");
	    }
	}

	// For now this os support only one processor, so we must initialize
	// only one tss. If you want use more processors -- you should
	// initialize tss for each one.
	tss[0].rsp0 = EXCEPTION_STACK_TOP;
	tss[0].ist1 = INTERRUPT_STACK_TOP;
	ltr(GD_TSS);

	// We are going to use IO APIC, so we must disable PIC.
	outb(PIC1_DATA, 0xff);
	outb(PIC2_DATA, 0xff);

#if LAB >= 7
	apic_enable();

	if (ioapic_init() != 0)
		panic("ioapic_init failed");
#endif
}

void interrupt_enable(void)
{
	if (timer_init() != 0)
		panic("timer_init failed");
}

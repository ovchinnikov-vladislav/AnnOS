#include "kernel/misc/gdt.h"

# Start the CPU: switch to 32-bit protected mode, jump into C.
# The BIOS loads this code from the first sector of the hard disk into
# memory at physical address 0x7c00 and starts executing in real mode
# with %cs=0 %ip=7c00.

.set PROT_MODE_CSEG, 0x8         # kernel code segment selector
.set PROT_MODE_DSEG, 0x10        # kernel data segment selector
.set CR0_PE_ON,      0x1         # protected mode enable flag

.globl boot_entry
boot_entry:

.code16
	// first of all: disable interrupts
	// данная команда в системе команд x86 (сокр. от Clear Interrupt-Enable Flag)
	// сбрасывает interrupt flag (IF) в регистре EFLAGS
	// когда данный флаг сброшен, процессор игнорирует все прерывания (кроме NMI) от внешних устройств
	cli

	// initialize segment registers
	xorw %ax, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %ss

	// enable a20: wait until 8042 will be ready
	// расширенная память за границами первого мегабайта недоступна, требуется включить линию A20
	// для включения линии A20 используем контроллер клавиатуры (PS/2)
	call wait_8042 // ожидание готовности контроллера

	// tell 8042 that we want to write
	// команды выдачи в порт 2 контроллера
	movb $0xd1, %al
	outb %al, $0x64

	// wait until 8042 will be ready
	// ожидание готовности контроллера
	call wait_8042

	// данные для порта 2 контроллера
	movb $0xdf, %al
	outb %al, $0x60

	// wait until 8042 process our command
	// ожидание готовности контроллера
	call wait_8042
	// линия A20 включена, переходим к следующему блоку
	jmp a20_enabled

	// ожидание готовности контроллера PS/2 к приему очередного байта
	wait_8042:
		inb $0x64, %al // чтение состояния контроллера
		testb $0x2, %al // входной буфер пуст?
		jnz wait_8042  // если не пуст, продолжаем
		ret


	a20_enabled:

	// Store memory map into 0x7e00 (0x7c00 + 512)
	movw $0x7de8, %di // 0x7e00 - 24 (отнимаем 24, потому что добавляем 24 вначаел detect_high_memory)
	xorl %ebx, %ebx // вначале ebx содержит нулевое смещение от начала карты памяти

	// before first call `%edx' must contain this magic value
	movl $0x534d4150, %edx // `SMAP'
	xorl %esi, %esi

	detect_high_memory:
		movl $0xe820, %eax
		movl $24, %ecx
		addw $24, %di
		int $0x15   // вызов BIOS для обнаружения доступной памяти

		// carry flag is clear in case all is ok (cf - функция BIOS отсутствует)
		jc memory_detected

		// magic value must be inside `%eax' in case success call
		cmpl $0x534d4150, %eax // `SMAP'
		jne memory_detected

		// increase records count
		incl %esi

		// `%ebx' will be set to `0' at the end of list
		test %ebx, %ebx
		jnz detect_high_memory

	memory_detected:

	// enable protected mode (set first bit in cr0)
	movl %cr0, %eax
	orl  $CR0_PE_ON, %eax
	movl %eax, %cr0

	lgdt gdtdesc    // загрузка глобальной таблицы дескприпторов
	ljmp $PROT_MODE_CSEG, $complete_flush

.code32

complete_flush:
	// set up protected mode segment registers
	// установка сегмента (переход от реальных адресов к виртуальным)
	movw $PROT_MODE_DSEG, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %ss
	movw %ax, %fs
	movw %ax, %gs

	// setup stack, and start our C part of the bootloader
	movl $boot_entry, %esp

	// Ugly hack, to store records count. It works in case bootloader
	// size lower than 508 bytes
	movl $0x7e00, %edi
	movl %esi, -4(%edi)

	call bootmain

	// we should never reach this code
	spin:
		jmp spin

// force 4 byte alignment
.p2align 2

gdt:
	SEG(0x0, 0x0, 0x0) // null seg
	SEG(UST_X|USF_D|USF_P|USF_S|USF_G|UST_R, 0x0, 0xfffff) // code seg
	SEG(USF_D|USF_P|USF_S|USF_G|UST_W, 0x0, 0xfffff) // data seg

gdtdesc:
	.word (. - gdt - 1)
	.long gdt

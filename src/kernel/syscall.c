#include "kernel/task.h"
#include "kernel/syscall.h"
#include "kernel/lib/memory/map.h"
#include "kernel/lib/memory/mmu.h"
#include "kernel/lib/memory/layout.h"

#include "stdlib/assert.h"
#include "stdlib/string.h"
#include "stdlib/syscall.h"

#include "kernel/lib/console/terminal.h"

// LAB5 Instruction:
// - find page, virtual address `va' belongs to. Use page_lookup
// - insert it into `dest->pml4' and `src->pml4' if needed
// Задание №13
// Подготовка страницы для форкнутого процесса
// маппинг страницы к дочерней и родительской задаче
static int task_share_page(struct task *dest, struct task *src, void *va, unsigned perm)
{
	uintptr_t va_addr = (uintptr_t)va;
	struct page *p = page_lookup(src->pml4, va_addr, NULL); // не NULL - страница ассоциирована с виртуальным адресом
	assert(p != NULL);

	// Можем ли писать или копировать страницу на запись
	if ((perm & PTE_W) != 0 || (perm & PTE_COW) != 0) {
	    perm = (perm | PTE_COW) & ~PTE_W; // убираем биты на запись, фиксируем биты для копирования на запись
	    if (page_insert(src->pml4, p, va_addr, perm) != 0) {
	        return -1;
	    }
	}

    if (page_insert(dest->pml4, p, va_addr, perm) != 0) {
        return -1;
    }

	terminal_printf("share page %p (va: %p): refs: %d\n", p, va, p->ref);

	return 0;
}

// LAB5 Instruction:
// - create new task, copy context, setup return value
//
// - share pages:
// - check all entries inside pml4 before `USER_TOP'
// - check all entrins inside page directory pointer size NPDP_ENTRIES
// - check all entries inside page directory size NPD_ENTRIES
// - check all entries inside page table and share if present NPT_ENTRIES
//
// - mark new task as `ready'
// - return new task id
// Задание №14
static int sys_fork(struct task *task)
{
	struct task *child = task_new("child");

	if (child == NULL)
		return -1;
	child->context = task->context;
	child->context.gprs.rax = 0; // return value

	// Цикл для прохода по всем таблица страничного преобразования вниз до таблицы PTE
    for (uintptr_t i = 0; i <= PML4_IDX(USER_TOP); i++) {
        uintptr_t p_dir_pointer_entry_addr = PML4E_ADDR(task->pml4[i]);

        if ((task->pml4[i] & PML4E_P) != 0) {
            pdpe_t *p_dir_pointer_entry = VADDR(p_dir_pointer_entry_addr);

            for (uint16_t j = 0; j < NPDP_ENTRIES; j++) {
                uintptr_t p_dir_entry_addr = PDPE_ADDR(p_dir_pointer_entry[j]);

                if ((p_dir_pointer_entry[j] & PDPE_P) != 0) {
                    pde_t *p_dir_entry = VADDR(p_dir_entry_addr);

                    for (uint16_t k = 0; k < NPD_ENTRIES; k++) {
                        uintptr_t p_tab_entry_addr = PTE_ADDR(p_dir_entry[k]);

                        if ((p_dir_entry[k] & PDE_P) != 0) {
                            pte_t *p_tab_entry = VADDR(p_tab_entry_addr);

                            for (uint16_t l = 0; l < NPT_ENTRIES; l++) {

                                // task_share_page - определяем задачу дочернего процесса, задачу родительского процесса,
                                // виртуальный адрес страницы дочернего процесса, и 12 битов, определяющих разрешение
                                // на чтение и запись, в случае неудачи уничтожаем новый процесс
                                if ((p_tab_entry[l] & PTE_P) != 0 &&
                                    task_share_page(child, task, PAGE_ADDR(i, j, k, l, 0),
                                                    p_tab_entry[l] & PTE_FLAGS_MASK) != 0) {
                                    task_destroy(child);
                                    return -1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Переводим задачу в статус готовой к исполнению
    child->state = TASK_STATE_READY;
	
	return child->id;
}

// LAB5 Instruction:
// - implement `puts', `exit', `fork' and `yield' syscalls
// - you can get syscall number from `rax'
// - return value also should be passed via `rax'
// Задание №12
// Обработка системного вызова
void syscall(struct task *task)
{
    uint64_t result;
    uint64_t rax = task->context.gprs.rax; // номер системного прерывания
    switch (rax) {
        case SYSCALL_PUTS:
            terminal_printf("Task %s with name %s: %s\n", task->id, task->name, (char *) task->context.gprs.rbx);
            break;
        case SYSCALL_EXIT:
            terminal_printf("Task %s with name %s finish\n", task->id, task->name);
            task_destroy(task);
            return schedule();
        case SYSCALL_FORK:
            result = sys_fork(task);
            task->context.gprs.rax = result;
            terminal_printf("Task %s forked task %s\n", task->id, result);
            break;
        case SYSCALL_YIELD:
            terminal_printf("Task %s - yield\n", task->id);
            return schedule();
        default:
            panic("ERROR: syscall undefined");
    }
	task_run(task);
}

#ifndef __UTIL_H__
#define __UTIL_H__

// Объединение a_ с b_
#define CONCAT(a_, b_) a_ ## b_
#define CONCAT2(a_, b_) CONCAT(a_, b_)
// Получение уникального имени токена путем конкатенации переданного имени
// и номера строки исполнения
#define UNIQ_TOKEN(name_) CONCAT2(name_, __LINE__)

// 1. Вывод типа по addr_, генерация новой переменной с данным типом и присвоение значения этой переменной addr_
// 2. Возврат значение addr_ - (addr_ % align_), что дает выравние вниз значения addr_ относительно align_
#define ROUND_DOWN(addr_, align_) ({				\
	__typeof__(addr_) UNIQ_TOKEN(addr) = addr_;		\
	UNIQ_TOKEN(addr) - (UNIQ_TOKEN(addr) % (align_));	\
})

// 1. Вывод типа по addr_, генерация новой переменной с данным типом и присвоение значения этой переменной addr_
// 2. Возврат значения (addr_ + align_ - 1) - ((addr_ + align_ - 1) % align_), что дает сдвиг по верху области
// путем добавления размера области и вычитания -1 от текущего адреса, по которой происходит выравние, т.е.
// выравние происходит как-будто по нижней границе -1 адрес следующего блока.
#define ROUND_UP(addr_, align_) ({					\
	__typeof__(align_) UNIQ_TOKEN(align) = align_;			\
	ROUND_DOWN(addr_ + UNIQ_TOKEN(align) - 1, UNIQ_TOKEN(align));	\
})

#endif

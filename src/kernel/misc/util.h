#ifndef __UTIL_H__
#define __UTIL_H__

// Объединение a_ и b_
#define CONCAT(a_, b_) a_ ## b_
#define CONCAT2(a_, b_) CONCAT(a_, b_)

// Получение уникального токена путем конкатенации переданного имени
// и номера строки исполнения
#define UNIQ_TOKEN(name_) CONCAT2(name_, __LINE__)

// 1. Вывод типа по addr_, генерация новой переменной с данным типом и присвоения значения этой переменной addr_
// 2. Возврат значения addr_ - (addr_ % aling_), что дает выравнивание вниз значения addr_ относительно align_
#define ROUND_DOWN(addr_, align_) ({				\
	__typeof__(addr_) UNIQ_TOKEN(addr) = addr_;		\
	UNIQ_TOKEN(addr) - (UNIQ_TOKEN(addr) % (align_));	\
})

// 1. Вывод типа addr_, генерация новой переменной с данным типом и присвоения значения этой переменной addr_
// 2. Возврат значения (addr_ + align_ - 1) - ((addr_ + align_ - 1) % align_), что дает сдвиг по верху области
// путем добавления размера области и вычитания -1 от текущего адреса, по которой происходит выравнивание, т.е.
// выравнивание происходит как-будто по нижней границе следующего блока -1 адрес.
#define ROUND_UP(addr_, align_) ({					\
	__typeof__(align_) UNIQ_TOKEN(align) = align_;			\
	ROUND_DOWN(addr_ + UNIQ_TOKEN(align) - 1, UNIQ_TOKEN(align));	\
})

#endif

#ifndef __STACK_FRAME__
	#define __STACK_FRAME__

	/* Stack frame structure taken from RowDaBoat */
	typedef struct StackFrame {
		//Registers restore context
		uint64_t gs;
		uint64_t fs;
		uint64_t r15;
		uint64_t r14;
		uint64_t r13;
		uint64_t r12;
		uint64_t r11;
		uint64_t r10;
		uint64_t r9;
		uint64_t r8;
		uint64_t rsi;
		uint64_t rdi;
		uint64_t rbp;
		uint64_t rdx;
		uint64_t rcx;
		uint64_t rbx;
		uint64_t rax;

		//iretq hook
		uint64_t rip;
		uint64_t cs;
		uint64_t eflags;
		uint64_t rsp;
		uint64_t ss;
		uint64_t base;
	} StackFrame;

#endif
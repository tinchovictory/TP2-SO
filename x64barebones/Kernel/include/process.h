#ifndef __PROCESS
	#define __PROCESS

	#include <threads.h>

	#define NULL ((void *)0)

	#define NUMBER_OF_PAGES 256

	#define NAME_LENGTH 50

	typedef enum {READY, RUNNING, BLOCKED_IO, BLOCKED_MUTEX, BLOCKED_PIPE, DEAD} processState;

	/*
		Registers are pushed into the stack instead of being stored in the pcb
		in processSwitch function inside schedulerSwitch.asm
	*/

	typedef struct heapNode {
		void * address;
		int size;
		struct heapNode * next;
	} heapNode;
	
	typedef struct process {
		int ppid;
		int pid;
		processState state;
		char * name;

		threadLibrary * threadLibrary;
		threadLibrary * currentThread;
		int threadSize;

		int threadTick;

		int currentPThread;

		heapNode * heap;
	} processNode;

	/* Creates a process in memory from the process entry point */
	processNode * createProcess(char * name, int ppid);

	void createProcessName(processNode * process, char * name);

	/* Free Process structure */
	void freeProcess(processNode * process);

	/* Print process state */
	void printProcessInfo(processNode * process);
	char * getStatus(processState state);

	/* Create a stack frame for proces initialization */
	//void * fillStackFrame(void * entryPoint, void * baseStack);

	void * pmalloc(int nBytes);

	void pfree(void * mem);

	heapNode * pfreeR(heapNode * node, void * mem);

	heapNode * addHeap(heapNode * node, void * mem, int nBytes);

	void freeHeap(heapNode * node);

#endif
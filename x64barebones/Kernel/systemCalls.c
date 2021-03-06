#include "systemCalls.h"
#include <videoDriver.h>
#include <keyBoardDriver.h>
#include <RTL8139.h>
#include <scheduler.h>
#include <screenLoader.h>
#include <mutex.h>
#include <semaphores.h>
#include <process.h>
#include <pipefs.h>

int writeToVideo(void * buf, uint64_t nBytes);
int writeToMyScreen(void * buf, uint64_t nBytes);

uint64_t clearScreenSys(){
	clearScreen();
	return 0;
}

uint64_t read(uint64_t fileDescriptor, void * buf, uint64_t nBytes){
	if(fileDescriptor == STANDARD_IO_FD){
		if (blockIfNotOnFocus()) {
			return 0;
		};

		char * myBuf = (char *) buf;
		int cont = 1, readBytes=0;
		for (int i = 0; i < nBytes && cont; i++){
			*(myBuf + i) = (char) getKey();
			if(*(myBuf + i) == 0){
				cont = 0;
			}else{
				readBytes++;
			}
		}
		
		if(readBytes == 0) {
			blockThread(getCurrentPid(),getCurrentPthread(), T_BLOCKED_IO);
		}
		return readBytes;
	}else if(fileDescriptor == ETHERNET_FD){
		return getMsg((ethMsg *) buf);
	}
	return -1;
}

int blockIfNotOnFocus(){
	if (!isCurrentProcessOnFocus()) {
		blockThread(getCurrentPid(),getCurrentPthread(), T_BLOCKED_IO);
		return 1;
	}
	return 0;
}


uint64_t write(uint64_t fileDescriptor, void * buf, uint64_t nBytes){
	if(fileDescriptor == STANDARD_IO_FD){
		if (isCurrentProcessOnFocus()) {
			return writeToVideo(buf,nBytes);
		} else {
			return writeToMyScreen(buf,nBytes);
		}
	}else if(fileDescriptor == ETHERNET_FD){
		sendMsg(*((ethMsg *) buf));
		return nBytes;
	}
	return -1;
}

int writeToVideo(void * buf, uint64_t nBytes) {
	char * myBuf = (char *) buf;
	int i;
	for(i = 0; i < nBytes && myBuf[i] != 0; i++){
		printCharacters(myBuf[i]);
	}
	return i;
}

int writeToMyScreen(void * buf, uint64_t nBytes) {

	char * myBuf = (char *) buf;
	char * myScreen = getCurrentScreen();
	char * myScreenPos = getCurrentScreenPosition();
	int i;

	for(i = 0; i < nBytes && myBuf[i] != 0; i++){
		printCharactersInner(myBuf[i],myScreen,&myScreenPos);
	}

	setCurrentScreenPosition(myScreenPos);

	return i;
}

uint64_t pcreate(void * exec, void * entryPoint, char * name) {
	int pid = addProcess(exec, entryPoint, name);
	loadScreen(pid);
	return pid;
}

uint64_t pcreateBackground(void * exec, void * entryPoint, char * name) {
	return addProcess(exec, entryPoint, name);
}

uint64_t tcreate(uint64_t pid, void * exec, void * entryPoint){
	return addThreadToProcess(pid, exec, entryPoint);
}

uint64_t pkill(uint64_t pid){
	removeScreen(pid);
	loadScreen(getParentPid(pid));
	removeProcess(pid);
	return 1;
}

uint64_t tkill(uint64_t pid, uint64_t pthread){
	removeThread(pid, pthread);
	return 1;
}

uint64_t ps(){
	printAllProcess();
	return 1;
}

void * mallocSysCall(uint64_t bytes) {
	return pmalloc(bytes);
}

uint64_t freeSysCall(void * memoryPosition) {
	pfree(memoryPosition);
	return 1;
}

uint64_t createMutexSysCall() {
	return createMutex();
}

uint64_t endMutexSysCall(uint64_t id) {
	mutexDestroy(id);
	return 1;
}

uint64_t upMutexSysCall(uint64_t id) {
	mutexUp(id);
	return 1;
}

uint64_t downMutexSysCall(uint64_t id) {
	mutexDown(id);
	return 1;
}

uint64_t getCurrentPidSysCall() {
	return getCurrentPid();
}

uint64_t createSemaphoreSysCall(uint64_t start) {
	return createSemaphore(start);
}

uint64_t endSemaphoreSysCall(uint64_t id) {
	semaphoreDestroy(id);
	return 1;
}

uint64_t upSemaphoreSysCall(uint64_t id) {
	semaphoreUp(id);
	return 1;
}

uint64_t downSemaphoreSysCall(uint64_t id) {
	semaphoreDown(id);
	return 1;
}


uint64_t createPipeSysCall(uint64_t fromPid, uint64_t toPid){
	return (uint64_t) do_pipe(fromPid,toPid);
}

uint64_t sendSysCall(char * name, char * buff){
	writeToPipe(name, buff);
	return 1;
}

uint64_t receiveSysCall(char * name, char * buff){
	readFromPipe(name,buff);
	return 1;
}




uint64_t systemCall(uint64_t systemCallNumber, uint64_t param1, uint64_t param2, uint64_t param3){

	switch(systemCallNumber) {
		case SYS_CALL_READ:
			/* param1: filedescriptor, param2: buffer, param3: nBytes */
			return read(param1, (void *) param2, param3);

		case SYS_CALL_WRITE:
			/* param1: filedescriptor, param2: buffer, param3: nBytes */
			return write(param1, (void *) param2, param3);

		case SYS_CALL_CLEAR_SCREEN:
			/* no params */
			return clearScreenSys();

		case SYS_CALL_MEMORY_ASSIGN:
			/* param1: nBytes */
			return (uint64_t) mallocSysCall(param1);

		case SYS_CALL_MEMORY_FREE:
			/* param1: memoryPosition */
			return freeSysCall((void *) param1);

		case SYS_CALL_PS:
			/* no params */
			ps();
			return 1;//HAY QUE HACERLO

		case SYS_CALL_CREATE_PROCESS:
			/* param1: exec, param2: entryPoint, param3: name */
			return pcreate((void *) param1, (void *) param2, (char *) param3);

		case SYS_CALL_CREATE_PROCESS_BACKGROUND:
			/* param1: exec, param2: entryPoint, param3: name */
			return pcreateBackground((void *) param1, (void *) param2, (char *) param3);

		case SYS_CALL_END_PROCESS:
			/* param1: pid */
			return pkill(param1);

		case SYS_CALL_CREATE_THREAD:
			/* parm1: pid, param2: exec, param3: entryPoint */
			return tcreate(param1, (void *) param2, (void *) param3);

		case SYS_CALL_END_THREAD:
			/* param1: pid, param2: pthread */
			return tkill(param1, param2);

		case SYS_CALL_CREATE_MUTEX:
			/* no params */
			return createMutexSysCall();

		case SYS_CALL_END_MUTEX:
			/* param1: id */
			return endMutexSysCall(param1);

		case SYS_CALL_UP_MUTEX:
			/* param1: id */
			return upMutexSysCall(param1);

		case SYS_CALL_DOWN_MUTEX:
			/* param1: id */			
			return downMutexSysCall(param1);

		case SYS_CALL_CURRENT_PID:
			/* no params */
			return getCurrentPidSysCall();

		case SYS_CALL_CREATE_SEMAPHORE:
			/* param1: start */
			return createSemaphoreSysCall(param1);

		case SYS_CALL_END_SEMAPHORE:
			/* param1: id */
			return endSemaphoreSysCall(param1);

		case SYS_CALL_UP_SEMAPHORE:
			/* param1: id */
			return upSemaphoreSysCall(param1);

		case SYS_CALL_DOWN_SEMAPHORE:
			/* param1: id */			
			return downSemaphoreSysCall(param1);
		case SYS_CALL_SEND:
			/* param1: name, param2: buff */
			return sendSysCall((char *)param1, (char *) param2);
		case SYS_CALL_RECEIVE:
			/* param1: name, param2: buff */
			return receiveSysCall((char *)param1, (char *)param2);
		case SYS_CALL_CREATE_PIPE:
			/* param1: fromPid, param2: toPid */
			return createPipeSysCall(param1, param2);
	}

	
	return 0;
}
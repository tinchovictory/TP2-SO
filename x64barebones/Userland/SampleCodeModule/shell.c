#include <stdio.h>
#include <string.h>
#include <systemCalls.h>
#include <philosophers.h>

void shell();
void testPipes1();
typedef int (*EntryPoint)();

void exec(void * entryPoint) {
	((EntryPoint)entryPoint)();
	//kill;	
	exit();
	//while(1);
}

void run(void * entryPoint, char * name) {
	pcreate(exec, entryPoint, name);
}

void processComand(char * buffer){
	if (!strcmp(buffer,"help")){
		printf("  echo : print on screen\n");
		printf("  hola : saludo de la consola\n");
		printf("  2048game : Juego 2048\n");
		printf("  chat : enter to chat\n");
		printf("  clear : clear screen\n");
		printf("  ps : prints processes with attributes\n");
		printf("  philosophers : ​prints solution to philosophers problem\n");
		printf("  prodcons : prints solution to producer-consumer problem\n");
		printf("  new shell : prints solution to producer-consumer problem\n");
		printf("  pipes : prints solution to producer-consumer problem\n");
	}
	else if(startsWith("echo ",buffer)){
		puts(buffer+5);
	}
	else if(!strcmp(buffer,"hola")){
		puts("  Hola! Mi nombre es NetSky");
	}else if(!strcmp(buffer,"2048game")){
		clearScreen();
		printf("Run 2048\n");
		//game2048();
	}
	else if(!strcmp("clear",buffer)){
		clearScreen();
	}else if(startsWith("chat",buffer)){
		clearScreen();
		printf("Run my Chat\n");
		//myChat();
	}else if(!strcmp("ps",buffer)){
		ps();
	}else if(!strcmp("philosophers",buffer)){
		char * name = (char *) malloc(sizeof(char)*10);
		name[0] = 'p';name[1] = 'h';name[2] = 'i';name[3] = 'l';name[4] = 0;
		run(philosophers, name);
	}else if(!strcmp("prodcons",buffer)){
		char * name = (char *) malloc(sizeof(char)*10);
		name[0] = 'p';name[1] = 'r';name[2] = 'o';name[3] = 'd';name[4] = 0;
		run(prodcons, name);
	}else if(!strcmp("new shell",buffer)){
		char * name = (char *) malloc(sizeof(char)*10);
		name[0] = 's';name[1] = 'h';name[2] = 'e';name[3] = 'l';name[4] = 'l';name[5] = 0;
		run(shell, name);
	}else if(!strcmp("pipes",buffer)){
		char * name = (char *) malloc(sizeof(char)*10);
		name[0] = 'p';name[1] = 'i';name[2] = 'p';name[3] = 'e';name[4] = '1';name[5] = 0;
		run(testPipes1, name);
	}else{
		puts("  Command not found - help for instructions");
	}
}

void shell(){

	char buffer[100];
	int i=0;
	while(1){
		char c;
		printf("$> ");
		while ((c=getchar())!= '\n'){
			if(c != '\b'){
				buffer[i++]=c;
				putchar(c);
			}else if (i>0){
				i--;
				putchar(c);
			}	
		}
		putchar(c);
		buffer[i]=0;
		processComand(buffer);
		i=0;
	}
}


/****************************************************/
/****************************************************/
/*******************PHILOSOPHERS*******************/
/****************************************************/
/****************************************************/

int auxCounter = 0;
int mainMutexId;
int philoMutexId[PHILOSOPHERS_COUNT];
int philosopherId[PHILOSOPHERS_COUNT];
char * stateStrings[3] = { "Hungry", "Thinking", "Eating" };
State philoState[PHILOSOPHERS_COUNT];
int forkState[PHILOSOPHERS_COUNT];


void philosophers() {
	int pid = getCurrentPid();
	int i; 
	mainMutexId = createMutex();
	for(i = 0; i < PHILOSOPHERS_COUNT; i++){
		//initialize forks
		philoMutexId[i] = createMutex();
		mutexDown(philoMutexId[i]);
		forkState[i] = -1;
		//initialize philosophers
		philosopherId[i] = i;
		philoState[i] = THINKING;
		philosopherId[i] = tcreate(pid, exec, philosopher);
	}
	printf("running\n");


	//exit();
	while(1);
}

void philosopher() {
	int id = auxCounter;
	auxCounter++;
	while(1) {
		//Think
		//myTestSleep(10000000);
		takeForks(id);

		//Eat
		//myTestSleep(10000000);
		putForks(id);
	}
}

void checkForks(int philoId) {
	if(philoState[philoId] == HUNGRY && 
		philoState[left(philoId)] != EATING && 
		philoState[right(philoId)] != EATING){
		//forks available
		philoState[philoId] = EATING;
		forkState[left(philoId)] = philoId;
		forkState[philoId] = philoId;
		render();

		mutexUp(philoMutexId[philoId]); //Pick up forks
	}
}

void takeForks(int philoId) {
	mutexDown(mainMutexId);

	//Set state
	philoState[philoId] = HUNGRY;
	render();

	checkForks(philoId);
	mutexUp(mainMutexId);
	mutexDown(philoMutexId[philoId]); //Lock if forks not acquired
}

void putForks(int philoId) {
	mutexDown(mainMutexId);

	philoState[philoId] = THINKING;
	forkState[left(philoId)] = -1;
	forkState[philoId] = -1;
	render();

	checkForks(left(philoId)); //philosopher on my left tries to acquire forks
	checkForks(right(philoId)); //philosopher on my right tries to acquire forks
	mutexUp(mainMutexId);

}

int left(int i) {
	return (i + PHILOSOPHERS_COUNT - 1) % PHILOSOPHERS_COUNT;
}

int right(int i) {
	return (i + 1) % PHILOSOPHERS_COUNT;
}

void render() {
	int i;
	for(i = 0; i < PHILOSOPHERS_COUNT; i++) {
		printf("Philosopher %d: %s\n", i, stateStrings[philoState[i]]);
		printf("Fork - ");

		if (forkState[i] == -1)
			printf("Free\n");
		else
			printf("Owner %d\n", forkState[i]);
	}

	putchar('\n');
	putchar('\n');
	myTestSleep(100000000);
}

void myTestSleep(int wakeUpCall) {
	int i = 0;
	while(i < wakeUpCall){
		i++;
	}
}

/****************************************************/
/****************************************************/
/*******************PRODCONS*******************/
/****************************************************/
/****************************************************/

int producerThread;
int consumerThread;

int itemMutex;
int emptyCount;
int fullCount;
int aux = 1;

int prodSleepTime = 10000000;
int consSleepTime = 100000000;

int r = 0;
int w = 0;
char buffer[BUFFERSIZE];
int cant = 0;

void prodcons() {
	int pid = getCurrentPid();

	//Semaphore creation
	itemMutex = createMutex();
	emptyCount = createSemaphore(BUFFERSIZE);
	fullCount = createSemaphore(0);
	
	//Create threads
	producerThread = tcreate(pid, exec, producer);
	consumerThread = tcreate(pid, exec, consumer);

	control();
}

void producer() {

	while (1) {
		myTestSleep(prodSleepTime);

		char item = (char)aux;
		aux++;
		cant++;
		printf("Produce %d items:%d\n", item, cant);

		//Decrement the count of empty slots in the buffer (semaphore goes down)
		//Locks when the remaining empty slots are zero
		semaphoreDown(emptyCount);
		mutexDown(itemMutex);

		insertItem(item);
		mutexUp(itemMutex);

		//Increment the count of full slots in the buffer (semaphore goes up)
		semaphoreUp(fullCount);
	}
}

void consumer() {
	int item;

	while (1) {
		myTestSleep(consSleepTime);

		//Decrement the count of full slots in the buffer (semaphore goes down)
		//Locks when there is no more information in the buffer
		semaphoreDown(fullCount);
		mutexDown(itemMutex);

		item = removeItem();
		mutexUp(itemMutex);

		//Increment the count of empty slots in the buffer (semaphore goes up)
		semaphoreUp(emptyCount);
		cant--;
		printf("Consume %d items:%d\n", item, cant);
	}
}

void insertItem(char c) {
	buffer[w] = c;
	w = (w + 1) % BUFFERSIZE;
}

char removeItem() {
	char result = buffer[r];
	r = (r + 1) % BUFFERSIZE;
	return result;
}

void control() {
	int end = 0;

	while(!end) {
		int c = getchar();

		switch(c) {
			case 'a':
				prodSleepTime = prodSleepTime * 10;
			break;

			case 'z':
				prodSleepTime = prodSleepTime / 10;
			break;

			case 's':
				consSleepTime = consSleepTime * 10;
			break;

			case 'x':
				consSleepTime = consSleepTime / 10;
			break;

			case 'q':
				end = 1;
			break;
		}
	}
}


/****************************************************/
/****************************************************/
/*******************PRODCONS*******************/
/****************************************************/
/****************************************************/

void mySleep() {
	int i = 0;
	while(i<100000000) {
		i++;
	}
}

char * pipeName = NULL;
char * pipeName2 = NULL;
int ppid = -1;
int count = 0;
void testPipes2() {
	char * buff = (char *) malloc(200);
	char * buff2 = (char *) malloc(200);


	printf("Cargando ella...\n");

	printf("Afinando ella...\n");

	pipeName2 = createPipe(getCurrentPid(), ppid);

	while(pipeName == NULL || pipeName2 == NULL);

	buff2[0] = 0;
	while(buff2[0] == 0) {
		receive(pipeName, buff2);
	}
	printf("%d - Joaquin: %s...\n", count++, buff2);



	mySleep();

	strcpy(buff,"Soy yo...");
	send(pipeName2, buff);


	buff2[0] = 0;
	while(buff2[0] == 0) {
		receive(pipeName, buff2);
	}
	printf("%d - Joaquin: %s...\n", count++, buff2);



	mySleep();

	strcpy(buff,"A ti ...");
	send(pipeName2, buff);

	buff2[0] = 0;
	while(buff2[0] == 0) {
		receive(pipeName, buff2);
	}
	printf("%d - Joaquin: %s...\n", count++, buff2);


	mySleep();
	strcpy(buff,"Porque...");
	send(pipeName2, buff);

	buff2[0] = 0;
	while(buff2[0] == 0) {
		receive(pipeName, buff2);
	}
	printf("%d - Joaquin: %s...\n", count++, buff2);


	mySleep();mySleep();mySleep();

	printf("Terminado ella\n");

	mySleep();mySleep();mySleep();
}

void testPipes1() {
	ppid = getCurrentPid();
	char * buff = (char *) malloc(200);
	char * buff2 = (char *) malloc(200);

	/* Create process */
	char * name = (char *) malloc(sizeof(char)*10);
	name[0] = 'p';name[1] = 'i';name[2] = 'p';name[3] = 'e';name[4] = '2';name[5] = 0;
	int childPid = pcreate(exec, testPipes2, name);


	printf("Cargando joaquin...\n");

	printf("Afinando joaquin...\n");

	pipeName = createPipe(ppid, childPid);

	while(pipeName == NULL || pipeName2 == NULL);

	mySleep();

	strcpy(buff,"Quien es...");
	send(pipeName, buff);

	buff2[0] = 0;
	while(buff2[0] == 0) {
		receive(pipeName2, buff2);
	}
	printf("%d - Ella: %s...\n",count++, buff2);




	mySleep();

	strcpy(buff,"Que vienes a buscar...");
	send(pipeName, buff);

	buff2[0] = 0;
	while(buff2[0] == 0) {
		receive(pipeName2, buff2);
	}
	printf("%d - Ella: %s...\n",count++, buff2);




	mySleep();

	strcpy(buff,"Ya es tarde ...");
	send(pipeName, buff);

	buff2[0] = 0;
	while(buff2[0] == 0) {
		receive(pipeName2, buff2);
	}
	printf("%d - Ella: %s...\n", count++, buff2);



	mySleep();

	strcpy(buff,"Porque ahora soy yo la que quiere estar sin ti ...");
	send(pipeName, buff);

	buff2[0] = 0;
	while(buff2[0] == 0) {
		receive(pipeName2, buff2);
	}
	printf("%d - Ella: %s...\n", count++, buff2);


	mySleep();mySleep();mySleep();

	printf("Terminado el\n");

	mySleep();mySleep();mySleep();

}
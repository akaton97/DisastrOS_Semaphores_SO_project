#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include "disastrOS.h"
#include "disastrOS.c"
//numeri da definire meglio durante i test successivi
//definisco delle macro per la lunghezza del buffer e le iterazioni da fare su di esso
#define BUFFER_LENGTH 100
#define ITERATION 20

//inizializzo il buffer e gli indici di scrittura e lettura dei semafori più i rispettivi semafori
int filled_sem, empty_sem , read_sem , write_sem;
int buffer[BUFFER_LENGTH];
int w_index = 0;
int r_index = 0;

// variabile condivisa
unsigned long int shared_variable;

int consumer(){
	
	disastrOS_semWait(filled_sem);
	disastrOS_semWait(read_sem);
	
	int var_cons = buffer[r_index];
	//evitiamo overflow, rilegendo eventualmente zone di memoria iniziali
	r_index = (r_index + 1) % BUFFER_LENGTH;
	
	disastrOS_semPost(empty_sem);
	disastrOS_semPost(write_sem);
	return var_cons;
}

int producer(){
	disastrOS_semWait(empty_sem);
	disastrOS_semWait(write_sem);

	int var_prod = shared_variable;
	buffer[w_index] = shared_variable;
	//evitiamo overflow, riscrivendo eventualmente zone di memoria iniziali
	w_index = (w_index + 1) % BUFFER_LENGTH;
	shared_variable ++;
	
	disastrOS_semPost(filled_sem);
	disastrOS_semPost(read_sem);
	
	return var_prod;
}

// we need this to handle the sleep state
void sleeperFunction(void* args){
  printf("Hello, I am the sleeper, and I sleep %d\n",disastrOS_getpid());
  while(1) {
    getc(stdin);
    disastrOS_printStatus();
  }
}

void childFunction(void* args){

  //creazione e apertura dei semafori
  printf("Child - creazione ed apertura dei semafori\n");
  
  empty_sem = disastrOS_semOpen(1,0);
  filled_sem = disastrOS_semOpen(2,BUFFER_LENGTH);
  read_sem = disastrOS_semOpen(3,1);
  write_sem = disastrOS_semOpen(4,1);

  for (int i=0; i<ITERATION; ++i){
	
	if((disastrOS_getpid()%2)==0){
		int write_val = producer();
		printf("Child-Info - Thread #%d: valore buffer scritto: %d \n",disastrOS_getpid(),(write_val));
	}
	
	else {
		int read_val = consumer();
		printf("Child-Info - Thread #%d: valore buffer letto: %d \n",disastrOS_getpid(),(read_val));
	}
	
   }
  
  //chiusura semafori e uscita del figlio
  printf("Child - chiusura semafori\n");
  disastrOS_semClose(empty_sem); 
  disastrOS_semClose(filled_sem); 
  disastrOS_semClose(read_sem); 
  disastrOS_semClose(write_sem); 
  disastrOS_exit(disastrOS_getpid()+1);
}


void initFunction(void* args) {
	
  disastrOS_printStatus();
  printf(" ### Start ###");
  //disastrOS_spawn(sleeperFunction, 0);
  
  //stampo lo stato iniziale del buffer ai fini della verifica del test
  printf(" ---- Stato iniziale Buffer ---- ");
  for(int buf=0; buf < BUFFER_LENGTH; buf++){
	  printf("%d",buffer[buf]);
  }
  printf("\n");
  
  shared_variable = 1; //valore della variabile condivisa

  printf("I feel like to spawn 10 nice threads\n");
  int alive_children=0;
  
  for (int i=0; i<10; ++i) {
    disastrOS_spawn(childFunction, 0);
    alive_children++;
  }

  disastrOS_printStatus();
  int retval;
  int pid;
  while(alive_children>0 && (pid=disastrOS_wait(0, &retval))>=0){ 
    disastrOS_printStatus();
    printf("initFunction, child: %d terminated, retval:%d, alive: %d \n",
	   pid, retval, alive_children);
    --alive_children;
  }
  
  //stampo lo stato finale del buffer ai fini della verifica del test con quello iniziale
  printf(" ---- Stato iniziale Buffer ---- ");
  for(int buf=0; buf < BUFFER_LENGTH; buf++){
	  printf("%d",buffer[buf]);
  }
  printf("\n");
  
  printf(" ### shutdown ###");
  disastrOS_shutdown();
}

int main(int argc, char** argv){
  char* logfilename=0;
  if (argc>1) {
    logfilename=argv[1];
  }
  // we create the init process processes
  // the first is in the running variable
  // the others are in the ready queue
  printf("the function pointer is: %p", childFunction);
  // spawn an init process
  printf("start\n");
  disastrOS_start(initFunction, 0, logfilename);
  return 0;
}

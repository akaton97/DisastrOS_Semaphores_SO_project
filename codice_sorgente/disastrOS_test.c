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
int buffer[BUFFER_LENGTH];
int w_index = 0;
int r_index = 0;

// variabile condivisa
int shared_variable = 0;

void childFunction(void* args){

  printf("Hello, I am the child function %d\n",disastrOS_getpid());
  printf("I will iterate a bit, before terminating\n");
  int type=0;
  int mode=0;
  int fd = disastrOS_openResource(disastrOS_getpid(),type,mode);
  printf("fd=%d\n",fd);
  printf("PID: %d, terminating\n", disastrOS_getpid());
  
  //creazione e apertura dei semafori
  printf("Child - creazione ed apertura dei semafori\n");
  
  int empty_sem = disastrOS_semOpen(1,0);
  int filled_sem = disastrOS_semOpen(2,BUFFER_LENGTH);
  int read_sem = disastrOS_semOpen(3,1);
  int write_sem = disastrOS_semOpen(4,1);
  
  printf("Child - fine apertura ed apertura dei semafori\n");

  for (int i=0; i<ITERATION; ++i){
	
	if((disastrOS_getpid()%2)==0){
		
		disastrOS_semWait(filled_sem);
		disastrOS_semWait(write_sem);

		int var_prod = shared_variable;
		buffer[w_index] = shared_variable;
		//evitiamo overflow, riscrivendo eventualmente la zona di memoria in modo circolare 
		w_index = (w_index + 1) % BUFFER_LENGTH;
		shared_variable ++;
		disastrOS_sleep(10);
	
		disastrOS_semPost(write_sem);
		disastrOS_semPost(empty_sem);
		
		printf("Child-Info - Thread #%d: valore buffer scritto: %d \n",disastrOS_getpid(),(var_prod));
	}
	
	else {
		
		disastrOS_semWait(empty_sem);
		disastrOS_semWait(read_sem);
	
		int var_cons = buffer[r_index];
		//evitiamo overflow, rilegendo eventualmente zone di memoria iniziali
		r_index = (r_index + 1) % BUFFER_LENGTH;
		disastrOS_sleep(10);
	
		disastrOS_semPost(read_sem);
		disastrOS_semPost(filled_sem);
		
		printf("Child-Info - Thread #%d: valore buffer letto: %d \n",disastrOS_getpid(),(var_cons));	
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


// we need this to handle the sleep state
void sleeperFunction(void* args){
  printf("Hello, I am the sleeper, and I sleep %d\n",disastrOS_getpid());
  while(1) {
    getc(stdin);
    disastrOS_printStatus();
  }
}


void initFunction(void* args) {
	
  disastrOS_printStatus();
  printf(" ### Start-Init ###\n");
  disastrOS_spawn(sleeperFunction, 0);
  int fd[ITERATION];

  printf("I feel like to spawn %d nice threads\n", ITERATION);
  int alive_children=0;
  
  for (int i=0; i<ITERATION; ++i) {
	int type=0;
    int mode=DSOS_CREATE;
    printf("mode: %d\n", mode);
    printf("opening resource (and creating if necessary)\n");
    fd[i]=disastrOS_openResource(i,type,mode);
    printf("fd=%d\n", fd[i]);
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
  
  printf("dealloco risorse\n");
  int i;
  for (i = 0; i <ITERATION; ++i)
  {
	disastrOS_closeResource(fd[i]);
	disastrOS_destroyResource(i);
	disastrOS_printStatus();
  } 
  
  printf("STATO FINALE DEL SISTEMA:\n");
  disastrOS_printStatus();
  printf("BUFFER ALLA FINE\n");
  for(i=0;i<BUFFER_LENGTH;i++){
	  printf("%d ", buffer[i]);
  }
  printf("\n");
  printf(" ### shutdown ###\n");
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

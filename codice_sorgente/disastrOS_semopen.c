#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"
#include "disastrOS_globals.h"

void internal_semOpen(){
	
	printf("going to open a sem \n");
	
	/*argomenti della syscall per gestire il semaforo
	 * [0] è l'id del semaforo
	 * [1] è il counter del semaforo
 	*/ 
	int semid = running->syscall_args[0];
	int counter = running->syscall_args[1];
	
	if(semid < 0){
		printf("ERRORE - semid negativo \n");
		running->syscall_retvalue = DSOS_EWRONG_ID;
		return;
	}
	
	//verifico se il semaforo esiste giò
	Semaphore* aux = SemaphoreList_byId(&semaphores_list,semid);
	
	if(aux==NULL){
		
		//alloco il semaforo
		aux = Semaphore_alloc(semid,counter);
		
		if(!aux){
			printf("ERRORE - allocazione semaforo \n");
			running->syscall_retvalue = DSOS_ESEM_ALLOC;
			return;
		}
		
		//inserisco
		List_insert(&semaphores_list,semaphores_list.last,(ListItem*)aux);

	}
	
	//in ogni caso ora devo allocare il descrittore
	SemDescriptor* sds = SemDescriptor_alloc(running -> last_sem_fd,aux,running);
	
	if(!sds){
		printf("ERRORE - allocazione descrittore \n");
		running->syscall_retvalue = DSOS_ESEM_DES_ALLOC;
		return;
	}
	
	//allocazione puntatore descrittore
	SemDescriptorPtr* sdsptr = SemDescriptorPtr_alloc(sds);
	
	if(!sdsptr){
		printf("ERRORE - allocazione puntatore a descrittore");
		running->syscall_retvalue = DSOS_ESEM_DES_PTR_ALLOC;
		return;
	}
	
	//aggiorno la lista di puntatori ai descrittori dei semaforiChild-Info
	SemDescriptor* sem_des =(SemDescriptor*)List_insert(&running -> sem_descriptors,running -> sem_descriptors.last, (ListItem*)sds);
	
	if(!sem_des){
		printf("ERRORE - nell'aggiornamento della lista puntatori \n");
		running->syscall_retvalue = DSOS_ELIST_INSERT;
		return;
	}
	
	sds -> ptr = sdsptr; //descrittore nella struct del descrittore

	//aggiunta ptr del descrittore del sem alla lista dei descrittori
	SemDescriptorPtr* auxPtr = (SemDescriptorPtr*)List_insert(&aux -> descriptors, aux -> descriptors.last,(ListItem*)(sds -> ptr));
	
	if(!auxPtr){
		printf("ERRORE - inserimento puntatore \n");
		running->syscall_retvalue = DSOS_ELIST_INSERT;
		return;
	}
	
	//incremento puntatore al pcb dei semafori aperti
	(running -> last_sem_fd)++;

	//assegno come ret-value della syscall il fd del semaforo
	running->syscall_retvalue = sds->fd; 
	
	disastrOS_printStatus();
}

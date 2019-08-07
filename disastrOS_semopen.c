#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"
#include "disastrOS_globals.h"

void internal_semOpen(){
	
	//argomenti della syscall per gestire il semaforo
	int semid = running->syscall_args[0];
	int counter = running->syscall_args[1];
	void* ret;
	
	if(semid < 0){
		printf("Errore, semid negativo");
		return;
	}
	
	//verifico se il semaforo esiste giò
	Semaphore* aux = SemaphoreList_byId(&semaphores_list,semid);
	
	if(aux==NULL){
		
		//alloco il semaforo
		Semaphore* aux = Semaphore_alloc(semid,counter);
		
		if(!aux){
			printf("errore allocazione semaforo");
			return;
		}
		
		//inserisco
		ret = List_insert(&semaphores_list,semaphores_list.last,(ListItem*)aux);
		if(ret==NULL){
			printf("errore nell'inserimento del sem");
			return;
		}
	}
	
	//in ogni caso ora devo allocare il descrittore
	SemDescriptor* sds = SemDescriptor_alloc(running -> last_sem_fd,aux,running);
	
	if(!sds){
		printf("errore allocazione descrittore");
		return;
	}
	
	//allocazione puntatore descrittore
	SemDescriptorPtr* sdsptr = SemDescriptorPtr_alloc(sds);
	
	if(!sdsptr){
		printf("errore allocazione puntatore a descrittore");
		return;
	}
	
	//aggiorno la lista di puntatori ai descrittori dei semafori
	SemDescriptor* reta =(SemDescriptor*) List_insert(&running -> sem_descriptors,running -> sem_descriptors.last, (ListItem*) sdsptr);
	
	if(!reta){
		printf("errore nell'aggiornamento della lista puntatori");
		return;
	}
	
	sds -> ptr = sdsptr; //descrittore nella struct del descrittore

	//aggiunta ptr del descrittore del sem alla lista dei descrittori
	SemDescriptorPtr* auxPtr = (SemDescriptorPtr*) List_insert(&aux -> descriptors, aux -> descriptors.last,(ListItem*)(sds -> ptr));
	if(!auxPtr){
		printf("errore inserimento puntatore");
		return;
	}
	
	//incremento puntatore al pcb dei semafori aperti
	(running -> last_sem_fd)++;
}

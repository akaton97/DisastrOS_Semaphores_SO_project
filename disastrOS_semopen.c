#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semOpen(){
	
	//argomenti della syscall per gestire il semaforo
	int semid = running -> syscall_args[0];
	int counter = running -> syscall_args[1];
	int ret;
	
	//verifico se il semaforo esiste giò
	Semaphore* aux = SemaphoreList_byId(&semaphore_list,semid);
	
	if(aux==NULL){
		//alloco il semaforo
		Semaphore* sem = Semaphore_alloc(semid,counter);
		//inserisco
		ret = List_insert(&semaphore_list,semaphores_list.last,sem);
		if(ret==NULL){
			printf("errore nell'inserimento del sem");
			return;
		}
	}
	
	//in ogni caso ora devo allocare il descrittore
	SemDescriptor* sds = SemDescriptor_alloc(running -> last_sem_fd,sem,running);
	
	SemDescriptorPtr* sdsptr = SemDescriptorPtr_alloc(sds);
	
	//aggiorno la lista di puntatori ai descrittori dei semafori
	ret = List_insert(running -> sem_descriptor, sem_descriptor.last, sdsptr);
	if(ret==NULL){
		printf("errore nell'aggiornamento della lista puntatori");
		return;
	}
}

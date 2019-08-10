#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semClose(){
	
	int semid = running -> syscall_args[0];
	int ret ;
	
	//ricerca descrittore
	SemDescriptor* sds = SemDescriptorList_byFd(&running->sem_descriptors, semid);
	
	if(sds == NULL){
		printf("descrittore non trovato");
		return;
	}
	
	//rimozione descrittore
	sds = (SemDescriptor*)List_detach(&running->sem_descriptors, (ListItem*)sds);
	if(sds== NULL){
		printf("errore nella rimozione del descrittore");
		return;
	}
	
	//semaforo associato al descrittore
	Semaphore* sem = sds->semaphore;
	
	//prendo il puntatore dalla lista semafori per chiamare poi la free
	SemDescriptorPtr* sdptr = (SemDescriptor*)List_detach(&sem->descriptors, (ListItem*)sds->ptr);
	if(!sdptr){
		printf("errore nella rimozione del ptr al descrittore dalla lista");
		return;
	}
	
	ret = SemDescriptorPtr_free(sdptr);
	if(ret){
		printf("errore nella free del ptr al descrittore");
		return;
	}
	
	ret = SemDescriptor_free(sds);
	if(ret){
		printf("errore nella free del descrittore");
		return;
	}
	
	//faccio una verifica e chiudo il semaforo 
	if(sem->descriptors.size == 0 && sem->waiting_descriptors.size == 0){
		printf("chiusura semaforo");
		Semaphore* aux = (Semaphore*)List_detach(&semaphores_list,(ListItem*)sem);
		if(!aux){
			printf("errore rimozione semaforo");
			return;
		}
		ret = Semaphore_free(sem);
		if(ret){
			printf("errore free del semaforo");
			return;
		}
		disastrOS_printStatus();
	}
		
	running->last_sem_fd --;
	
}

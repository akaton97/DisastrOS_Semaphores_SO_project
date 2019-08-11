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
	
	if(!sds){
		printf("ERRORE - descrittore non trovato");
		running->syscall_retvalue=DSOS_ESEM_DES_LIST;
		return;
	}
	
	//rimozione descrittore
	sds = (SemDescriptor*)List_detach(&running->sem_descriptors, (ListItem*)sds);
	if(!sds){
		printf("ERRORE - nella rimozione del descrittore");
		 running->syscall_retvalue = DSOS_ELIST_DETACH;
		return;
	}
	
	//semaforo associato al descrittore
	Semaphore* sem = sds->semaphore;
	
	//prendo il puntatore dalla lista semafori per chiamare poi la free
	SemDescriptorPtr* sdptr = (SemDescriptor*)List_detach(&sem->descriptors, (ListItem*)sds->ptr);
	if(!sdptr){
		printf("ERRORE - nella rimozione del ptr al descrittore dalla lista");
		return;
	}
	
	ret = SemDescriptorPtr_free(sdptr);
	if(ret){
		printf("ERRORE - nella free del ptr al descrittore");
		running->syscall_retvalue = DSOS_ESEM_DES_PTR_FREE;
		return;
	}
	
	ret = SemDescriptor_free(sds);
	if(ret){
		printf("ERRORE - nella free del descrittore");
		running->syscall_retvalue = DSOS_ESEM_DES_FREE;
		return;
	}
	
	//faccio una verifica e chiudo il semaforo 
	if(sem->descriptors.size == 0 && sem->waiting_descriptors.size == 0){
		printf("chiusura semaforo");
		Semaphore* aux = (Semaphore*)List_detach(&semaphores_list,(ListItem*)sem);
		if(!aux){
			printf("ERRORE - rimozione semaforo");
			 running->syscall_retvalue = DSOS_ELIST_DETACH;
			return;
		}
		ret = Semaphore_free(sem);
		if(ret){
			printf("ERRORE - free del semaforo");
			running->syscall_retvalue = DSOS_ESEM_FREE;
			return;
		}
		disastrOS_printStatus();
	}
		
	running->last_sem_fd --;
	
	running -> syscall_retvalue = 0;
}

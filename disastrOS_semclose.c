#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semClose(){
	
	int semid = running -> syscall_args[0];
	void* ret;
	
	//ricerca descrittore
	SemDescriptor* sds = SemDescriptorList_byFd(&running->sem_descriptors, semid);
	
	if(sds == NULL){
		printf("descrittore non trovato");
		return;
	}
	
	//rimozione descrittore
	ret = (SemDescriptor*)List_detach(&running->sem_descriptors, (ListItem*)sds);
	if(ret== NULL){
		printf("errore nella rimozione del descrittore");
		return;
	}
	
	Semaphore* sem = sds->semaphore;
	
	//rimozione puntatore
	ret = List_detach(&sem->descriptors, (ListItem*)sds->ptr);
	if(ret==NULL){
		printf("errore nella rimozione del puntatore");
		return;
	}
}

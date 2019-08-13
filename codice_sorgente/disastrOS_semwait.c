#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semWait(){
  //estraggo l'id del semaforo
  int semid = running->syscall_args[0];
  //prendo il descrittore del semaforo assocciato all'id
  SemDescriptor* semdesc = SemDescriptorList_byFd(&running->sem_descriptors, semid);
  //controllo se ho trovato il descrittore, altrimento termino
  if(semdesc==0){
    printf("[SEMAFORO] semWait() del semaforo id=%d fallita\n",semid);
    running->syscall_retvalue= DSOS_ESEM_DES_LIST;
	return;
  }
  //salvo il semaforo per riutilizzarlo
  Semaphore* sem = semdesc->semaphore;
  //salvo il puntatore del descrittore
  SemDescriptorPtr* desc_ptr = semdesc->ptr;
  //decremento il contatore del SEMAFORO
  sem->count--;
  //controllo se il contatore del semaforo è sceso sotto lo 0
  if(sem->count < 0){
    //estraggo il descrittore dalla lista dei descrittori
    SemDescriptorPtr* desc_aux = (SemDescriptorPtr*)List_detach(&sem->descriptors,(ListItem*)desc_ptr);
    if(!desc_aux){
		  printf("[SEMAFORO] semWait(): estrazione del puntatore a descrittore dalla lista dei descrittori del semaforo fallita\n");
		  running->syscall_retvalue= DSOS_ELIST_DETACH;
	  }
    //lo infilo nella lista di waiting
    desc_aux = (SemDescriptorPtr*)List_insert(&sem->waiting_descriptors, sem->waiting_descriptors.last, (ListItem*) semdesc->ptr);
    if(!desc_aux){
		  printf("[SEMAFORO] semWait(): inserimento del puntatore a descrittore nella lista di waiting del semaforo fallito\n");
		  running->syscall_retvalue= DSOS_ELIST_INSERT;
		  return;
	  }
    //imposto lo status del programma attuale in waiting
    running->status = Waiting;
    //inserisco il processo in esecuzione dentro la waiting list
    PCB* pcbAux = (PCB*)List_insert(&waiting_list, waiting_list.last, (ListItem*) running);
    if(!pcbAux){
		  printf("[SEMAFORO] semWait():  inserimento del processo nella lista di attesa del sistema fallito\n");
		  running->syscall_retvalue= DSOS_ELIST_INSERT;
		  return;
	  }
    //prendo il primo elemento disponibile dalla ready list
    pcbAux = (PCB*)List_detach(&ready_list, (ListItem*)ready_list.first);
    if(!pcbAux){
		  printf("[SEMAFORO] semWait(): rimozione del processo dalla ready queue del sistema fallita\n");
		  running->syscall_retvalue= DSOS_ELIST_DETACH;
		  return;
	  }
    //mando in esecuzione l'elemento appena preso dalla ready list
    running = (PCB*)pcbAux; 
  }
  return;
}

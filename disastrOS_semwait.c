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
    //in questo caso, estraggo il descrittore dalla lista dei descrittori
    List_detach(&sem->descriptors,(ListItem*)desc_ptr);
    //lo infilo nella lista di waiting
    List_insert(&sem->waiting_descriptors, sem->waiting_descriptors.last, (ListItem*) semdesc->ptr);
    //imposto lo status del programma attuale in waiting
    running->status = Waiting;
    //inserisco il processo in esecuzione dentro la waiting list
    List_insert(&waiting_list, waiting_list.last, (ListItem*) running);
    //prendo il primo elemento disponibile dalla ready list
    PCB* pcbReady = (PCB*) List_detach(&ready_list, (ListItem*)ready_list.first);
    //mando in esecuzione l'elemento appena preso dalla ready list
    running = (PCB*)pcbReady; 
  }
  return;
}

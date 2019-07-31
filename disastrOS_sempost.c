#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semPost(){
    //prendo l'id del semaforo
    int id = running->syscall_args[0];
    //estraggo il descrittore del semaforo con quell'id dalla lista nel PCB
    SemDescriptor* sem_desc = SemDescriptorList_byFd(&running->sem_descriptors, id);
    //effettuo un controllo sul descrittore per verificare se l'ho effettivamente trovato
    if(sem_desc==0){ 
        printf("[SEMAFORI] SEMPOST FALLITA - il semaforo (id)%d\n non esiste!", id);
	//impongo, come valore di ritorno, l'errore standard per i semafori di disastrOS
        running->syscall_retvalue = DSOS_ESEMPOST; 
        return;
    }
    
    Semaphore* sem = sem_desc->semaphore; //prendo il semaforo associato al descrittore
    sem->count++; //incremento contatore di semafori
}

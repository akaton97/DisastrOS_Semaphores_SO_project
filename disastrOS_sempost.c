#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"
#include "disastrOS_globals.h"

void internal_semPost(){
    //prendo l'id del semaforo
    int id = running->syscall_args[0];
    //estraggo il descrittore del semaforo con quell'id dalla lista nel PCB
    SemDescriptor* sem_desc = SemDescriptorList_byFd(&running->sem_descriptors, id);
    //effettuo un controllo sul descrittore per verificare se l'ho effettivamente trovato
    if(sem_desc==0){
        printf("[SEMAFORI] SEMPOST FALLITA - il semaforo (id)%d\n non esiste!", id);
	//impongo, come valore di ritorno, l'errore standard per i semafori di disastrOS
        return;
    }
    Semaphore* sem = sem_desc->semaphore; //prendo il semaforo associato al descrittore
    sem->count++; //incremento contatore di semafori
    if(sem->count <= 0){
		//estraggo il primo descrittore del semaforo dalla lista dei descrittori in attesa
		SemDescriptorPtr* d_ptr_aux = (SemDescriptorPtr*) List_detach(&sem->waiting_descriptors, (ListItem*) sem->waiting_descriptors.first);
		if(!d_ptr_aux){
			printf("[SEMAFORI] errore nella rimozione del descrittore dalla lista di waiting del semaforo\n");
			return;
		}
		//inserisco il puntatore al descrittore nella lista dei descrittori del semaforo
		SemDescriptorPtr* dptr = (SemDescriptorPtr*) List_insert(&sem->descriptors, sem->descriptors.last, (ListItem*) d_ptr_aux);
		if(!dptr){
			printf("[SEMAFORI] errore nell'inserimetno del puntatore a descrittore nella lista dei descrittori del semaforo\n");
		}
		//imposto lo stato del pcb del processo attuale in ready
		PCB* exPCB = d_ptr_aux->descriptor->pcb;
		exPCB->status = Ready;
		//sposto il pcb del processo corrente nella lista di ready del sistema operativo
		PCB* pcbAux = (PCB*)List_detach(&waiting_list, (ListItem*) exPCB);
		if(pcbAux==NULL){
				printf("[SEMAFORI] errore nella rimozione del processo dalla waiting list\n");
		}
		//inserisco il processo attuale nella fila di ready
		pcbAux = (PCB*)List_insert(&ready_list, ready_list.last, (ListItem*) exPCB);
        if(!pcbAux){
			printf("[SEMAFORO] errore nell'inserimento del processo nella fila di ready\n");
		}
	}
}

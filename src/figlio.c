#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/msg.h>

#include <figlio.h>
#include <helpers.h>
#include <constants.h>
#include <types.h>
#include <nipote.h>

struct Status* status;

int figlio(int lines) {
    // Catturo la signal SIGUSR1
    signal(SIGUSR1, status_updated);

    // Creo il semaforo utilizzato dai sottoprocessi
    int semid;
    if((semid = semget(SEM_KEY, 1, IPC_CREAT | 0666)) == -1 ) {
        syserr("figlio", "impossibile creare il semaforo");
    }

    struct sembuf *sops = (struct sembuf*)malloc(sizeof(struct sembuf));
    sops->sem_num = 0;
    sops->sem_op = 1;
    sops->sem_flg = 0;

    if (semop(semid, sops, 1) == -1) {
        syserr("figlio", "impossibile impostare il semaforo a 1");
    }

    // Ottengo il segmento di memoria condiviso per l'input
    int shmid;
    if((shmid = shmget(SHMKEY_INPUT, sizeof(struct Status), 0666)) < 0) {
        syserr("figlio", "shmget");
    }

    void* shm;
    if ((shm = shmat (shmid , NULL , 0)) == ( void *) -1) {
	  	syserr("figlio", "shmat");
	}

    status = (struct Status*)shm;
    // Creo nipote 1
    pid_t pid_nipote1 = fork();
    if(pid_nipote1 == -1) {
        syserr("figlio", "impossibile creare nipote");
    }
    else if(pid_nipote1 == 0) {
        return nipote(1, lines);
    }

    pid_t pid_nipote2 = fork();
    if(pid_nipote2 == -1) {
        syserr("figlio", "impossibile creare nipote");
    }
    else if (pid_nipote2 == 0) {
        return nipote(2, lines);
    }

    // Attendo la terminazione di entrambi i nipoti
    wait(&pid_nipote1);
    wait(&pid_nipote2);

    // Deposito il messaggio di fine
    struct Message* message = (struct Message*)malloc(sizeof(struct Message));

    // Apertura della coda di messaggi corrispondente alla chiave MSGKEY
    int msgid;
	if((msgid = msgget(MSGKEY, 0666)) == -1) {
		syserr("figlio", "Apertura della coda di messaggi fallita.\n");
    }
    
    // Creo il messaggio da inviare
    char tmp[128] = "Ricerca conclusa";
    message->mtype = 1;
    for(int i = 0; i < 128; i++) {
        message->text[i] = tmp[i];
    }
    // Spedisco il messaggio e rimango in attesa se la coda è piena
    if(msgsnd(msgid, message, sizeof(struct Message) - sizeof(message->mtype), 0) == -1) {
        syserr("figlio", "msgsnd");
    }

    // Elimino il semaforo
    if(semctl(semid, 0, IPC_RMID) == -1) {
        syserr("figlio", "semctl");
    }
    printf("Semaforo eliminato\n");

    // TODO: deposito il messaggio sulla coda del processo logger
}

void status_updated() {
    char* stringa= "ciao\0";
    //printing("ciao");
    printf("Il nipote %d sta analizzando la stringa %d \n",  status->granson, status->id_string);
}

void send_terminate() {

}
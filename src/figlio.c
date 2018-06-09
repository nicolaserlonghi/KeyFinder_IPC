#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <pthread.h>

#include <helpers.h>
#include <constants.h>
#include <types.h>
#include <nipote.h>
#include <figlio.h>


int figlio(int lines) {
    int semid;

    // Catturo la signal SIGUSR1
    signal(SIGUSR1, status_updated);

    // Creo il semaforo utilizzato dai sottoprocessi
    if((semid = semget(SEM_KEY, 2, IPC_CREAT | 0666)) == -1 ) {
        syserr("figlio", "impossibile creare il semaforo");
    }

    struct sembuf *sops = (struct sembuf*)malloc(sizeof(struct sembuf));
    sops->sem_num = 0;
    sops->sem_op = 1;
    sops->sem_flg = 0;

    if (semop(semid, sops, 1) == -1) {
        syserr("figlio", "impossibile impostare il semaforo 1");
    }

    sops->sem_num = 1;
    sops->sem_op = 0;
    sops->sem_flg = 0;

    if (semop(semid, sops, 1) == -1) {
        syserr("figlio", "impossibile impostare il semaforo 2");
    }
    free(sops);


    if(THREAD == 0) { 
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
    } else {
        int num_threads, i;

        pthread_t* threads;   // per tenere traccia degli identificatori delle thread
        struct Package* package;

        // Viene creata una thread per ogni riga del file di input
        threads = (pthread_t *) malloc(lines * sizeof(pthread_t));

        // Viene creato un thread per ogni riga del file di input
        num_threads = 0;
        for(i = 0; i < lines; i++) {

            package = (struct Package *)malloc(sizeof(struct Package));
            package->id = num_threads+1;
            package->lines = lines;

            pthread_create(&threads[num_threads], NULL, nipote_thread, package);

            num_threads++;
        }

       // Attendo che tutte le thread abbiano terminato la loro elaborazione
        for (i = 0; i < (lines); i++) {
            pthread_join(threads[i], NULL);
        }
        free(package);
        free(threads);
    }

    send_terminate();

    // Elimino il semaforo
    if(semctl(semid, 0, IPC_RMID) == -1) {
        syserr("figlio", "semctl");
    }

    char buffer[] = "Semaforo eliminato";
    printing(buffer);
}

void status_updated() {
    // Ottengo il segmento di memoria condiviso per l'input
    int shmid;
    if((shmid = shmget(SHMKEY_INPUT, sizeof(struct Status), 0666)) < 0) {
        syserr("figlio", "shmget");
    }

    void* shm;
    if ((shm = shmat (shmid , NULL , 0)) == ( void *) -1) {
	  	syserr("figlio", "shmat");
	}

    struct Status* status = (struct Status*)shm;

    char* granson = int_to_string(status->granson);
    char* id_string = int_to_string(status->id_string);
    char* tmp = "Il nipote ";
    char* buffer = concat_string("Il nipote ", granson);
    tmp = " sta analizzando la stringa ";
    buffer = concat_string(buffer, " sta analizzando la stringa ");
    buffer = concat_string(buffer, id_string);
    printing(buffer);
    unlock(1);
}

void send_terminate() {
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
    
    free(message);
}
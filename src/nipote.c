#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <pthread.h>

#include <constants.h>
#include <helpers.h>
#include <types.h>
#include <nipote.h>


int nipote(int mid, int lines) {
    
    // Ottengo il segmento di memoria condiviso per l'input
    int shmid_s1;
    if((shmid_s1 = shmget(SHMKEY_INPUT, sizeof(struct Status) + (sizeof(struct Line) * lines) + 1, 0666)) < 0) {
        syserr("nipote", "shmget");
    }
    void* shm_s1;
    if ((shm_s1 = shmat (shmid_s1 , NULL , 0)) == ( void *) -1) {
        syserr("nipote", "shmat");
    }

    struct Status* status = (struct Status*)shm_s1;
    struct Line* input = (struct Line*)(shm_s1 + sizeof(struct Status));
    // Ottengo il segmento di memoria condiviso per l'output
    int shmid_s2;
    if((shmid_s2 = shmget(SHMKEY_OUTPUT, sizeof(struct Status), 0666)) < 0) {
        syserr("nipote", "shmget");
    }
    void* shm_s2;
    if ((shm_s2 = shmat (shmid_s2 , NULL , 0)) == ( void *) -1) {
        syserr("nipote", "shmat");
    }
    void* output = (void*)shm_s2;

    while(load_string(lines, mid, status, input, output) == 0);

    return 0;
}

void* nipote_thread(void* arg) {
    struct Package* package = (struct Package *)arg;
    nipote(package->id, package->lines);

    return (NULL);
}


int load_string(int lines, int mid, struct Status* status, struct Line* input, void* output) {
    lock(0);
    int my_string = status->id_string;
    if(my_string == lines) {
        unlock(0);
        return -1;
    }
    status->granson = mid;
    status->id_string = ++my_string;
    // Segnalo lo stato
    if(THREAD == 0) {
        kill(getppid(), SIGUSR1);
    } else {
        kill(getpid(), SIGUSR1);
    }
    
    lock(1);
    unlock(0);
    struct Line* line = (struct Line*)(&input[my_string-1]);

    // Cerco la chiave
    find_key(line, (my_string-1), output);
    return 0;
}

void lock(int n_sem) {
     // Recupero il semaforo
    int semid;
    if(((semid = semget(SEM_KEY, 2, 0666)) == -1)) {
        syserr("nipote", "impossibile recuperare il semaforo");
    }
    struct sembuf *sops = (struct sembuf *)malloc(sizeof(struct sembuf));

    sops->sem_num = n_sem;
    sops->sem_op = -1;
    sops->sem_flg = 0;

    if (semop(semid, sops, 1) == -1) {
        syserr("nipote", "impossibile bloccare il semaforo");
    }

    free(sops);
}

void unlock(int n_sem) {
     // Recupero il semaforo
    int semid;
    if(((semid = semget(SEM_KEY, 2, 0666)) == -1)) {
        syserr("nipote", "impossibile recuperare il semaforo");
    }
    struct sembuf *sops = (struct sembuf *)malloc(sizeof(struct sembuf));

    sops->sem_num = n_sem;
    sops->sem_op = 1;
    sops->sem_flg = 0;

    if (semop(semid, sops, 1) == -1) {
        syserr("nipote", "impossibile sbloccare il semaforo");
    }

    free(sops);
}

void find_key(struct Line* line, int my_string, void* output) {
    int fine_stringa = 0;
    char clear[512];
    char encrypt[512];
    int i = 0, j = 0;
    unsigned key = 0;
    struct timeval start, finish;

    // Cerco la chiave di criptazione
    gettimeofday(&start, NULL); 
    while((line->clear ^ key) != line->encrypt) {
        key++;
    }
    gettimeofday(&finish, NULL);
    // Calcolo il tempo impiegato
    int time_spent = (int)(finish.tv_nsec - start.tv_nsec);

    save_key(key, my_string, output);
    // deposito il messaggio
    send_timeelapsed(time_spent);
}

void send_timeelapsed(int time_spent) {
    struct Message* message = (struct Message*)malloc(sizeof(struct Message));

    // Apertura della coda di messaggi corrispondente alla chiave MSGKEY
    int msgid;
	if((msgid = msgget(MSGKEY, 0666)) == -1) {
		syserr("nipote", "Apertura della coda di messaggi fallita.\n");
		exit(1);
    }
    // Converto il tempo in stringa
    char* time = int_to_string(time_spent);
    // Creo il messaggio da inviare
    char* tmp = "Chiave trovata in ";
    char* buffer = concat_string(tmp, time);
    int i = 0;
    for(i = 0; i < string_length(buffer); i++) {
        message->text[i] = buffer[i];
    }
    message->mtype = 2;
    // Spedisco il messaggio e rimango in attesa se la coda è piena
    if(msgsnd(msgid, message, sizeof(struct Message) - sizeof(message->mtype), 0) == -1) {
        syserr("nipote", "msgsnd");
        exit(1);
    }

    free(message);
}

void save_key(unsigned key, int my_string, void* output) {
    // Copio la chiave nello spazio di memoria condiviso
    unsigned* write = (unsigned*) output + (my_string); 
    *write = key;
}
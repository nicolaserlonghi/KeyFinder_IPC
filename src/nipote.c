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
#include <time.h>
#include <pthread.h>

#include <constants.h>
#include <helpers.h>
#include <types.h>
#include <nipote.h>


pid_t figlio_pid;
int semid;
struct Status* status;
struct Line* input;
void* output;
int id;
time_t start, finish;

int nipote(int mid, int lines) {
    int mlines = lines;
    id = mid;
    
    // TODO: Vedere se tenere
    if(THREAD == 0) {
        figlio_pid = getppid();
    } else {
        figlio_pid = getpid();
    }
    
    // Recupero il semaforo
    if(((semid = semget(SEM_KEY, 2, 0666)) == -1)) {
        syserr("nipote", "impossibile recuperare il semaforo");
    }
    
    // Ottengo il segmento di memoria condiviso per l'input
    int shmid_s1;
    if((shmid_s1 = shmget(SHMKEY_INPUT, sizeof(struct Status) + (sizeof(struct Line) * lines) + 1, 0666)) < 0) {
        syserr("nipote", "shmget");
    }
    void* shm_s1;
    if ((shm_s1 = shmat (shmid_s1 , NULL , 0)) == ( void *) -1) {
        syserr("nipote", "shmat");
    }

    status = (struct Status*)shm_s1;
    input = (struct Line*)(shm_s1 + sizeof(struct Status));
    // Ottengo il segmento di memoria condiviso per l'output
    int shmid_s2;
    if((shmid_s2 = shmget(SHMKEY_OUTPUT, sizeof(struct Status), 0666)) < 0) {
        syserr("nipote", "shmget");
    }
    void* shm_s2;
    if ((shm_s2 = shmat (shmid_s2 , NULL , 0)) == ( void *) -1) {
        syserr("nipote", "shmat");
    }
    output = (void*)shm_s2;

    while(load_string(mlines) == 0);

    return 0;
}

void* nipote_thread(void* arg) {
    struct Package* package = (struct Package *)arg;
    nipote(package->id, package->lines);

    return (NULL);
}


int load_string(int lines) {
    lock(0);
    int my_string = status->id_string;
    if(my_string == lines) {
        unlock(0);
        return -1;
    }
    status->granson = id;
    status->id_string = ++my_string;
    // Segnalo lo stato
    kill(figlio_pid, SIGUSR1);
    lock(1);
    unlock(0);
    struct Line* line = (struct Line*)(input + ((my_string-1)* sizeof(struct Line)));

    // Cerco la chiave
    find_key(line, (my_string-1));
    return 0;
}

void lock(int n_sem) {
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
    struct sembuf *sops = (struct sembuf *)malloc(sizeof(struct sembuf));

    sops->sem_num = n_sem;
    sops->sem_op = 1;
    sops->sem_flg = 0;

    if (semop(semid, sops, 1) == -1) {
        syserr("nipote", "impossibile sbloccare il semaforo");
    }

    free(sops);
}

void find_key(struct Line* line, int my_string) {
    int fine_stringa = 0;
    char clear[512];
    char encrypt[512];
    int i = 0, j = 0;
    unsigned key = 0;
    // Cerco la chiave di criptazione
    clock_t begin = clock();
    while((line->clear ^ key) != line->encrypt) {
        key++;
    }
    clock_t end = clock();
    // Calcolo il tempo impiegato
    int time_spent = (int)(end - begin) / CLOCKS_PER_SEC;
    save_key(key, my_string);
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
    for(int i = 0; i < string_length(buffer); i++) {
        message->text[i] = buffer[i];
    }
    message->mtype = 2;
    // Spedisco il messaggio e rimango in attesa se la coda è piena
    if(msgsnd(msgid, message, sizeof(struct Message) - sizeof(message->mtype), 0) == -1) {
        syserr("nipote", "msgsnd");
        exit(1);
    }
}

void save_key(unsigned key, int my_string) {
    // Copio la chiave nello spazio di memoria condiviso
    unsigned* write = (unsigned*) output + (my_string); 
    *write = key;
}
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/msg.h>

#include <logger.h>
#include <constants.h>
#include <helpers.h>
#include <types.h>

struct Message* message;
int msgid;

int logger() {
    message = (struct Message*)malloc(sizeof(struct Message));
    
    // Creo la coda di messaggi con permessi di lettura e scrittura per tutti
	if((msgid = msgget(MSGKEY, (0666|IPC_CREAT|IPC_EXCL))) == -1) {
        syserr("logger", "Creazione della coda di messaggi fallita");
	}
    printf("Coda messaggi creata\n");

    // Ricevo i messaggi
    while(polling_receive() == 0) {
        sleep(1);
    }
    

    // Chiudo la coda
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        syserr("logger", "Eliminazione della coda dei messaggi fallita");
	}

	printf("Coda messaggi eliminata\n");
    return 1;
}

int polling_receive() {
    // ricevo i messaggi dalla coda msgid
    if(msgrcv(msgid, message, sizeof(struct Message) - sizeof(message->mtype), 0, IPC_NOWAIT) == -1) {
        return 0;
    }

    if(message->mtype == 1) {
        // Ricevo e stampare tutti i messaggi rimasti nella coda
        do {
            printf("message = %s\n", message->text);
        } while (msgrcv(msgid, message, sizeof(struct Message) - sizeof(message->mtype), 0, IPC_NOWAIT) != -1);
        
        return -1;
    }

    // stampo il contenuto del messaggio
    printf("message = %s\n", message->text);

    return 0;
}
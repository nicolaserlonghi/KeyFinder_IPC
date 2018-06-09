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

#include <logger.h>
#include <constants.h>
#include <helpers.h>
#include <types.h>


int logger() {
    int msgid;

    // Creo la coda di messaggi con permessi di lettura e scrittura per tutti
	if((msgid = msgget(MSGKEY, (0666|IPC_CREAT|IPC_EXCL))) == -1) {
        syserr("logger", "Creazione della coda di messaggi fallita");
	}
    char buffer[] = "Coda messaggi creata";
    printing(buffer);

    // Ricevo i messaggi
    while(polling_receive(msgid) == 0) {
        sleep(1);
    }
    

    // Chiudo la coda
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        syserr("logger", "Eliminazione della coda dei messaggi fallita");
	}

    char buf[] = "Coda messaggi eliminata";
    printing(buf);
	
    return 1;
}

int polling_receive(int msgid) {
    struct Message* message = (struct Message*)malloc(sizeof(struct Message));
    
    // ricevo i messaggi dalla coda msgid
    if(msgrcv(msgid, message, sizeof(struct Message) - sizeof(message->mtype), 0, IPC_NOWAIT) == -1) {
        return 0;
    }

    if(message->mtype == 1) {
        // Ricevo e stampare tutti i messaggi rimasti nella coda
        do {
            char* buffer = concat_string("Message = ", message->text);
            printing(buffer);
        } while (msgrcv(msgid, message, sizeof(struct Message) - sizeof(message->mtype), 0, IPC_NOWAIT) != -1);
        
        return -1;
    }

    // stampo il contenuto del messaggio
    char* buffer = concat_string("Message = ", message->text);
    printing(buffer);
    free(message);

    return 0;
}
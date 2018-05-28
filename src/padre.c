#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <padre.h>
#include <types.h>
#include <helpers.h>
#include <figlio.h>
#include <constants.h>
#include <logger.h>

int shmid_input;
int shmid_output;
int input_fd; // File descriptor

int padre(char* input_file, char* output_file) {
	
	if(access(input_file, F_OK | R_OK) != 0) {
		syserr("padre", "il file di input non esiste");
		return 1;
	}

	if(access(output_file, F_OK | R_OK) == 0) {
		// TODO: Decidere che fare
		//syserr("padre", "Il file di output esiste già");
	}
	
    if((input_fd = open(input_file, O_RDONLY, 0644)) == -1) {
        syserr("padre", "impossibile aprire il file");
    }
	// Conto il numero di righe del file di input
    int lines = 0;
    int n = 0;
    char buffer[SIZE_BUFFER];
    while((n = read(input_fd, buffer, SIZE_BUFFER)) > 0) {
        for (int i = 0; i < n; i++) {
            if (buffer[i] == '\n') {
                lines++;
            }
        }
    }
	// Riposiziono l'offset a zero
	lseek(input_fd, 0, SEEK_SET);

	// Creo il segmento di memoria condiviso per l'input
	void* s1 = attach_segments(SHMKEY_INPUT, sizeof(struct Status) + (lines * 1030) + 1, IPC_CREAT | 0666);

	struct Status* status = (struct Status*)s1;
	status->id_string = 0;
	status->granson = 0;
	char* file = (char*)(s1 + sizeof(struct Status));
	// leggo il file di input e lo carico nella memoria condivisa
	load_file(input_file, file);
	// Creo il segmento di memoria condiviso per l'output
	void* s2 = (void*)attach_segments(SHMKEY_OUTPUT, (lines + 1030), IPC_CREAT | 0666);

	// Creo il sottoprocesso logger
	pid_t pid_logger = fork();
	if(pid_logger == -1) {
		syserr("padre", "errore creazione logger");
	}

	if(pid_logger == 0) {
		// Questo è logger
		return logger();
	}

	// creo il sottoprocesso figlio
	pid_t pid_figlio = fork();
	if(pid_figlio == -1) {
		syserr("padre", "errore creazione figlio");
	}

	if(pid_figlio == 0) {
		// Questo è il figlio
		return figlio(lines);
	}
	// questo è il padre

	// Attendo la terminazione del figlio
	wait(&pid_figlio);
	// Attendo la terminazione del logger
	wait(&pid_logger);

	if(check_keys(s2, file, lines) == -1) {
		// TODO: decidere che fare
		printf("Le chiavi trovate non sono corrette");
		//syserr("padre", "Le chiavi trovate non sono corrette");
	}
	save_keys(output_file, s2, lines);

	// Elimino i segmenti di memoria condivisi
	detach_segments(s1, shmid_input);
	detach_segments(s2, shmid_output);


	return 0;
}


// Creo il segmento di memoria condiviso
void* attach_segments(key_t key, size_t size, int flags) {
	int shmid; // id shared memory
	/* Creo il segmento */
	if ((shmid = shmget (key , size, flags)) < 0) {
		perror (" shmget ");
	   	exit (1);
	}
	// indirizzo del segmento di memoria collegato al spazio di indirizzamento del processo
	void *shm;
	/* Attacco il segmento all' area dati del processo */
	if ((shm = shmat (shmid , NULL , 0)) == ( void *) -1) {
	  	perror (" shmat ");
	  	exit (1);
	}

	if(key == SHMKEY_INPUT) {
		shmid_input = shmid;
	} else {
		shmid_output = shmid;
	}
	printf("Memoria condivisa creata\n");
	return shm;
}

// Elimina il segmento di memoria condiviso
void detach_segments(char* shm, int shmid) {
	// scollego il segmento di memoria condiviso dal processo
	shmdt(shm);
  	// cancello il segmento di memoria condiviso
  	if(shmctl(shmid , IPC_RMID , NULL ) == -1) {
		syserr("padre", "errore eliminazione memoria condivisa");
	}
  	printf("Memoria condivisa eliminata\n");
}


void load_file(char* name, char* segment) {
	int n, offset = 0;
	char buf[SIZE_BUFFER]; // buffer di lettura

	// lettura del contenuto del file nel buffer
	while ((n = read(input_fd, buf, SIZE_BUFFER)) > 0) {
		for(int i = 0, j = 0; i < n; i++, j++) {
			
				int index = offset + j;
				
				// carico il contenuto del file nella memoria condivisa
				segment[index] = buf[i];
				if(buf[i] == '\n') {
					offset += 1030;
					j = -1;
				}
			
		}
	}
	segment[offset] = '\0';
	printf("File letto e caricato in memoria\n");

	// Chiudo il file
	if(close(input_fd) == -1) {
        syserr("padre", "errore nel chiudere il file");
    }
}

void save_keys(char* name, unsigned* keys, int lines) {
	int fd;
	// TODO: Open sovrascrivere da chidere
	if((fd = creat(name, O_RDWR ^ 0644)) == -1) {
        syserr("padre", "impossibile creare il file");
    }

	int i;
	for(i = 0; i < lines; i++) {
		char *hexa = unsigned_to_hexa(*(keys + i));
		char *buffer = concat_string("0x", hexa);
		buffer = concat_string(buffer, "\n");
		write(fd, buffer, string_length(buffer));
	}
}

int check_keys(unsigned* keys, char* input, int lines) {
	for(int i = 0; i < lines; i++) {
		char* line = (char*)(input + (i * 1030));
		unsigned key = keys[i];
		int fine_stringa = 0;
		char clear[512];
		char encrypt[512];
		int z = 0, j = 0;
		
		// Cerco la fine del testo in chiaro
		while(line[fine_stringa] != '>') {
			fine_stringa++;
		}
		// Copio la parola in chiaro
		for(z = 1, j = 0; z < fine_stringa; z++, j++) {
			clear[j] = line[z];
		}
		// Cerco la fine del testo criptato
		for(z = (fine_stringa+3), j = 0; line[z] != '>'; z++, j++) {
			encrypt[j] = line[z];
		}
		// Converto da char ad unsigned il testo in chiaro ed il testo cifrato
		unsigned* unsigned_clear = (unsigned*) clear;
		unsigned* unsigned_encrypt = (unsigned*) encrypt;
		// Cripto il testo in chiaro con la chiave trovata e controllo che sia corretto
		if((*unsigned_clear ^ key) != *unsigned_encrypt) {

			return -1;
		}

		return 0;
	}
}
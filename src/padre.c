#include <stdlib.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <types.h>
#include <helpers.h>
#include <figlio.h>
#include <constants.h>
#include <logger.h>
#include <padre.h>

int shmid_input;
int shmid_output;


int padre(char* input_file, char* output_file) {
	int input_fd; // File descriptor

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
    char buf[SIZE_BUFFER];
	int n_char = 0;
	int j = 0;
	int control = 1029;

    while((n = read(input_fd, buf, SIZE_BUFFER)) > 0) {
		int i = 0;
		for(i = 0; i < n; i++) {
			if(j != control) {
				if(buf[i] == '>' && n_char == 0) {
					n_char = j;
					control = (n_char*2)+3;
					lines++;
				}
				j++;
			} else {
				j = 0;
				n_char = 0;
				control = 1029;
			}
		}
    }

	// Riposiziono l'offset a zero
	lseek(input_fd, 0, SEEK_SET);

	// Creo il segmento di memoria condiviso per l'input
	void* s1 = attach_segments(SHMKEY_INPUT, sizeof(struct Status) + (sizeof(struct Line) * lines), IPC_CREAT | 0666);
	struct Status* status = (struct Status*)s1;
	status->id_string = 0;
	status->granson = 0;
	struct Line* file = (struct Line*)(s1 + sizeof(struct Status));
	// leggo il file di input e lo carico nella memoria condivisa
	load_file(input_file, file, input_fd);
	// Creo il segmento di memoria condiviso per l'output
	void* s2 = (void*)attach_segments(SHMKEY_OUTPUT, (lines * sizeof(unsigned)), IPC_CREAT | 0666);

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
		char buffer[] = "Le chiavi trovate non sono corrette";
		printing(buffer);
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
		syserr("padre", "shmget");
	   	exit (1);
	}
	// indirizzo del segmento di memoria collegato al spazio di indirizzamento del processo
	void *shm;
	/* Attacco il segmento all' area dati del processo */
	if ((shm = shmat (shmid , NULL , 0)) == ( void *) -1) {
		syserr("padre", "shmat");
	  	exit (1);
	}

	if(key == SHMKEY_INPUT) {
		shmid_input = shmid;
	} else {
		shmid_output = shmid;
	}

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
}


void load_file(char* name, struct Line* segment, int input_fd) {
	int n;
	char buf[SIZE_BUFFER]; // buffer di lettura
	int j = 0;
	int n_char = 0;
	char temp[1030] = "";
	int control = 1029;

	// lettura del contenuto del file nel buffer
	while ((n = read(input_fd, buf, SIZE_BUFFER)) > 0) {
		int i = 0;
		for(i = 0; i < n; i++) {
			if(j != control) {
				if(buf[i] == '>' && n_char == 0) {
					n_char = j;
					control = (n_char*2)+3;
				}
				temp[j] = buf[i];
				j++;
			} else {
				j = 0;
				int z = 0;
				int i = 0;
				char sup[4];
				
				for(i = 1; i < n_char; i++, z++) {
					sup[z] = temp[i];
					if(z == 3) {
						z = -1;
						unsigned* unsigned_clear = (unsigned*) sup;
						segment->clear[j] = *unsigned_clear;
						j++;
						// Pulisco l'array
						int i = 0;
						for(i = 0; i < 4; i++) {
							sup[i] = 0;
						}
					}
				}
				j = 0;
				z = 0;
				for(i = n_char+3; i < (n_char*2)+3; i++, z++) {
					sup[z] = temp[i];
					if(z == 3) {
						z = -1;
						unsigned* unsigned_encrypt = (unsigned*) sup;
						segment->encrypt[j] = *unsigned_encrypt;
						j++;
						// Pulisco l'array
						int i = 0;
						for(i = 0; i < 4; i++) {
							sup[i] = 0;
						}
					}
				}
				j = 0;
				n_char = 0;
				control = 1029;
				segment++;
				// Pulisco l'array
				for(i = 0; i < 1030; i++) {
					temp[i] = 0;
				}
			}
			
		}
	}

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
		int hexa_length = 8 - string_length(hexa);
		char *temp = "";
		int i;
		for(i = 0; i < hexa_length; i++) {
			temp = concat_string(temp, "0");
		}
		char* buffer = concat_string("0x", temp);
		buffer = concat_string(buffer, hexa);
		buffer = concat_string(buffer, "\n");
		write(fd, buffer, string_length(buffer));
	}
}

int check_keys(unsigned* keys, struct Line* input, int lines) {
	int i = 0;
	for(i = 0; i < lines; i++) {
		struct Line* line = (struct Line*)(input + (i * sizeof(struct Line)));
		unsigned key = keys[i];

		// Cripto il testo in chiaro con la chiave trovata e controllo che sia corretto
		int j = 0;
		for(j = 0; j < 128; j++) {
			if(line->clear[j] != 0 && line->encrypt[j] != 0) {
				if((line->clear[j] ^ key) != line->encrypt[j]) {
					return -1;
				}
			}
		}

		return 0;
	}
}
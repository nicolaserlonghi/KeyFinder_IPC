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
    char buf[SIZE_BUFFER];
	int cont = 0;
	int n_char = 0;
	int offset = 0;
	int fine_stringa = 0;

    while((n = read(input_fd, buf, SIZE_BUFFER)) > 0) {
        for (int i = offset; i < n; i++) {

			if(buf[i] == '<' && cont == 0) {
				// start
				cont = 1;
				i++;
				if(fine_stringa == 1) {
					offset = n - i - n_char;
					if(offset < 0) {
						offset = offset * -1;
						i = n;
					} else {
						offset = 0;
						i = n - (n - i - n_char);
					} 
				}
			}

			if(buf[i] == '>' && cont == 1 && i < n) {
				// fine
				if(fine_stringa == 0) {
					cont = 0;
					fine_stringa = 1;
				} else {
					n_char = 0;
					lines++;
					cont = 0;
					fine_stringa = 0;
				}
			}

			if(cont == 1 && i < n) {
				if(fine_stringa == 0) {
					n_char++;
				}
			}
		}
    }
	printf("lines = %d\n", lines);
	// Riposiziono l'offset a zero
	lseek(input_fd, 0, SEEK_SET);

	// Creo il segmento di memoria condiviso per l'input
	void* s1 = attach_segments(SHMKEY_INPUT, sizeof(struct Status) + (sizeof(struct Line) * lines) + 1, IPC_CREAT | 0666);

	struct Status* status = (struct Status*)s1;
	status->id_string = 0;
	status->granson = 0;
	struct Line* file = (struct Line*)(s1 + sizeof(struct Status));
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
	char buffer[] = "Memoria condivisa creata";
	printing(buffer);

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
	char buffer[] = "Memoria condivisa eliminata";
	printing(buffer);
}


void load_file(char* name, struct Line* segment) {
	int n;
	char buf[SIZE_BUFFER]; // buffer di lettura
	int fine_stringa = 0;
	char clear[512];
	char encrypt[512];
	int cont = 0;
	int j = 0;

	// lettura del contenuto del file nel buffer
	while ((n = read(input_fd, buf, SIZE_BUFFER)) > 0) {

		for(int i = 0; i < n; i++) {
			
			if(buf[i] == '<' && cont == 0) {
				// start
				cont = 1;
				i++;
				if(fine_stringa == 1) {
					j = 0;
				}
			}

			if(buf[i] == '>' && cont == 1 && i < n) {
				// fine
				if(fine_stringa == 0) {
					cont = 0;
					fine_stringa = 1;
				} else {
					j = 0;
				cont = 0;
				unsigned* unsigned_clear = (unsigned*) clear;
				unsigned* unsigned_encrypt = (unsigned*) encrypt;

				segment->clear = *unsigned_clear;
				segment->encrypt = *unsigned_encrypt;
	
				// Pulisco gli array
				for(int i = 0; i < 512; i++) {
				
					clear[i] = 0;
					encrypt[i] = 0;

				}
				segment+= sizeof(struct Line);
				fine_stringa = 0;
				}
			}

			if(cont == 1 && i < n) {
				if(fine_stringa == 0) {
					clear[j] = buf[i];
					j++;
				} else {
					encrypt[j] = buf[i];
					j++;
				}
			}
		}
		
	}
	char buffer[] = "File letto e caricato in memoria";
	printing(buffer);

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
		for(int i = 0; i < hexa_length; i++) {
			temp = concat_string(temp, "0");
		}
		char* buffer = concat_string("0x", temp);
		buffer = concat_string(buffer, hexa);
		buffer = concat_string(buffer, "\n");
		write(fd, buffer, string_length(buffer));
	}
}

int check_keys(unsigned* keys, struct Line* input, int lines) {
	for(int i = 0; i < lines; i++) {

		struct Line* line = (struct Line*)(input + (i * sizeof(struct Line)));
		unsigned key = keys[i];

		// Cripto il testo in chiaro con la chiave trovata e controllo che sia corretto
		if((line->clear ^ key) != line->encrypt) {

			return -1;
		}

		return 0;
	}
}
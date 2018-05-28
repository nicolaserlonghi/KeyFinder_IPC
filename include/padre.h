/// @file
/// @defgroup padre
/// @{
#ifndef PADRE_H_
#define PADRE_H_

/**
 * @brief wrapper del processo padre
 * @param input descrittore file input
 * @param output descrittore file output
 * @return 0 se non ci sono stati problemi, altrimenti 1
 */
int padre(char* input_file, char* output_file);

/**
 * @brief crea il segmento di memoria condivisa
 * @param key identificatore del segmento di memoria
 * @param size dimensione del segmento di memoria
 * @param flags flag
 */
void* attach_segments(key_t key, size_t size, int flags);

/**
 * @brief elimina il segmento di memoria condivisa
 * @param shm indirizzo del segmento di memoria
 * @param shmid id del segmento di memoria
 */
void detach_segments(char* shm, int shmid);

/**
 * @brief carica il file di input
 * @param name nome file
 * @param segment indirizzo del segmento di memoria
 */
void load_file(char* name, char* segment);

/**
 * @brief salva le chiavi sul file di output
 * @param name nome file
 * @param keys chiavi da salvare
 * @param lines numero di linee del file in input
 */
void save_keys(char* name, unsigned* keys, int lines);

/**
 * @brief controlla che le chiavi siano corrette per tutta la lunghezza delle stringhe
 * @param keys chiavi da controllare
 * @param file in input
 * @param lines numero delle linee del file di input
 */
int check_keys(unsigned* keys, char* input, int lines);

#endif
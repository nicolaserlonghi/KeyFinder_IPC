/// @file

/// @defgroup nipote
/// @{
#ifndef NIPOTE_H_
#define NIPOTE_H_

/**
 * @brief wrapper del processo nipote
 * @param id id del nipote
 * @param lines numero di linee del file in input
 * @return 0 se non ci sono stati problemi, altrimenti 1
 */
int nipote(int id, int lines);

#if THREAD == 1
void* nipote_thread(void* arg);
#endif

/**
 * @brief legge la stringa dal segmento S1
 * @param lines numero di linee del file in input
 * @param mid id del processo
 * @param status struttura di gestione
 * @param input puntatore alla memoria condivisa per l'input
 * @param output puntatore alla memoria condivisa per l'output
 * @return 0 se c'è una stringa da decodificare, altrimenti -1
 */
int load_string(int lines, int mid, struct Status* status, struct Line* input, void* output);

/*
 * @brief blocca l'accesso esclusivo regione critica
 * @param n_sem numero semaforo
 */
void lock(int n_sem);

/*
 * @brief sblocca l'accesso esclusivo regione critica
 * @param n_sem numero semaforo
 */
void unlock(int n_sem);

/*
 * @brief trova la chiave
 * @param line linea del file in input che deve essere elaborare
 * @param my_string ultima linea del file di input che è stata elaborata
 * @param output puntatore alla memoria condivisa per l'output
 */
void find_key(struct Line* line, int my_string, void* output);

/*
 * @brief deposita il messaggio "chiave trovata/secondi" nella coda di messagi del processo logger
 * @param time_spent tempo impiegato per la ricerca della chiave
 */
void send_timeelapsed(int time_spent);

/*
 * @brief salva la chiave nel segmento S2
 * @param key chiave da salvare
 * @param my_string linea del file di input a cui corrisponde la chiave
  * @param output puntatore alla memoria condivisa per l'output
 */
void save_key(unsigned key, int my_string, void* output);

#endif

/// @}
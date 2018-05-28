/// @file

/// @defgroup figlio
/// @{
#ifndef FIGLIO_H_
#define FIGLIO_H_

/**
 * @brief wrapper del processo padre
 * @param lines numero di linee del file in input
 * @return 0 se non ci sono stati problemi, altrimenti 1
 */
int figlio(int lines);

/**
 * @brief crea segmento di memoria condivisa
 */
void status_updated();


/**
 * @brief deposita il messaggio di terminazione nella coda di messaggi del processo logger
 */
void send_terminate();

#endif

/// @}
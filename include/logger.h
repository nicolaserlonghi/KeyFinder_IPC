/// @file

/// @defgroup logger
/// @{
#ifndef LOGGER_H_
#define LOGGER_H_

/**
 * @brief wrapper del processo logger
 * @return 0 se non ci sono stati problemi, altrimenti 1
 */
int logger();

/**
 * @brief scarica la coda di messaggi e la stampa su stdout
 * @return 0 se il messaggio ricevuto non ha mtype != 1, altimenti return -1
 */
int polling_receive(int msgid);

#endif

/// @}
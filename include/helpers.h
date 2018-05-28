/// @file

/// @defgroup tools
/// @{

#ifndef HELPERS_H_
#define HELPERS_H_

/**
 * @brief calcola la lunghezza di un array di char
 * param array di char di cui si vuole sapere la lunghezza
 * return lunghezza dell'array
 */
int string_length(char* string);

/**
 * @brief converte un unsigne in un array di char
 * param numero numero da convertire
 * return numero convertito
 */
char* unsigned_to_string(unsigned numero);

/**
 * @brief converte un intero in array di char
 * param numero numero da convertire
 * return numero convertito
 */
char* int_to_string(int numero);

/**
 * @brief converte un unsigned in esadecimale
 * param numero numero da convertire in esadecimale
 * return numero convertito
 */
char* unsigned_to_hexa(unsigned numero);

/**
 * @brief converte un array di char in unsigned
 * param stringa array di char da convertire
 * return array di char convertito
 */
unsigned string_to_unsigned(char *stringa);

/**
 * @brief copia un array di char in un altro array di char
 * param stringa1 array di char da cui copiare
 * param stringa2 array di char in cui copiare
 */
void copy_string(char *stringa1, char* stringa2);

/**
 * @brief concatena due array di char
 * param stringa1 primo array di char da concatenare
 * param stringa2 secondo array di char da concatenare
 */
char* concat_string(char* stringa1, char* stringa2);

/**
 * @brief stampa a video l'array di char
 * param stringa array di char da stampare
 */
void printing(char *stringa);

/**
 * @brief funzione di errore alternativa
 * param prog parte del programma in cui è avvenuto l'errore
 * param msg messaggio d'errore
 */
void syserr (char *prog, char *msg);

#endif

/// @}
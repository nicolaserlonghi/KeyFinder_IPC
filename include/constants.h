/// @file

/// @defgroup costanti
/// @{

/**
 * @brief id della sezione di memoria condivisa s1 (input)
 */
#define SHMKEY_INPUT 1234

/**
 * @brief id della sezione di memoria condivisa s2 (output)
 */
#define SHMKEY_OUTPUT 12345

/**
 * @brief dimensione buffer di lettura/scrittura
 */
#define SIZE_BUFFER 512

/**
 * @brief id del semaforo
 */
#define SEM_KEY 123456

/**
 * @brief id code di messaggi
 */
#define MSGKEY 90

/**
 * @brief numero di thread generati
 */
#define NUM_THREADS 4

#ifndef THREAD

#define THREAD
#endif

/// @}
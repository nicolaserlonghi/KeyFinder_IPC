/// @file

/// @defgroup types
/// @{
#ifndef TYPES
#define TYPES

/**
 * @brief struttura di controllo dei nipoti
 */

struct Status {
	int granson;
	int id_string;
};

/**
 * @brief struttura messaggio
 */
struct Message {
	long mtype;
	char text[128];
};

/**
 * @brief struttura linea file di input
 */
struct Line {
	unsigned clear;
	unsigned encrypt;
};

#endif

/// @}
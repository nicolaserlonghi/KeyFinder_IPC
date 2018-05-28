#ifndef HELPERS_H_
#define HELPERS_H_

int string_length(char* string);
char* unsigned_to_string(unsigned numero);
char* int_to_string(int numero);
char* unsigned_to_hexa(unsigned numero);
unsigned string_to_unsigned(char *stringa);
void copy_string(char *stringa1, char* stringa2);
char* concat_string(char* stringa1, char* stringa2);
void printing(char *stringa);
void syserr (char *prog, char *msg);

#endif
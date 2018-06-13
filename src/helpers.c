#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

#include <helpers.h>

int string_length(char* string){
    int i;

    for(i = 0; *string != '\0'; i++, string++);

    return i;
}

char* unsigned_to_string(unsigned numero){ 
    unsigned val = numero;
    int cifre = 0;

    while(val != 0){
        val = val / 10;
        cifre++;
    }

    char *new_string = (char*) malloc(sizeof(char) * cifre + 1);

    int i = 0;
    for(i = cifre - 1; i >= 0; i-- ) {
        new_string[i] = (char) (numero % 10) + '0';
        numero = numero/10;
    }
    new_string[i] = '\0';

    return new_string; 
}

char* int_to_string(int numero){ 
    int val = numero;
    int cifre = 0;

    do {
        val = val / 10;
        cifre++;
    } while(val != 0);

    char *new_string = (char*) malloc(sizeof(char) * cifre + 1);

    int i = 0;
    for(i = cifre - 1; i >= 0; i-- ){
        new_string[i] = (char) (numero % 10) + '0';
        numero = numero/10;
    }
    
    new_string[cifre] = '\0';

    return new_string; 
}

char* unsigned_to_hexa(unsigned numero){ 
    unsigned val = numero;
    int cifre = 0;

    while(val != 0){
        val = val / 16;
        cifre++;
    }

    char *new_string = (char*) malloc(sizeof(char) * 9);

    int i = 0;
    for(i = cifre - 1; i >= 0; i-- ){
        if((numero % 16) <= 9){
            new_string[i] = (char) (numero % 16) + '0';
        }
        else{
            new_string[i] = (char) ((numero % 16) - 10) + 'A';
        }
        numero = numero/16;
    }
    new_string[i] = '\0';

    return new_string; 
}

unsigned string_to_unsigned(char *stringa){
    unsigned numero = 0;
    
    int i;
    for(i = 0; stringa[i] != '\0' ; i++){
        numero = (numero * 10) + (stringa[i] - '0');
    }

    return numero;
}

void copy_string(char *stringa1, char* stringa2){

    int i;
    for(i = 0; *stringa2 != '\0'; stringa1++, stringa2++, i++){
        *stringa1 = *stringa2;
    }
    i++;
    *stringa1++ = '\0';
}

char* concat_string(char* stringa1, char* stringa2){
    int size1 = string_length(stringa1);
    int size2 = string_length(stringa2);
    char* new_string = (char*) malloc(sizeof(char) * (size1 + size2 + 1));

    int i;
    for(i = 0; *stringa1 != '\0'; i++, stringa1++, new_string++){
        *new_string = *stringa1;
    }
    for( ; *stringa2 != '\0'; i++, stringa2++, new_string++){
        *new_string = *stringa2;
    }
    i++;
    *new_string++ = '\0';
    new_string = new_string - i;

    return new_string;
}

void printing(char *stringa){

    int i = 0;
    for( i = 0; stringa[i] != '\0'; i++);
    stringa[i] = '\n';
    i++;
    write(1, stringa, i);
}

void syserr (char *prog, char *msg) {

    char* temp = concat_string(prog, ", ");
    temp = concat_string(temp, msg);
    printing(temp);
    perror ("system error");
    exit (1);
}
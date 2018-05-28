#ifndef PADRE_H_
#define PADRE_H_

// Dichiaro le funzioni

int padre(char* input_file, char* output_file);
void* attach_segments(key_t key, size_t size, int flags);
void detach_segments(char* shm, int shmid);
void load_file(char* name, char* segment);
void save_keys(char* name, unsigned* keys, int lines);
int check_keys(unsigned* keys, char* input, int lines);

#endif
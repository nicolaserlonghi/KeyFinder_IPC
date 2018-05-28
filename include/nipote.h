#ifndef NIPOTE_H_
#define NIPOTE_H_

// Dichiaro le funzioni

int nipote(int id, int lines);
int load_string(int lines);
void lock();
void unlock();
void find_key(char* line, int my_string);
void send_timeelapsed(int time_spent);
void save_key(unsigned key, int my_string);

#endif
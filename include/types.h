// Sruttura di controllo dei nipoti
#ifndef TYPES
#define TYPES

struct Status {
	int granson;
	int id_string;
};

struct Message {
	long mtype;
	char text[128];
};

#endif
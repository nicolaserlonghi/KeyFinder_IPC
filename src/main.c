#include <stdlib.h>

#include <padre.h>
#include <helpers.h>

int main(int argc, char *argv[]) {
	if (argc != 3) {
		syserr("main", "key_finder <input> <output>");
        return 1;
    }

	exit(padre(argv[1], argv[2]));
	return 0;
}
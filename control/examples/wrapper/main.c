#include "b15f/wrapper/b15f.h"
#include <stdio.h>

int main() {
	char buf[512];
	b15f_t* instance = get_instance(buf, sizeof(buf));

	if (instance == NULL) {
		fprintf(stderr, "%s", buf);
		return -1;
	}

	return 0;
}
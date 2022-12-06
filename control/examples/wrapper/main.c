#include "b15f/wrapper/b15f.h"
#include <stdio.h>

int main() {
	char buf[512];
	b15f_t* instance = get_instance(buf, sizeof(buf));

	if (instance == NULL) {
		fprintf(stderr, "%s", buf);
		return -1;
	}

	uint8_t dip = read_dip_switch(instance, NULL, 0);

	printf("DIP switch reads: ");
	for(uint8_t mask = 1; mask != 0; mask <<= 1) {
		printf("%d", (dip & mask) == mask ? 1 : 0);
	}
	printf("\n");

	return 0;
}
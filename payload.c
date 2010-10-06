#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	int badcode = 1;
	int tramp(int i) {
		return badcode + i;
	}


	int (*t)(int) = (void *)((char*)&tramp + 0);
	int j = t(10);



		printf("Hello world from untrusted binary %i!\n", j);
	return(0);
}

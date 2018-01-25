#include <stdio.h>
#include <stdlib.h>

int main() {
	printf("Hello from %d\n", getpid());
	if(fork() == -1) {
		perror("fork\n");
	}
	printf("Goodbye from %d\n", getpid());
	return 0;
}

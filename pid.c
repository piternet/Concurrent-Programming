#include <stdio.h>
#include <stdlib.h>

int main() {

	pid_t my_pid = getpid();
	printf("My PID is %d\n", my_pid);
	return 0;
}

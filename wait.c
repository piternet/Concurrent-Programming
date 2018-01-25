#include <stdio.h>
#include <stdlib.h>

int main() {
	pid_t pid;
	switch(pid = fork()) {
		case -1:
			perror("fork\n");
		case 0:
			printf("I am a child, my pid is %d, fork() return val is %d\n", getpid(), pid);
			return 1;
		default:
			printf("I am a parent, my pid is %d, fork() return val is %d\n", getpid(), pid);
			int child_return, child_pid;
			child_pid = wait(&child_return);
			if(child_pid == -1)
				perror("wait\n");
			printf("child pid and return: %d, %d\n", child_pid, child_return);
			return 0;
	}
	return 0;
}

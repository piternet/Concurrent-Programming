#include <stdio.h>
#include <stdlib.h>
#include <unisth.h>
#include <time.h>
#include <semaphore.h>

#define N 3
#define K 2

sem_t *sem;
int compteur = 0;
int barrier_counter = 0;

void child(pid_t pid_barrier, int write_dsc) {
	printf("Jestem dziecko o pid %d.\n", pid_child);
	// symuluje prace
	unsigned time_to_sleep = rand() % 10 + 1;
	sleep(time_to_sleep);
	barrier_counter++;
	sem_wait(sem);
	int sig = rand() % (62+1-35) + 35;
	printf("Wylosowalem sygnal nr %d\n" sig);
	if(kill(pid_barrier, sig) == -1)
		perror("kill");
}

void sig_handler(int signo) {
    printf("otrzymalem sygnal nr %d\n", signo);
}  

int main() {
	srand(time(NULL));
	pid_t pid_child, pid_barrier;
	pid_barrier = getpid();
	int i, barrier_counter = 0;
	sem_init(sem, 1, 0);
	sem = sem_open("/sem", O_CREAT, 0644, compteur);
	for(i=0; i<N; i++) {
		switch(pid_child = fork()) {
			case -1:
				perror("fork\n");
			case 0:
				child(pid_barrier, pipe_dsc[1]);
			default:
				while(barrier_counter < K)
					sleep(1);
				int j;
				for(j=0; j<N; j++)
					sem_post(sem);
				printf("Otrzymuje sygnaly:\n");
				if (signal(SIGINT, sig_handler) == SIG_ERR)
					perror("signal\n");
		}
	}
	return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <stdbool.h>
#include <sys/syscall.h>
#include <stdatomic.h>

#define LOWER_LIMIT 0 //Empieza el 1/4 de circulito
#define UPPER_LIMIT 1 //Termina el 1/4 de circulito
#define NUMBER_OF_TRAPEZES 99999999 //Cuantos trapecios tiene el circulito (osease n)
#define NUMBER_OF_PROCESSES 9
#define MEMBAR __sync_synchronize()

sem_t *sem;

float x = 0;
double *sum_x;
double func_eval = 0;
float delta_x;


void pi_calculation_inner_trapezes(long double, long double);

int main(int argc, char **argv)
{
	sem = sem_open("mysemaphore", O_CREAT | O_EXCL, 0666, 1);

	int p;
	pid_t pid = getpid();
	printf("Hola soy el padre\n");
    	printf("Este es mi pid %d\n", pid);

	long long start_ts;
	long long stop_ts;
	long long elapsed_time;
	struct timeval ts;

	key_t shmkey = ftok(".", 'P');
	int shmid = shmget(shmkey, sizeof(double), IPC_CREAT | 0666);

	sum_x = (double *)shmat(shmid, NULL, 0);
	*sum_x = 0.0;

	for (int i=0; i<NUMBER_OF_PROCESSES; i++)
	{
		if(pid == getpid())
        	{
            		p = fork();
        	}
	}

	gettimeofday(&ts, NULL);
    	start_ts = ts.tv_sec;

    	delta_x = (UPPER_LIMIT - LOWER_LIMIT) / (float)NUMBER_OF_TRAPEZES;


    	if(pid != getpid())
    	{
		MEMBAR;
		sem_wait(sem);
		
        	printf("Hola, soy el hijo %d\n", getpid());
        	printf("Voy a entrar a la seccion critica\n");
		MEMBAR;
        	pi_calculation_inner_trapezes((getpid()-pid-1)*(NUMBER_OF_TRAPEZES/NUMBER_OF_PROCESSES)+1, (getpid()-pid)*(NUMBER_OF_TRAPEZES/NUMBER_OF_PROCESSES)); //Calculo del pi con los trapecios internos, sin contar el trapecio 0 y el trapecio n
        	MEMBAR;
        	sem_post(sem);
		MEMBAR;
		usleep(500);

        	exit(1);
    	}else
    	{
        	while(wait(NULL)>0);
		MEMBAR;
		
    		x = (float)LOWER_LIMIT + (0.0 * delta_x);
    		func_eval = sqrt(1 - (x*x));
    		*sum_x += func_eval;

    	
    		x = (float)LOWER_LIMIT + ((double)NUMBER_OF_TRAPEZES * delta_x);
    		func_eval = sqrt(1 - (x*x));
    		*sum_x += func_eval;

    		*sum_x *= delta_x;
    		*sum_x /= 2.0;

    		printf("Valor de PI: %.10lf\n", *sum_x * 4.0);

    		gettimeofday(&ts, NULL);
    		stop_ts = ts.tv_sec;
    		elapsed_time = stop_ts - start_ts;

    		printf("------------------------------\n");
    		printf("TIEMPO TOTAL, %lld segundos\n",elapsed_time);


    	}

    	shmdt(sum_x);
    	shmctl(shmid, IPC_RMID, NULL);
    	sem_close(sem);
    	sem_unlink("mysemaphore");
	return 0;
}

void pi_calculation_inner_trapezes(long double start, long double end)
{
	long double i;
	printf("inicio %Lf\n", start);
	printf("fin %Lf\n", end);
	
    	for(i = start; i <= end;  i++)
    	{
        
        	if(i == 99999999)
        	{
            		break;
        	}

        	x = (float)LOWER_LIMIT + (i * delta_x);
        	func_eval = (sqrt(1 - (x*x))*2);
        	*sum_x += func_eval;
    	}
}

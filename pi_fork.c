/*
Nelly Garcia Sanchez
Juan Pablo Lopez Zuñiga
Equipo: Quokka
*/

//Decalracion de librarias a utilizar
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

sem_t *sem;      //Declaracion del semaforo

//Variables del circulo
float x = 0;
double *sum_x;
double func_eval = 0;
float delta_x;


void pi_calculation_inner_trapezes(long double, long double);

int main(int argc, char **argv)
{
	sem = sem_open("mysemaphore", O_CREAT | O_EXCL, 0666, 1); //se inicia un semaforo

	//Variables de procesos
	int p;
	pid_t pid = getpid();
	printf("Hola soy el padre\n");
    printf("Este es mi pid %d\n", pid);

	//Variables del Tiempo
	long long start_ts;
	long long stop_ts;
	long long elapsed_time;
	struct timeval ts;

	key_t shmkey = ftok(".", 'P'); //Declaracion de clave de comunicación
	int shmid = shmget(shmkey, sizeof(double), IPC_CREAT | 0666);  //Declaracion de una variable para la memoria compartida.

	sum_x = (double *)shmat(shmid, NULL, 0);    //Variable que suma los valores de pi
	*sum_x = 0.0;

	for (int i=0; i<NUMBER_OF_PROCESSES; i++)    // se inica el ciclo para obtener pi tomando en cuenta los procesos establecidos
	{
		if(pid == getpid())
        {
            p = fork();
        }

    }

	gettimeofday(&ts, NULL);//Toma el tiempo inicial de la computadora
    start_ts = ts.tv_sec; // Tiempo inicial

    delta_x = (UPPER_LIMIT - LOWER_LIMIT) / (float)NUMBER_OF_TRAPEZES; //Base de los trapecios (delta>


    if(pid != getpid())  //Zona crita, inicia el calculo de pi con los trapecios internos , sin contar el traoecio 0 y el trapecio n
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
    	//f(x0) Calculo del primer trapecio
    	x = (float)LOWER_LIMIT + (0.0 * delta_x); //x0 = a+0*delta x
    	func_eval = sqrt(1 - (x*x)); //f(x0)
    	*sum_x += func_eval;

    	//f(xn) Calculo del ultimo trapecio
    	x = (float)LOWER_LIMIT + ((double)NUMBER_OF_TRAPEZES * delta_x); //xn = a+n*delta x
    	func_eval = sqrt(1 - (x*x)); //f(xn)
    	*sum_x += func_eval;

    	*sum_x *= delta_x; //delta x * sumatoria de todas las areas
    	*sum_x /= 2.0; //sumatoria de todas las areas entre 2

    	printf("Valor de PI: %.10lf\n", *sum_x * 4.0);

    	gettimeofday(&ts, NULL); //Toma el tiempo final de la computadpra
    	stop_ts = ts.tv_sec; // Tiempo final
    	elapsed_time = stop_ts - start_ts; //Calculo del tiempo de ejecucion


    	printf("------------------------------\n");
    	printf("TIEMPO TOTAL, %lld segundos\n",elapsed_time);


    }

    shmdt(sum_x); //Desprendimiento de variable de memoria compartida
    shmctl(shmid, IPC_RMID, NULL); //Operacion de control para la variable de memoria compartida
    sem_close(sem);
    sem_unlink("mysemaphore");
	return 0;
}

void pi_calculation_inner_trapezes(long double start, long double end)
{
	long double i;
	printf("inicio %Lf\n", start);
	printf("fin %Lf\n", end);
	// 2*f(n-1) Operaciones para calcular los trapecios de en medio con threads
    for(i = start; i <= end;  i++)
    {
        //Condicion para no calcular el ultimo trapecio
        if(i == 99999999)
        {
            break;
        }

        x = (float)LOWER_LIMIT + (i * delta_x); //xi = a+i*delta x
        func_eval = (sqrt(1 - (x*x))*2); //2*f(xn-1)
        *sum_x += func_eval;
        //printf("Iteracion %Lf\n", i);
    }
}

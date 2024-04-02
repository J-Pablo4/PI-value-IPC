$(CC)= gcc

final: pi_fork.o

	$(CC) pi_fork.c -o final -lm -pthread

	./final

pi_fork.o: pi_fork.c

	$(CC) -c pi_fork.c

clean:

	rm *.o final

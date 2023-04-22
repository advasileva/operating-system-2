#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <semaphore.h>

void seller(sem_t *sem) {
    printf("Seller PID: %d\n", getpid());

    while (1) {
        sem_wait(sem);

        printf("Selling stock\n");
        sleep(1);

        sem_post(sem);
    }
}

void buyer(int *list[], int size, sem_t *sem_first, sem_t *sem_second) {
    printf("Buyer PID: %d\n", getpid());

    for (size_t i = 0; i < size; i++)
    {
        if (i % 2 == 0) {
            sem_wait(sem_first);

            printf("Buying stock\n");
            sleep(1);

            sem_post(sem_first);
        } else {
            sem_wait(sem_second);

            printf("Buying stock\n");
            sleep(1);

            sem_post(sem_second);
        }
    }
}

int main(int argc, char **argv) {
    sem_t *sem_first, *sem_second;

    sem_first = sem_open("first", 0);
    sem_second = sem_open("second", 0);
    
    int id = fork();
    if (id == 0) {
        seller(sem_first);
    } else {
        id = fork();
        if (id == 0) {
            seller(sem_second);
        }
        else {
            id = fork();
            if (id == 0) {
                buyer(0, 5, sem_first, sem_second);
            }
        }
    }

    sem_close(sem_first);
    sem_close(sem_second);

    return 0;
}

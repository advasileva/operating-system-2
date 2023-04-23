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
#include <sys/mman.h>
#include <string.h>

void seller(sem_t *sem, char *addr, char *name) {
    printf("Seller PID: %d\n", getpid());

    while (addr[0] == 1) {
        sem_wait(sem);

        printf("Selling stock by %s\n", name);
        sleep(1);

        sem_post(sem);
    }
}

void buyer(int *list[], int size, sem_t *sem_first, sem_t *sem_second, char *addr) {
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
    char memn[] = "shared-memory";
    int shm, size = 10;
    char *addr;
    sem_t *sem_first, *sem_second;

    shm = shm_open(memn, O_RDWR | O_CREAT, 0777);
    ftruncate(shm, size);
    addr = mmap(0, size, PROT_WRITE|PROT_READ, MAP_SHARED, shm, 0);
    addr[0] = 1;

    sem_first = sem_open("first", 0);
    sem_second = sem_open("second", 0);
    
    int id = fork();
    if (id == 0) {
        seller(sem_first, addr, "first");
    } else {
        id = fork();
        if (id == 0) {
            seller(sem_second, addr, "second");
        }
        else {
            id = fork();
            if (id == 0) {
                buyer(0, 5, sem_first, sem_second, addr);
            }
        }
    }

    addr[0] = 0;

    shm_unlink(memn);

    sem_close(sem_first);
    sem_close(sem_second);

    return 0;
}

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

void seller(sem_t *sem, char *addr, int id) {
    printf("Seller PID: %d\n", getpid());

    while (addr[0] == 1
    ) {
        sem_wait(sem);

        if (addr[id] != 0)
        {
            printf("Selling stock by %d\n", id);
            addr[id] = 0;
        }
        sleep(1);

        sem_post(sem);
    }
    printf("Seller %d finished\n", id);
}

void buyer(int *list[], int size, sem_t *sem_first, sem_t *sem_second, char *addr) {
    printf("Buyer PID: %d\n", getpid());

    for (size_t i = 0; i < size; i++)
    {   
        // addr[0] = 1;
        if (i % 2 == 0) {
            sem_wait(sem_first);

            if (addr[1] == 0) {
                printf("Buying stock from 1\n");
                addr[1] = 1;
            } else {
                i--;
            }
            
            sleep(1);

            sem_post(sem_first);
        } else {
            sem_wait(sem_second);

            if (addr[2] == 0) {
                printf("Buying stock from 2\n");
                addr[2] = 1;
            } else {
                i--;
            }

            sleep(1);

            sem_post(sem_second);
        }
    }
    addr[0] = 0;
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
    addr[1] = 0;
    addr[2] = 0;

    sem_first = sem_open("first", 0);
    sem_second = sem_open("second", 0);
    
    int id = fork();
    if (id == 0) {
        seller(sem_first, addr, 1);
    } else {
        id = fork();
        if (id == 0) {
            seller(sem_second, addr, 2);
        }
        else {
            id = fork();
            if (id == 0) {
                buyer(0, 5, sem_first, sem_second, addr);
            }
        }
    }

    shm_unlink(memn);

    sem_close(sem_first);
    sem_close(sem_second);

    return 0;
}

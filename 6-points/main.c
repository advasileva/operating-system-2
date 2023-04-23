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

// DTO покупателя
typedef struct person {
    int list[30];
    int size;
 } person;

// Логика работы продавца
void seller(int sem, char *addr, int id) {
    struct sembuf second_buf = {.sem_num = 0, .sem_op = 1, .sem_flg = 0};
    struct sembuf first_buf = {.sem_num = 0, .sem_op = -1, .sem_flg = 0};
    printf("Seller %d PID: %d\n", id, getpid());

    while (addr[0] == 1) { // Пока не закончили работу
        semop(sem, &first_buf, 1);

        if (addr[id] != 0) // Если в нашей очереди есть покупатель, то обслуживаем его
        {
            printf("Selling stock %d by %d\n", addr[id], id);
            addr[id] = 0;
        }
        sleep(1);

        semop(sem, &second_buf, 1);
    }
    printf("Seller %d finished\n", id);
}

// Логика работы покупателя
void buyer(int *list, int size, int sem_first, int sem_second, char *addr) {
    struct sembuf second_buf = {.sem_num = 0, .sem_op = 1, .sem_flg = 0};
    struct sembuf first_buf = {.sem_num = 0, .sem_op = -1, .sem_flg = 0};
    printf("Buyer PID: %d\n", getpid());
    addr[3]++;

    for (size_t i = 0; i < size; i++) { // Последовательно покупаем все товары из списка
        if (list[i] % 2 == 1) { // Проверяем, в какой отдел нам надо идти
            semop(sem_first, &first_buf, 1);

            if (addr[1] == 0) { // Если продавец свободен, то покупаем
                printf("Buying stock %d from 1\n", list[i]);
                addr[1] = list[i];
            } else {
                i--;
            }
            
            sleep(1);

            semop(sem_first, &second_buf, 1);
        } else {
            semop(sem_second, &first_buf, 1);

            if (addr[2] == 0) {
                printf("Buying stock %d from 2\n", list[i]);
                addr[2] = list[i];
            } else {
                i--;
            }

            sleep(1);

            semop(sem_second, &second_buf, 1);
        }
    }

    addr[3]--; // Текущий покупатель купил все товары из списка
    if (addr[3] == 0) // Проверяем, все ли покупатели всё купили
    {
        addr[0] = 0; // Завершаем работу
    }
    
}

// Рекурсивный форк процессов-покупателей
int fork_buyers(person buyers[], int n, sem_t *sem_first, sem_t *sem_second, char *addr) { 
    if (n == 0) {
        return 0;
    }
    if (fork() == 0) {
        buyer(buyers[n - 1].list, buyers[n - 1].size, sem_first, sem_second, addr);
        return 0;
    }
    return fork_buyers(buyers, n - 1, sem_first, sem_second, addr);
}

int main(int argc, char **argv) {
    char memn[] = "shared-memory";
    int shm, shm_size = 10, n = 0, k = 0, t = 0;
    char *addr;
    int sem_first, sem_second;
    person buyers[20];

    // Ввод информации о покупателях
    int input = open(argv[1], O_RDONLY);
    read(input, &n, 1);
    n -= '0';
    for (size_t i = 0; i < n; i++) {
        read(input, &k, 1);
        read(input, &k, 1);
        k -= '0';
        buyers[i].size = k; 
        for (size_t j = 0; j < k; j++) {
            read(input, &t, 1);
            read(input, &t, 1);
            t -= '0';
            buyers[i].list[j] = t;
        }
    }

    // Работа с разделяемой памятью
    shm = shm_open(memn, O_RDWR | O_CREAT, 0777);
    ftruncate(shm, shm_size);
    addr = mmap(0, shm_size, PROT_WRITE|PROT_READ, MAP_SHARED, shm, 0);
    addr[0] = 1;
    addr[1] = 0;
    addr[2] = 0;
    addr[3] = 0;

    // Создание семафоров в стандарте UNIX SYSTEM V
    key_t key_first = ftok("first", 0);
    key_t key_second = ftok("second", 0);

    sem_first = semget(key_first, 1, 0666 | IPC_CREAT);
    sem_second = semget(key_second, 1, 0666 | IPC_CREAT);
    
    int id = fork();
    if (id == 0) {
        seller(sem_first, addr, 1);
    } else {
        id = fork();
        if (id == 0) {
            seller(sem_second, addr, 2);
        }
        else {
            fork_buyers(buyers, n, sem_first, sem_second, addr);
        }
    }

    // Освобождение памяти
    shm_unlink(memn);

    // Удаление семафоров в стандарте UNIX SYSTEM V
    semctl(sem_first, 0, IPC_RMID, 0);
    semctl(sem_second, 0, IPC_RMID, 0);

    return 0;
}

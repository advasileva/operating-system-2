#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

int input_size = 5000;
int output_size = 3;

void first_read(int fd[2], char* file) {
    printf("Reader PID: %d\n", getpid());

    char buffer[input_size];
    int input = open(file, O_RDONLY);
    int num = read(input, buffer, input_size);

    char size[10];
    sprintf(size, "%d", num);
    write(fd[1], size, 10);
    write(fd[1], buffer, num);
}

void second_process(int fd1[2], int fd2[2]) {
    printf("Processor PID: %d\n", getpid());

    char size[10];
    read(fd1[0], size, 10);
    int num;
    sscanf(size, "%d", &num);

    char buffer[num];
    read(fd1[0], buffer, num);

    char min = buffer[0], max = buffer[0];
    for (size_t i = 1; i < num; i++)
    {
        if (min > buffer[i]) {
            min = buffer[i];
        }
        if (max < buffer[i]) {
            max = buffer[i];
        }
    }

    buffer[0] = min;
    buffer[1] = '\n';
    buffer[2] = max;
    write(fd2[1], buffer, output_size);
}

void third_write(int fd[2], char* file) {
    printf("Writer PID: %d\n", getpid());

    char buffer[output_size];
    read(fd[0], buffer, output_size);

    int output = open(file, O_WRONLY | O_CREAT);
    write(output, buffer, output_size);
}

int main(int argc, char **argv) {
    int input[2], output[2];
    pipe(input);
    pipe(output);
    
    int id = fork();
    if (id == 0) {
        first_read(input, argv[1]);
    } else {
        id = fork();
        if (id == 0) {
            second_process(input, output);
        }
        else {
            id = fork();
            if (id == 0) {
                third_write(output, argv[2]);
            }
        }
    }
    return 0;
}

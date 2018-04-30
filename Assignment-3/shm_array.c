#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/sem.h>
#include <unistd.h>

#define size 1024

key_t key = (key_t)100;
int shmid;
char (*Array[10]);

int main() {
    if ((shmid = shmget(key, sizeof(char[size][10]), IPC_CREAT | 0666)) == -1)
    {
        perror("shmget");
        printf("shmget error 1\n");
        exit(0);
    }
    printf("hey1\n");
    // *Array = shmat(shmid, 0, 0);
    // printf("hey2\n");

    if ((*Array = shmat(shmid, 0, 0)) == (void *)-1)
    {
        printf("Shmat error 1\n");
        exit(0);
    }
    printf("hey2\n");
    int j;
    for ( j=0; j<10; j++ )
    {
        strcpy((Array + j), itoa(j));
        // printf("hey3\n");
        printf("%s\n", (Array + j));
    }
}
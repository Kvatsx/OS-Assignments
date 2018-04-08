#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
// #include <sys/pthread.h>

#include "shared_common.h"

int main()
{

    int running = 1;
    void *shared_memory = (void *)0;
    struct shared_use_st *shared_stuff;
    char buffer[TEXT_SZ ];
    int shmid;
    int mykey = getuid();


    shmid = shmget((key_t)mykey, sizeof(struct shared_use_st), 0666 | IPC_CREAT);

    if (shmid == -1) {
        fprintf(stderr, "shmget failed\n");
        exit(EXIT_FAILURE);
    }

    shared_memory = shmat(shmid, (void *)0, 0);
    if (shared_memory == (void *)-1) {
        fprintf(stderr, "shmat failed\n");
        exit(EXIT_FAILURE);
    }

    printf("Memory attached at %X\n", (int)shared_memory);

    shared_stuff = (struct shared_use_st *)shared_memory;
    if ( fork() )
    {
        while(running) {
            // printf("shared_stuff->written_by_you %d\n", shared_stuff->written_by_you);
            while(shared_stuff->written_by_you == 1) {
                sleep(1);            
                printf("waiting for client...\n");
            }
            printf("shared_stuff->written_by_you %d\n", shared_stuff->written_by_you);
            printf("Enter some text: ");
            fgets(buffer, BUFSIZ, stdin);
            
            strncpy(shared_stuff->some_text, buffer, TEXT_SZ);
            shared_stuff->written_by_you = 1;

            if (strncmp(buffer, "end", 3) == 0) {
                    running = 0;
            }
        }
    }
    else
    {

    }

    if (shmdt(shared_memory) == -1) {
        fprintf(stderr, "shmdt failed\n");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

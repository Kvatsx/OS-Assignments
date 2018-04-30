// Kaustav Vats(2016048)
// Saksham Vohra

#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <pthread.h>
#include "shared.h"

// key_t key;
int shmid;

char *shmaddr;
char *shmaddr2;
char *shmbuf;
char *shmbuf2;

int semflag;
int semid;
int semid2;
short sema[1];

int i;
struct sembuf sembuff[1];
struct sembuf sembuff2[1];

void *Sender(void * null)
{
    struct shared *shared_data;
    if ((shmaddr = shmat(shmid, NULL, 0)) == (char *)-1)
    {
        printf("Shmat error\n");
        exit(0);
    }

    shmbuf = shmaddr;
    shared_data = (struct shared *) shmaddr;

    while (1)
    {
        while (shared_data->working == 1) {
            sleep(1);
        }
        sembuff[0].sem_num = 0;
        sembuff[0].sem_op = 1;
        sembuff[0].sem_flg = 0;

        if ((semop(semid, sembuff, 1)) == -1) {
            printf("Semop Error\n");
            exit(0);
        }

        printf("Message send: ");
        fgets(shmbuf, BUFSIZ, stdin);

        strncpy(shared_data->text, shmbuf, 1024);
        shared_data->working = 1;

        sembuff[0].sem_num = 0;
        sembuff[0].sem_op = -1;
        sembuff[0].sem_flg = 0;

        if ((semop(semid, sembuff, 1)) == -1) {
            printf("Semop Error\n");
            exit(0);
        }

        if (strncmp(shmbuf, "exit", 4) == 0) {
            break;
        }
    }

    if ((shmdt(shmaddr)) == -1) {
        printf("Shmdt error\n");
        exit(0);
    }
}

void *Receive(void * null)
{
    struct shared *shared_data;
    if ((shmaddr2 = shmat(shmid, NULL, 0)) == (char *)-1)
    {
        printf("Shmat error\n");
        exit(0);
    }

    shared_data = (struct shared *) shmaddr2;
    // printf("%s", shmaddr2);
    
    while (1)
    {
        if (shared_data->working) {
            sembuff2[0].sem_num = 0;
            sembuff2[0].sem_op = 1;
            sembuff2[0].sem_flg = 0;

            if ((semop(semid, sembuff2, 1)) == -1) {
                printf("Semop Error\n");
                exit(0);
            }

            printf("Message recv: %s", shared_data->text);
            sleep(rand() % 4);
            shared_data->working = 0;

            sembuff2[0].sem_num = 0;
            sembuff2[0].sem_op = -1;
            sembuff2[0].sem_flg = 0;

            if ((semop(semid, sembuff2, 1)) == -1) {
                printf("Semop Error\n");
                exit(0);
            }
        }
        if (strncmp(shared_data->text, "exit", 4) == 0) {
            break;
        }
    }

    if ((shmdt(shmaddr2)) == -1)
    {
        printf("Shmdt error\n");
        exit(0);
    }

    if (shmctl(shmid, IPC_RMID, 0) == -1)
    {
        printf("shmctl error\n");
        exit(0);
    }
}

int main()
{
    // if ((key = ftok( "/opt", 101)) == -1)
    // {
    //     printf("Couldn't create a uniqie key \n");
    //     exit(0);
    // }
    // pthread_t thread1;
    // pthread_t thread2;

    int key = getuid();
    
    srand(getpid());

    if ((shmid = shmget((key_t)key, sizeof(struct shared), 0666 | IPC_CREAT)) == -1)
    {
        printf("Message queue indentifier prob.\n");
        exit(0);
    }   
    // printf("Key: %d\nshmid: %d\n", key, shmid);

    printf("Welcome to syncronized multi user chatting Application\n\n");

    // if ((shmaddr = shmat(shmid, NULL, 0)) == (char *) -1) {
    //     printf("Shmat error\n");
    //     exit(0);
    // }

    if((semid = semget(key, 1, IPC_CREAT | 0666)) == -1) {
        printf("Failed to get a Semaphore ID\n");
        exit(0);
    }

    sema[0] = 0;

    if(( semctl(semid, 1, SETALL, sema)) == -1) {
        printf("Set semaphore control failed\n");
                  perror("semctl:");
        exit(0);
    }

    // pthread_create(&thread1, NULL, Sender, NULL);
    // pthread_create(&thread2, NULL, Receive, NULL);

    // pthread_join(thread1, NULL);
    // pthread_join(thread2, NULL);
    char in[10];
    while(1) {
        printf("1. Send\n2. Receive\n3. exit\n");
        scanf("%d",&i, in);

        if(i==1) {
            // Send
            struct shared *shared_data;
            if ((shmaddr = shmat(shmid, NULL, 0)) == (char *)-1)
            {
                printf("Shmat error\n");
                exit(0);
            }

            shmbuf = shmaddr;
            shared_data = (struct shared *) shmaddr;

            // while (1)
            // {
                printf("shared_data->working %d\n", shared_data->working);
                while (shared_data->working == 1) {
                    sleep(1);
                }
                while ( shared_data->some_one_writting == 2 )
                {
                    sleep(1);
                }
                shared_data->some_one_writting = 2;
                sembuff[0].sem_num = 0;
                sembuff[0].sem_op = 1;
                sembuff[0].sem_flg = 0;

                if ((semop(semid, sembuff, 1)) == -1) {
                    printf("Semop Error\n");
                    exit(0);
                }

                printf("Message send: ");
                fgets(shmbuf, BUFSIZ, stdin);

                strncpy(shared_data->text, shmbuf, 1024);
                shared_data->working = 1;
                shared_data->some_one_writting = 0;

                sembuff[0].sem_num = 0;
                sembuff[0].sem_op = -1;
                sembuff[0].sem_flg = 0;

                if ((semop(semid, sembuff, 1)) == -1) {
                    printf("Semop Error\n");
                    exit(0);
                }

                if (strncmp(shmbuf, "exit", 4) == 0) {
                    break;
                }
            // }

            if ((shmdt(shmaddr)) == -1) {
                printf("Shmdt error\n");
                exit(0);
            }
        }
        else if ( i == 2 ) {
            struct shared *shared_data;
            if ((shmaddr2 = shmat(shmid, NULL, 0)) == (char *)-1)
            {
                printf("Shmat error\n");
                exit(0);
            }

            shared_data = (struct shared *) shmaddr2;
            // printf("%s", shmaddr2);
            
            // while (1)
            // {
                // if (shared_data->working) {
                    sembuff2[0].sem_num = 0;
                    sembuff2[0].sem_op = 1;
                    sembuff2[0].sem_flg = 0;

                    if ((semop(semid, sembuff2, 1)) == -1) {
                        printf("Semop Error\n");
                        exit(0);
                    }

                    printf("Message recv: %s", shared_data->text);
                    sleep(rand() % 4);
                    shared_data->working = 0;

                    sembuff2[0].sem_num = 0;
                    sembuff2[0].sem_op = -1;
                    sembuff2[0].sem_flg = 0;

                    if ((semop(semid, sembuff2, 1)) == -1) {
                        printf("Semop Error\n");
                        exit(0);
                    }
                // }
                if (strncmp(shared_data->text, "exit", 4) == 0) {
                    break;
                }
            // }

            if ((shmdt(shmaddr2)) == -1)
            {
                printf("Shmdt error\n");
                exit(0);
            }

            if (shmctl(shmid, IPC_RMID, 0) == -1)
            {
                printf("shmctl error\n");
                exit(0);
            }
        }
        else {
            exit(0);
        }
    }
    return 0;
}
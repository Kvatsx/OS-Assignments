// Kaustav Vats(2016048)
// Saksham Vohra

#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include "shared.h"

key_t key;
int shmid;

char *shmaddr;
char *shmaddr2;
char *shmbuf;
char *shmbuf2;

int semflag;
int semid;
int semid2;

int i;
struct sembuf sembuff[1];
struct sembuf sembuff2[1];

void Sender()
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

void Receive()
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
    if ((key = ftok( "/opt", 101)) == -1)
    {
        printf("Couldn't create a uniqie key \n");
        exit(0);
    }
   
    if ((shmid = shmget(key, 1024, IPC_CREAT | 0666)) == -1)
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

    if(( semctl(semid, 1, SETALL, 0)) == -1) {
        printf("Set semaphore control failed\n");
                  perror("semctl:");
        exit(0);
    }

    // while(1) {
    //     printf("1. send\n2. recieve\n3. exit\n");
    //     scanf("%d",&i);

    //     if(i==1) {
    //         // Send
    //         shmbuf = shmaddr;
    //         //printf("%d\n",i );//sleep(3);
    //         fgets(shmbuf, BUFSIZ, stdin);
    //         //while(1){
    //         //printf("llllllllss\n");
    //         sembuffer[0].sem_num = 0;
    //         sembuffer[0].sem_op = 1;
    //         sembuffer[0].sem_flg = 0;
            
    //         if(( semop(semid, sembuffer, 1)) == -1) {
    //             printf("1.Failed in Semaphore operation\n");
    //             perror("semop:");
    //             exit(0);
    //         }
    //             //sleep(1);   
    //             // *shmbuf++ = c;
    //         printf("Enter some text: ");
    //         fgets(shmbuf, BUFSIZ, stdin);

    //         sembuffer[0].sem_num = 0;
    //         sembuffer[0].sem_op = -1;
    //         sembuffer[0].sem_flg = 0;
            
    //         if(( semop(semid, sembuffer, 1)) == -1) {
    //             printf("2.Failed in Semaphore operation\n");
    //             perror("semop:");
    //             exit(0);
    //         }
    //     }
    //     else if(i==2) {
    //         // Receive
    //     } 
    //     else if(i==3) {
    //         exit(0);
    //     }
    //     else {
    //         printf("llllllll\n");
        
    //     exit(0);
    //     }
    // }
    return 0;
}
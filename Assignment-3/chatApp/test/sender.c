#include<stdio.h>
#include<stdlib.h>
#include<sys/ipc.h>
#include<sys/types.h>
#include<sys/shm.h>
#include<string.h>
#include<sys/sem.h>

#define SEMNUMBER 1


int main()
{
    key_t key;
    int shmflag;
    int shmid;
    int shmsize = 27;
    char *shmaddr;
    char *shmbuf;
    char c;
    int semflag;
    int semid;
    short semaphore[SEMNUMBER];
    int i;
    struct sembuf sembuffer[1];

    if ((key = ftok( "/opt", 101)) == -1){
        printf("Failed to create unique key \n");
        exit(0);
    }
    shmflag = IPC_CREAT | 0666;
   
    if ((shmid = shmget(key,shmsize,shmflag)) == -1){
           printf("Failed to get a message queue identifier\n");
                  exit(0);
    }

    printf("....Welcome To Shared Memory IPC Mechanism\n");

    if ((shmaddr = shmat(shmid, NULL, 0)) == (char *) -1){
        printf("shmat failed\n");
                  perror("shmat:");
        exit(0);
    }

    semflag = IPC_CREAT | 0666;

    if((semid = semget(key, SEMNUMBER, semflag)) == -1){
           printf("Failed to get a Semaphore identifier\n");
                  perror("semget:");
           exit(0);
    }

    printf("Hey.... Now you are using Semaphore also\n");

    for(i=0; i < SEMNUMBER; i++){
        semaphore[i] = 0;
    }

    if(( semctl(semid, 1, SETALL, semaphore)) == -1){
        printf("Set semaphore control failed\n");
                  perror("semctl:");
        exit(0);
    }

   
    shmbuf = shmaddr;

    while(1){
        sembuffer[0].sem_num = 0;
        sembuffer[0].sem_op = 1;
        sembuffer[0].sem_flg = 0;
       
        if(( semop(semid, sembuffer, 1)) == -1){
            printf("1.Failed in Semaphore operation\n");
                          perror("semop:");
            exit(0);
        }
        sleep(1);   
        // *shmbuf++ = c;
        printf("Enter some text: ");
        fgets(shmbuf, BUFSIZ, stdin);

        sembuffer[0].sem_num = 0;
        sembuffer[0].sem_op = -1;
        sembuffer[0].sem_flg = 0;
       
        if(( semop(semid, sembuffer, 1)) == -1){
            printf("2.Failed in Semaphore operation\n");
                          perror("semop:");
            exit(0);
        }

    }

    *shmbuf = '\0';
   

        while(*shmaddr != '*')
        sleep(1);
    return 0;
}

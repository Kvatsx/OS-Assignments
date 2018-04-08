#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/ipc.h>
#include<sys/types.h>
#include<sys/shm.h>
#include<signal.h>
#include<sys/sem.h>

#define SEMNUMBER 1

void shmquit();

int shmid;
    int semid;

int main()
{
    int key;
    int shmflag;
    int shmsize = 27;
    char *shmaddr;
    char *shmbuf;
    int semflag;
    short semaphore[SEMNUMBER];
    int i;
    struct sembuf sembuffer[1];

    if ((key = ftok( "/opt", 101)) == -1){
        printf("Failed to create unique key \n");
        exit(0);
    }

     shmflag = IPC_CREAT | 0666;

    if ((shmid = shmget(key,shmsize, shmflag)) == -1){
           printf("Failed to get a shared memory identifier\n");
                  exit(0);
    }
    printf("....Welcome To Shared Memory IPC Mechanism\n");

    semflag = IPC_CREAT | 0666;

    if(( semid = semget(key, SEMNUMBER, semflag)) == -1){
           printf("Failed to get a Semaphore identifier\n");
                  exit(0);
    }
       
    printf("Hey now you are using semaphore also\n");

    for(i=0; i<SEMNUMBER; i++){
        semaphore[i] = 0;
    }

    if(( semctl(semid, 1, SETALL, semaphore)) == -1){
           printf("semaphore control Failed\n");
                  exit(0);
    }

    signal(SIGINT, (void *)shmquit);

    while(1){
   
        sembuffer[0].sem_num = 0;
        sembuffer[0].sem_op = 1;
        sembuffer[0].sem_flg = 0;

        if(( semop(semid, sembuffer, 1)) == -1){
            printf("Failed in semaphore operation\n");
            exit(0);
        }

        if( (shmaddr = shmat(shmid, NULL, 0)) == (char *) -1){
            printf("Shmat failed\n");
            exit(0);
        }
   
        printf("%s",shmaddr);
        // putchar('\n');

        if( (shmdt(shmaddr)) == -1){
            printf("Shmdt failed\n");
            exit(0);
        }
       
        sembuffer[0].sem_num = 0;
        sembuffer[0].sem_op = -1;
        sembuffer[0].sem_flg = 0;

        if(( semop(semid, sembuffer, 1)) == -1){
            printf("Failed in semaphore operation\n");
            exit(0);
        }

    }
    *shmaddr = '*';
    return 0;
}
void shmquit()
{
    signal(SIGINT, (void *)shmquit);
   
        shmctl(shmid, IPC_RMID, 0);
        semctl(semid,0, IPC_RMID);
        printf(" \nThank you for using Shared memory and semaphore .... DONE\n");   
        exit(0);

}

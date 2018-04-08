#include<stdio.h>
#include<stdlib.h>
#include<sys/ipc.h>
#include<sys/types.h>
#include<sys/shm.h>
#include<string.h>
#include<sys/sem.h>


int main()
{
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
    struct sembuf sembuffer[1];
    struct sembuf sembuffer2[1];

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

    if ((shmaddr = shmat(shmid, NULL, 0)) == (char *) -1){
        printf("Shmat error\n");
        exit(0);
    }

    if((semid = semget(key, 1, IPC_CREAT | 0666)) == -1){
        printf("Failed to get a Semaphore ID\n");
        exit(0);
    }

    if(( semctl(semid, 1, SETALL, 0)) == -1){
        printf("Set semaphore control failed\n");
                  perror("semctl:");
        exit(0);
    }
    int in;
    char n[10];
    while(1){
    printf("1. send\n2. recieve\n3. exit\n");
    scanf("%d",&i,n);

    if(i==1){
        // Send
        shmbuf = shmaddr;
        //printf("%d\n",i );//sleep(3);
        fgets(shmbuf, BUFSIZ, stdin);
        //while(1){
        //printf("llllllllss\n");
        sembuffer[0].sem_num = 0;
        sembuffer[0].sem_op = 1;
        sembuffer[0].sem_flg = 0;
          
        if(( semop(semid, sembuffer, 1)) == -1){
            printf("1.Failed in Semaphore operation\n");
            perror("semop:");
            exit(0);
        }
            //sleep(1);   
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
    else if(i==2){
        // Receive
        sembuffer2[0].sem_num = 0;
        sembuffer2[0].sem_op = 1;
        sembuffer2[0].sem_flg = 0;

        if(( semop(semid, sembuffer2, 1)) == -1){
            printf("Failed in semaphore operation\n");
            exit(0);
        }

        if( (shmaddr2 = shmat(shmid, NULL, 0)) == (char *) -1){
            printf("Shmat failed\n");
            exit(0);
        }
   
        printf("%s",shmaddr2);
        // putchar('\n');

        if( (shmdt(shmaddr2)) == -1){
            printf("Shmdt failed\n");
            exit(0);
        }
       
        sembuffer2[0].sem_num = 0;
        sembuffer2[0].sem_op = -1;
        sembuffer2[0].sem_flg = 0;

        if(( semop(semid, sembuffer2, 1)) == -1){
            printf("Failed in semaphore operation\n");
            exit(0);
        }

    //*shmaddr = '*';
    // return 0;
    }
        
    else if(i==3)
        exit(0);
    else{
        printf("llllllll\n");
      
      exit(0);
    }
}
   
   
    return 0;
}

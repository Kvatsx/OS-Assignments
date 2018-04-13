//Saksham Vohra 2016085
//Kaustav Vats 2016048

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/sem.h>
#include <unistd.h>
//queue is full
//queue is empty
//shared mem/semaphore creation error
//Index not yet written
#define SIZE 4

char *shmaddr[5];
char *shmaddr2[5];
char *shmaddr3[3];
int semflag;
int semid;
int semid2;
short semaphore[1];
int i;
struct sembuf sembuffer[1];
struct sembuf sembuffer2[1];


int main() {

    int i=0;
    for ( i=0; i<5; i++ ) {
        // Shared Memory on Messages.
        int key = (i+11);
        int shmid;
        if ((shmid = shmget(key, 1024, 0666 | IPC_CREAT)) == -1) {
            printf("SHMGET Error\n");
            exit(0);
        }
        char *add;
        if ( (add = (char *)shmat(shmid, NULL, 0)) == (char *)-1 ) {
            printf("SHMAT Error\n");
            exit(0);
        }
	    if ((semid = semget(key, 1, IPC_CREAT | 0666)) == -1)
	    {
	        printf("Failed to get a Semaphore identifier\n");
	        exit(0);
	    }

	    semaphore[0] = 0;
	    if(( semctl(semid, 1, SETALL, semaphore)) == -1){
	        printf("Set semaphore control failed\n");
	        exit(0);
	    }
        strcpy(add, "Empty");
        shmaddr[i] = add; 

        // Shared Memory Semaphores.
        int key2 = (i + 21);
        int shmid2;
        if ((shmid2 = shmget(key2, 1024, 0666 | IPC_CREAT)) == -1)
        {
            printf("SHMGET Error\n");
            exit(0);
        }
        char *add2;
        if ((add2 = (char *)shmat(shmid2, NULL, 0)) == (char *)-1)
        {
            printf("SHMAT Error\n");
            exit(0);
        }
        strcpy(add2, "Free");
        shmaddr2[i] = add2;

    }
    int kl;
    for ( kl=0; kl<3; kl++ )
    {
        int key = (kl + 50);
        int shmid3;
        if ((shmid3 = shmget(key, 1024, 0666 | IPC_CREAT)) == -1)
        {
            printf("SHMGET Error\n");
            exit(0);
        }
        char *add3;
        if ((add3 = (char *)shmat(shmid3, NULL, 0)) == (char *)-1)
        {
            printf("SHMAT Error\n");
            exit(0);
        }
        strcpy(add3, "0");
        shmaddr3[kl] = add3;
    }
    strcpy(shmaddr3[1], "-1");
    // printf("%s\t%s\n",shmaddr3[0], shmaddr3[1]);

    int j;
    int input;
    while(1) {
        printf("1. send\n2. recieve\n3. exit\n");
        scanf("%d",&input);
        // printf("input enter:  %d\n", input);
        if ( input == 1 ) {
            // Sending Message.
            // int end;
            // int start;
            
            // printf("Enter index Number: ");
            // scanf("%d", &index);
            int flag1 = 0;
            while( strcmp(shmaddr3[2], "1") == 0 )
            {
                if ( flag1 == 0 )
                {
                    printf("Someone is Writing\n");
                    sleep(1);
                    flag1 =1;
                }
            }
            strcpy(shmaddr3[2], "1");
            int start;
            int end;
            
            end = atoi(shmaddr3[1]);
            // int start;
            // printf("jdsfds %s\n",shmaddr3[0]);
            start = atoi(shmaddr3[0]);
            // printf("start: %d\nend: %d\n", start, end);
            int flag=0;
            // printf("Enter index Number: \n");
            if ( end == -1 )
            {
                end = end+1;
                flag =1;
            }
            else if ( ((end+1) % 5) == start )
            {
                printf("Queue is full\n");
                strcpy(shmaddr3[2], "0");
                continue;
            }
            else
            {
                end = (end + 1) % 5;
            }
            
            char *buffer;
            buffer = (char *)malloc(1024);
            sprintf(buffer, "%d", end);
            strcpy(shmaddr3[1], buffer);

            while (strcmp(shmaddr2[end], "Free") != 0)
            {
                sleep(1);
                if(flag==0){
                printf("Waiting for write permissions.\n");
            	}
            	flag =1;
            }
            // shmaddr2[index] = "Writing";
            sembuffer[0].sem_num = 0;
            sembuffer[0].sem_op = 1;
            sembuffer[0].sem_flg = 0;

            strcpy(shmaddr2[end], "Writing");
            fgets(shmaddr[end], BUFSIZ, stdin);
            printf("Message Send: ");
            fgets(shmaddr[end], BUFSIZ, stdin);
        
            if ((semop(semid, sembuffer, 1)) == -1)
            {
                printf("1.Failed in Semaphore operation\n");
                exit(0);
            }
            strcpy(shmaddr2[end], "Free");
            strcpy(shmaddr3[2], "0");
            sembuffer[0].sem_num = 0;
            sembuffer[0].sem_op = -1;
            sembuffer[0].sem_flg = 0;

            if ((semop(semid, sembuffer, 1)) == -1)
            {
                printf("2.Failed in Semaphore operation\n");
                exit(0);
            }
        }
        else if ( input == 2 ) {
            // Receiving Message.
            printf("Press\n1. Dequeue\n2. Enter index\n");
            int cho;
            scanf("%d", &cho);
            if ( cho == 1 ) {
                int index;
                int start;
                int end;
                start = atoi(shmaddr3[0]);
                end = atoi(shmaddr3[1]);
                // printf("start: %d\nend: %d\n", start, end);

                if ( start == 0 && end == -1 )
                {
                    printf("Queue Empty!\n");
                    continue;
                }
                else if ( start == end )
                {
                    // printf("hey1\n");
                    index = start;
                    start = 0;
                    end = -1;
                    char* buffer;
                    buffer = (char *) malloc(1024);
                    char* buffer2;
                    buffer2 = (char *)malloc(1024);
                    sprintf(buffer,"%d",0);
                    sprintf(buffer2,"%d",-1);
                    strcpy(shmaddr3[0], buffer);
                    strcpy(shmaddr3[1], buffer2);
                }
                else {
                    // printf("hey2\n");
                    index = start;
                    start = (start + 1) % 5;
                    char *buffer2;
                    buffer2 = (char *)malloc(1024);
                    sprintf(buffer2, "%d", start);
                    strcpy(shmaddr3[0], buffer2);
                }


                int flag2 = 0;
                while (strcmp(shmaddr2[index], "Writing") == 0)
                {
                    sleep(1);
                    if (flag2 == 0)
                    {
                        printf("Someone is writing currently.\n");
                    }
                    flag2 = 1;
                }
                // printf("value: %s\n", shmaddr[index]);
                // shmaddr2[index] = "Reading";
                sembuffer2[0].sem_num = 0;
                sembuffer2[0].sem_op = 1;
                sembuffer2[0].sem_flg = 0;

                strcpy(shmaddr2[index], "Writing");
                printf("Message Recv: %s\n", shmaddr[index]);
                char* ch;
                strcpy(ch,"Empty");
                shmaddr[index] = ch;
                if ((semop(semid, sembuffer2, 1)) == -1)
                {
                    printf("Failed in semaphore operation\n");
                    exit(0);
                }
                strcpy(shmaddr2[index], "Free");
                sembuffer2[0].sem_num = 0;
                sembuffer2[0].sem_op = -1;
                sembuffer2[0].sem_flg = 0;

                if ((semop(semid, sembuffer2, 1)) == -1)
                {
                    printf("Failed in semaphore operation\n");
                    exit(0);
                }
            }
            else {
                int index;
                printf("Enter index Number: ");
                scanf("%d", &index);
                int flag2 = 0;
                while (strcmp(shmaddr2[index], "Writing") == 0)
                {
                    sleep(1);
                    if (flag2 == 0)
                    {
                        printf("Someone is writing currently.\n");
                    }
                    flag2 = 1;
                }
                // printf("value: %s\n", shmaddr[index]);
                // shmaddr2[index] = "Reading";
                sembuffer2[0].sem_num = 0;
                sembuffer2[0].sem_op = 1;
                sembuffer2[0].sem_flg = 0;

                strcpy(shmaddr2[index], "Reading");
                printf("Message Recv: %s\n", shmaddr[index]);
                if ((semop(semid, sembuffer2, 1)) == -1)
                {
                    printf("Failed in semaphore operation\n");
                    exit(0);
                }
                strcpy(shmaddr2[index], "Free");
                sembuffer2[0].sem_num = 0;
                sembuffer2[0].sem_op = -1;
                sembuffer2[0].sem_flg = 0;

                if ((semop(semid, sembuffer2, 1)) == -1)
                {
                    printf("Failed in semaphore operation\n");
                    exit(0);
                }
            }
        }
        else {
            exit(0);
        }
    }

}

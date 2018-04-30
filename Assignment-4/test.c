#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define BUFFERSIZE 256

static char InputString[256];
static char OutputString[256];

void Testing(void)
{
	int fd;
	FILE *fp;
   char data[300];
   fp = fopen("data.txt", "r");
   fseek(fp, 0, SEEK_SET);
   fread(data, 256, 1, fp);
   fclose(fp);
   FILE *fp1;
   char key[16];
   char key1[16];
   fp1 = fopen("/dev/urandom","r");
   fread(key, 1, 16, fp1);
   fclose(fp1);
 //   char t[257];
 //   char message[256];
   int i,y;
   for(i=0; i<16; i++)
   {
        key1[i] = key[i];
   }
 //   // printf("sasa\n");
 //   // for(int x = 0;x<16;x++){
 //   // 	printf("%d,%d\n",(int)key[x],(int)key1[x]);
 //   // }
 //   // printf("sasa\n");
   strcat(data,"\0");
 //   printf("%s\n", data);
 //   printf("%c\n", data[strlen(data)-1]);
 //   // char data[256];
 //   /* Read and display data */
 //   	int iter;
 //   	printf("le: %d\n", strlen(data));
	// for(iter = 0; iter<strlen(data); iter++)
	// {
	// 	printf("%d\n", iter);
	// 	// if(data[iter]!='\0')
	// 	// {
	// 		t[iter] = (int)data[iter]^(int)key[iter%16];
	// 		key[iter%16] = t[iter];
	// 		printf("data: %c\n", data[iter]);
	// 	// }
	// 	// else
	// 	// {
	// 		// t[iter] = '\0';
	// 	// }
	// 	// printf("Encrypted message = %s\n", t);
	// }
	// t[iter] = '\0';
	// printf("%d\n", strlen(t));

 //    if ( (fd = open("/dev/encdev", O_RDWR)) < 0 )
 //    {
 //        printf("Failed to open /dev/encdev device.\n");
 //        return -1;
 //    }
 //    char tle[300];
 //    int retv;
 //    char NKey[16];
 //    if ( (retv = read(fd, NKey, 16)) < 0 )
 //    {
 //        perror("asndakjd Failed to recieve the message.\n");
 //        printf("%d\n", errno);
 //        return errno;
 //    }

 //    if ( (retv = read(fd, tle, strlen(data))) < 0 )
 //    {
 //        perror("asndakjd Failed to recieve the message.\n");
 //        printf("%d\n", errno);
 //        return errno;
 //    }
 //   printf("Encrypted Message = \n");
 //   printf("%s\n", tle);
   
 //   int iter1;
 //   for(iter1 = 0; iter1<strlen(data);iter1++)
 //   {
 //   	if(tle[iter1] != '\0')
 //   	{
 //   		message[iter1] = (int)tle[iter1]^(int)NKey[iter1%16];
 //   		NKey[iter1%16] = tle[iter1];
 //        printf("tle: %c\n", tle[iter]);
 //   	}
 //   	else
 //   	{
 //   		message[iter1] = '\0';
 //   	}
 //   }
 //   message[iter1]= '\0';
 //   printf("Decrypted Message=\n");
 //   printf("%s\n", message);
 //   printf("\n");
   // printf("aaaa%s\n", key);
   // printf("%s\n",buffer );
   
	
	// char t1[17];
 //   int i1;
 //   for(i1 =0;i1<16;i1++){
 //   	t1[i1] = t[i1]^(int)key[i1];
   	
 //   }
 //   t1[i1]='\0';
 //   	printf("decrypted message = %s\n",t1 );
	if ( (fd = open("/dev/encdev", O_RDWR)) < 0 )
	{
		printf("Failed to open /dev/encdev device.\n");
		return -1;
	}
	// printf("Enter your Message: ");
	// fgets(InputString, 256, stdin);
	// InputString[strlen(InputString)-1] = '\0';
	printf("Message Entered by user: %s\n", data);

    char Keyz[16];
    // int i;
    // for ( i=0; i<16; i++ )
    // {
    //     Keyz[i] = key1;
    // }
	int returnedValue;
	if ( (returnedValue = write(fd, key1, 16)) < 0 )
	{
		perror("Failed to write the Keyz.\n");
		return -1;
	}
    getchar();
    if ( (returnedValue = write(fd, data, strlen(data))) < 0 )
    {
        printf("Failed to write the data.\n");
        return -1;
    }

    // char New_Key[16];
    // printf("Reading back from /dev/encdev\n");
    // if ( (returnedValue = read(fd, New_Key, 16)) < 0 )
    // {
    //     perror("Failed to recieve the Key.\n");
    //     printf("%d\n", errno);
    //     return errno;
    // }
    // printf("Lenght of New_Key: %d\n", strlen(New_Key));
	// getchar();
	printf("Reading back from /dev/encdev\n");
	if ( (returnedValue = read(fd, OutputString, BUFFERSIZE)) < 0 )
	{
		perror("Failed to recieve the message.\n");
		printf("%d\n", errno);
		return errno;
	}
    printf("lenght: %d\n", strlen(OutputString));
    printf("OutputString: %s\n", OutputString);
    // Dec_device
    int fd2;
    printf("Writing to decdev\n");
    if ( (fd2 = open("/dev/decdev", O_RDWR)) < 0 )
    {
        printf("Failed to open /dev/decdev device.\n");
        return -1;
    }

    if ( (returnedValue = write(fd2, key1, 16)) < 0 )
    {
        perror("Failed to write the Keyz.\n");
        return -1;
    }
    getchar();
    if ( (returnedValue = write(fd2, OutputString, strlen(data))) < 0 )
    {
        printf("Failed to write the OutputString.\n");
        return -1;
    }

    printf("Reading back from /dev/decdev\n");
    if ( (returnedValue = read(fd2, OutputString, BUFFERSIZE)) < 0 )
    {
        perror("Failed to recieve the message.\n");
        printf("%d\n", errno);
        return errno;
    }

	printf("Mssg recv decrypted: %s\n", OutputString);
	printf("Exiting\n");
}

int main(int argc, char const *argv[])
{
	Testing();
	return 0;
}
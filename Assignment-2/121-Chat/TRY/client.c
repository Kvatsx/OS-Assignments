#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#define BUFSIZE 1024
int main(int argc, char const *argv[])
{
  int sockfd, size, i;
  struct sockaddr_in server;
  fd_set master;
  fd_set other;
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	perror("Socket");
	exit(1);
  }
  server.sin_family = AF_INET;
  server.sin_port = htons(5555);
  server.sin_addr.s_addr = inet_addr("127.0.0.1");
  memset(server.sin_zero, '\0', sizeof server.sin_zero);
  connect(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr));
  FD_ZERO(&master);
  FD_ZERO(&other);
  FD_SET(0, &master);
  FD_SET(sockfd, &master);
  size = sockfd;
  printf("\nMenu\n1) To send to a client followed by port number and message.\n2) To send message to all clients.\n3) Exit\n");
  while(1){
	other = master;
	select(size+1, &other, NULL, NULL, NULL);
	char input[2];
	for(i=0; i <= size; i++ )
	{
		if(FD_ISSET(i, &other))
		{
			char buffer_out[BUFSIZE];
			char buffer_in[BUFSIZE];
			int recieved;
			if (i == 0){
				
				scanf("%s", input);
				// fgets(input, 2, stdin);
				printf("input: %s\n", input);
				memset(&buffer_out[0], '\0', BUFSIZE);
				if ( strcmp(input, "1") == 0 )
				{
					char  port[10];
<<<<<<< HEAD
					// scanf("%s", port);
					fgets(port, 10, stdin);
					printf("port: %s\n", port);
					// scanf("%s", buffer_out);
					fgets(buffer_out, BUFSIZE, stdin);
					printf("buffer_out: %s\n", buffer_out);
=======
					scanf("%s", port);
					scanf("%[^\n]", buffer_out);
>>>>>>> 08de66d2818d77a97b3afe0a491997a3ae609ac0
					send(sockfd, input, sizeof(input), 0);
					send(sockfd, port, sizeof(port), 0);
			  		send(sockfd, buffer_out, strlen(buffer_out), 0);
				}
				else if ( strcmp(input, "2") == 0 )
				{
<<<<<<< HEAD
					// printf("Enter your message: ");
					fgets(buffer_out, BUFSIZE, stdin);
					printf("Your message: %s\n", buffer_out);
					// scanf("%s", &buffer_out);
=======
					// fgets(buffer_out, BUFSIZE, stdin);
					scanf("%[^\n]", &buffer_out);
>>>>>>> 08de66d2818d77a97b3afe0a491997a3ae609ac0
					send(sockfd, input, sizeof(input), 0);
			  		send(sockfd, buffer_out, strlen(buffer_out), 0);
				}
				else if( strcmp(input, "3")==0)
				{
					exit(1);
				}
				else{
<<<<<<< HEAD
					printf("INCORRECT INPUT\n");
					// printf("\nMenu\n1) To send to a client followed by port number and message.\n2) To send message to all clients.\n3) Exit");
=======
					continue;
>>>>>>> 08de66d2818d77a97b3afe0a491997a3ae609ac0
				}
			}
			else {
				printf("Waiting for clients message.\n");
			  	recieved = recv(sockfd, buffer_in, BUFSIZE, 0);
			  	buffer_in[recieved] = '\0';
			  	printf("%s\n" , buffer_in);
			  	printf("\nMenu\n1) To send to a client followed by port number and message.\n2) To send message to all clients.\n3) Exit\n");
			}
		}
	}
	printf("\nMenu\n1) To send to a client followed by port number and message.\n2) To send message to all clients.\n3) Exit\n");
  }
  printf("client-quited\n");
  close(sockfd);
  return 0;
}
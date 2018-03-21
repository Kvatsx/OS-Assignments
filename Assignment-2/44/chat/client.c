// Kaustav Vats (2016048)
// Saksham Vohra (2016085)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>

#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/socket.h>

#define BUFSIZE 1024

int main(int argc, char const *argv[])
{
  int fd_socket, size;
  struct sockaddr_in server;
  
  fd_set main;
  fd_set other;
  
  if ((fd_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
  {
	printf("Socket not creating\n");
	exit(1);
  }
  
  server.sin_family = AF_INET;
  server.sin_port = htons(5555);
  server.sin_addr.s_addr = inet_addr("127.0.0.1");
  
  memset(server.sin_zero, '\0', sizeof server.sin_zero);
  connect(fd_socket, (struct sockaddr *)&server, sizeof(struct sockaddr));
  
  FD_ZERO(&main);
  FD_ZERO(&other);
  
  FD_SET(0, &main);
  FD_SET(fd_socket, &main);
  
  size = fd_socket;
  int times = 1;
  printf("\nMenu\n1) To send to a client followed by port number and message.\n2) To send message to all clients.\n3) Exit\n");
  while(1)
  {
	other = main;
	select(size+1, &other, NULL, NULL, NULL);
	char input[2];
	int i;
	for ( i=0; i<=size; i++ )
	{
		if(FD_ISSET(i, &other))
		{	//printf("\nMenu\n1) To send to a client followed by port number and message.\n2) To send message to all clients.\n");
			char buffer_out[BUFSIZE];
			char buffer_in[BUFSIZE];
			int recieved;
			if (i == 0)
			{
				
				scanf("%s", input);
				// fgets(input, 2, stdin);
				printf("input: %s\n", input);
				memset(&buffer_out[0], '\0', BUFSIZE);
				if ( strcmp(input, "1") == 0 )
				{
					char  port[10];
					// scanf("%s", port);
					fgets(port, 10, stdin);
					printf("port: %s\n", port);
					// scanf("%s", buffer_out);
					fgets(buffer_out, BUFSIZE, stdin);
					printf("buffer_out: %s\n", buffer_out);
					
					send(fd_socket, input, sizeof(input), 0);
					send(fd_socket, port, sizeof(port), 0);
			  		send(fd_socket, buffer_out, strlen(buffer_out), 0);
				}
				else if ( strcmp(input, "2") == 0 )
				{
					// printf("Enter your message: ");
					fgets(buffer_out, BUFSIZE, stdin);
					printf("Your message: %s\n", buffer_out);
					// scanf("%s", &buffer_out);
					send(fd_socket, input, sizeof(input), 0);
			  		send(fd_socket, buffer_out, strlen(buffer_out), 0);
				}
				else if( strcmp(input , "3") == 0)
				{
					exit(1);
				}
				else
				{
					printf("INCORRECT INPUT\n");
					// printf("\nMenu\n1) To send to a client followed by port number and message.\n2) To send message to all clients.\n3) Exit");
				}
				printf("\nMenu\n1) To send to a client followed by port number and message.\n2) To send message to all clients.\n3) Exit\n");
			}
			else 
			{
				printf("Waiting for clients message.\n");
			  	
			  	recieved = recv(fd_socket, buffer_in, BUFSIZE, 0);
			  	buffer_in[recieved] = '\0';
			  	
			  	printf("%s\n" , buffer_in);
			  	printf("\nMenu\n1) To send to a client followed by port number and message.\n2) To send message to all clients.\n3) Exit\n");
			}
		}
	}
  }
  close(fd_socket);
  return 0;
}
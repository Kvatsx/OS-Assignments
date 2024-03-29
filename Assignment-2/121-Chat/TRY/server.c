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

#define PORT 5555
#define BUFSIZE 1024

int main(int argc, char const *argv[])
{
	int ports[20];
	int fds[20];
	int connections = 0;
	
	fd_set main;
	
	fd_set other;
	int size;
	
	int fd_serv= 0;
	
	struct sockaddr_in server, client;
	
	FD_ZERO(&main);
	FD_ZERO(&other);
	
	int yes = 1;
	if ((fd_serv = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
	{
		printf("Socket not connected\n");
		exit(1);
	} 

	server.sin_family = AF_INET;
	server.sin_port = htons(5555);
	server.sin_addr.s_addr = INADDR_ANY;
	
	memset(server.sin_zero, '\0', sizeof server.sin_zero);
	
	bind(fd_serv, (struct sockaddr *)&server, sizeof(struct sockaddr));
	
	listen(fd_serv, 10);
	
	printf("\nTCPServer Waiting for client on port 5555\n");
	FD_SET(fd_serv, &main);
	
	size = fd_serv;
	while(1)
	{
		
		other = main;
		select(size+1, &other, NULL, NULL, NULL);
		
		int i;
		for ( i=0; i<=size; i++ ) 
		{
			if (FD_ISSET(i, &other))
			{
				if (i == fd_serv)
				{
					socklen_t s_len;
					int newfd_serv;
					
					s_len = sizeof(struct sockaddr_in);
					
					if((newfd_serv = accept(fd_serv, (struct sockaddr *)&client, &s_len)) == -1) 
					{
						printf("socket accepting problem\n");
						exit(1);
					}
					else 
					{
						FD_SET(newfd_serv, &main);
						
						if(newfd_serv > size)
						{
							size = newfd_serv;
						}
						ports[connections] = ntohs(client.sin_port);
						fds[connections] = newfd_serv;
						connections++;
						
						printf("new connection from port %d \n", ntohs(client.sin_port));
						printf("Newfd_serv: %d\n", newfd_serv);
					}
				}
				else
				{
					int recieved, j;
					char buffer[BUFSIZE], buf[BUFSIZE], buffernew[BUFSIZE];
					char port[10];
					char input[2];
					
					memset(&buffer[0], '\0', BUFSIZE);
					memset(&buffernew[0], '\0', BUFSIZE);
					// char *in = (char *)&input;
					// printf("Input initial: %s\n", input);
					recv(i, input, sizeof(input), 0);
					// printf("Input val:- %s\n", input);
					// if ((recieved = recv(i, in, sizeof(in), 0)) <= 0) {
						// if (recieved == 0) {
							// printf("socket %d hung up\n", i);
						// }else {
							// perror("recv");
						// }
					// }
					// printf("Input: %s\n", input);
					// in =1;
					// int a = atoi(input);
					
					// printf("%d\n", a );
	
					if (strcmp(input, "1") == 0 )
					{
						printf("Recieved input = 1\n");
						
						recv(i, port, sizeof(port), 0);
						int b = atoi(port);
						printf("%d\n", b);
						
						if ((recieved = recv(i, buffer, BUFSIZE, 0)) <= 0) 
						{
							if (recieved == 0) 
							{
								printf("socket %d hung up\n", i);
							}
							else 
							{
								printf("Mssg not recieved\n");
							}
							close(i);

							FD_CLR(i, &main);
						}else 
						{ 
							printf("%s\n", buffer);
							int index = -1;
							int ij;
							printf("connections: %d\n", connections);
							for ( ij=0; ij<connections; ij++)
							{
								// printf("Port[%d] %d\n",ij, ports[ij] );
								if ( ports[ij] == b )
								{
									printf("Port: %d\n", ports[ij]);
									printf("FD: %d\n", fds[ij]);
									index = ij;
									break;
								}
							}
							if ( index != -1 && i!= fds[index])
							{
								printf("buffer: %s\n", &buffer);
								for ( ij=0; ij<connections; ij++ )
								{
									if ( fds[ij] == i )
									{
										break;
									}
								}
								//printf("B=%d\nport=%d\n",htons(client.sin_port),ports[ij] );
								int len = sprintf(buffernew,"%d says: %s",ports[ij], buffer);
								if (send(fds[index], buffernew, len, 0) == -1) {
									printf("Mssg not sent\n");
								}
								// else{
								// 	send(fds[index],"\nMenu\n1) To send to a client followed by port number and message.\n2) To send message to all clients.\n3) Exit\n",114,0);
								// }
								printf("Message send to the client.\n");
							}
							else
							{
								if ( i == fds[index] )
								{
								//	if(htons(client.sin_port)== ports[index]){
									send(fds[index],"Error: port number same as client", 33,0);
									send(fds[index],"\nMenu\n1) To send to a client followed by port number and message.\n2) To send message to all clients.\n3) Exit\n",114,0);
								}
								else
									printf("\n Wrong port number\n");
							}
						}
					}
					else
					{
						if ((recieved = recv(i, buffer, BUFSIZE, 0)) <= 0) 
						{
							if (recieved == 0) 
							{
								printf("socket %d hung up\n", i);
							}
							else 
							{
								printf("Group Mssg not Recieved\n");
							}
							close(i);
							FD_CLR(i, &main);
						}
						else 
						{ 
							printf("Messg-2-> %s\n", buffer);
							for(j = 0; j <= size; j++)
							{
								if (FD_ISSET(j, &main))
								{
									if (j != fd_serv && j != i) 
									{
										int ij;
										for ( ij=0; ij<connections; ij++ )
										{
											if ( fds[ij] == i )
											{
												break;
											}
										}
										int len = sprintf(buffernew,"%d says: %s", ports[ij],buffer);
										// send(j,"\nMenu\n1) To send to a client followed by port number and message.\n2) To send message to all clients.\n3) Exit\n",114,0);
										if (send(j, buffernew, len, 0) == -1) 
										{
											printf("Group Mssg not sent\n");
										}
										else
										{
											printf("Sending Menu:\n");
											// send(j,"\nMenu\n1) To send to a client followed by port number and message.\n2) To send message to all clients.\n3) Exit\n",114,0);
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return 0;
}
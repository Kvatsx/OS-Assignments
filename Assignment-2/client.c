#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAXSIZE 1024

int main(int argc, char const *argv[])
{
	int s;
	struct sockaddr_in server;
	fd_set Main;
	FD_ZERO(&Main);
	FD_SET(0, &Main);
	FD_SET(s, &Main);
	int size = s;
	char buff[MAXSIZE];

	s = socket(AF_INET, SOCK_STREAM, 0);
	server.sin_family = AF_INET;
	server.sin_port = htons(4950);
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(server.sin_zero, '\0', sizeof(server.sin_zero));

	connect(s, (struct sockaddr *)&server, sizeof(struct sockaddr));

	while(1)
	{
		int i;
		for ( i=0; i<=size; i++ )
		{
			if ( FD_ISSET(i, &Main) )
			{
				if ( i==0 )
				{
					printf("Enter your reply: ");
					fgets(buff, MAXSIZE, stdin);
					send(s, buff, strlen(buff), 0);
				}
				else
				{
					int len = recv(s, buff, MAXSIZE, 0);
					buff[len] = '\0';
					printf("Message from other users: %s\n", buff);
				}
			}
		}

	}

	return 0;
}

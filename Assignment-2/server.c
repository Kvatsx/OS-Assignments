#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT 5555
#define MAXSIZE 1024

int main(int argc, char const *argv[])
{
	fd_set Main;
	int size;
	int s,s2;
	struct sockaddr_in local, remote;
	socklen_t sock_size;
	FD_ZERO(&Main);

	if ( (s = socket(AF_INET, SOCK_STREAM, 0)) == -1 )
	{
		printf("Socket not created.\n");
		return 0;
	}
	local.sin_family = AF_INET;
	local.sin_port = htons(4950);
	local.sin_addr.s_addr = INADDR_ANY;
	memset(local.sin_zero, '\0', sizeof(local.sin_zero));

	bind(s, (struct sockaddr *)&local, sizeof(struct sockaddr));
	listen(s, 10);
	size = s;
	printf("Server waiting for clients.\n");
	FD_SET(s, &Main);

	while(1)
	{
		int i;
		for ( i = 0; i <= size; i++ )
		{
			if ( FD_ISSET(i, &Main) )
			{
				if ( i == s )
				{
					sock_size = sizeof(struct sockaddr_in);
					s2 = accept(s,(struct sockaddr *)&remote, &sock_size);
					FD_SET(s2, &Main);
					if ( s2 > size )
					{
						size = s2;
					}
					printf("Connected to client on port %d\n", ntohs(remote.sin_port));
				}
				else
				{
					int len;
					char buff[MAXSIZE];
					if ( (len = recv(i, buff, MAXSIZE, 0)) <= 0 )
					{
						printf("Socket not responding.\n");
						close(i);
						FD_CLR(i, &Main);
					}
					else
					{
						int j;
						for ( j=0; j<= size; j++ )
						{
							if ( FD_ISSET(j, &Main) && j != s && j != i )
							{
								send(j, buff, strlen(buff), 0);
							}
						}
					}
				}
			}
		}
	}

	return 0;
}
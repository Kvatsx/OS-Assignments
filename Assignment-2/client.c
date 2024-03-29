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
  while(1){
    other = master;
    select(size+1, &other, NULL, NULL, NULL);
    for(i=0; i <= size; i++ )
      if(FD_ISSET(i, &other))
      {
        char buffer_out[BUFSIZE];
        char buffer_in[BUFSIZE];
        int recieved;
        if (i == 0){
          fgets(buffer_out, BUFSIZE, stdin);
          send(sockfd, buffer_out, strlen(buffer_out), 0);
        }else {
          recieved = recv(sockfd, buffer_in, BUFSIZE, 0);
          buffer_in[recieved] = '\0';
          printf("%s\n" , buffer_in);
        }
      }
  }
  printf("client-quited\n");
  close(sockfd);
  return 0;
}
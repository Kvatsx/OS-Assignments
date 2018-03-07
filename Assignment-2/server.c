#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#define PORT 5555
#define BUFSIZE 1024
int main(int argc, char const *argv[])
{
  fd_set master;
  fd_set other;
  int size, i;
  int sockfd= 0;
  struct sockaddr_in server, client;
  FD_ZERO(&master);
  FD_ZERO(&other);
  int yes = 1;
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Socket");
    exit(1);
  } 
  server.sin_family = AF_INET;
  server.sin_port = htons(5555);
  server.sin_addr.s_addr = INADDR_ANY;
  memset(server.sin_zero, '\0', sizeof server.sin_zero);
  bind(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr));
  listen(sockfd, 10);
  printf("\nTCPServer Waiting for client on port 5555\n");
  FD_SET(sockfd, &master);
  size = sockfd;
  while(1){
    other = master;
    select(size+1, &other, NULL, NULL, NULL);
    for (i = 0; i <= size; i++){
      if (FD_ISSET(i, &other)){
        if (i == sockfd)
        {
          socklen_t socketlen;
          int newsockfd;
          socketlen = sizeof(struct sockaddr_in);
          if((newsockfd = accept(sockfd, (struct sockaddr *)&client, &socketlen)) == -1) {
            perror("accept");
            exit(1);
          }else {
            FD_SET(newsockfd, &master);
            if(newsockfd > size){
              size = newsockfd;
            }
            printf("new connection from port %d \n", ntohs(client.sin_port));
          }
        }
        else
        {
          int recieved, j;
          char buffer[BUFSIZE], buf[BUFSIZE];
          if ((recieved = recv(i, buffer, BUFSIZE, 0)) <= 0) {
            if (recieved == 0) {
              printf("socket %d hung up\n", i);
            }else {
              perror("recv");
            }
            close(i);
            FD_CLR(i, &master);
          }else { 
            printf("%s\n", buffer);
            for(j = 0; j <= size; j++){
              if (FD_ISSET(j, &master)){
                if (j != sockfd && j != i) {
                  if (send(j, buffer, recieved, 0) == -1) {
                    perror("send");
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
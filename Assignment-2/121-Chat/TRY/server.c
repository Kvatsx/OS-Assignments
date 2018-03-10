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
  int ports[20];
  int fds[20];
  int connections = 0;
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
            ports[connections] = ntohs(client.sin_port);
            fds[connections] = newsockfd;
            connections++;
            printf("new connection from port %d \n", ntohs(client.sin_port));
            printf("NewSockFD: %d\n", newsockfd);
          }
        }
        else
        {
          int recieved, j;
          char buffer[BUFSIZE], buf[BUFSIZE], buffernew[BUFSIZE];
          char port[10];
          char input[2];
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
              if ( index != -1 )
              {
                printf("buffer: %s\n", &buffer);
                sprintf(buffernew,"%d says: %s",ports[index], buffer);
                if (send(fds[index], buffernew, recieved+12, 0) == -1) {
                  perror("send");
                }
                printf("Message send to the client.\n");
              }
              else
              {
                printf("\n Wrong port number\n");
              }
            }
          }
          else
          {
            if ((recieved = recv(i, buffer, BUFSIZE, 0)) <= 0) {
              if (recieved == 0) {
                printf("socket %d hung up\n", i);
              }else {
                perror("recv");
              }
              close(i);
              FD_CLR(i, &master);
            }
            else { 
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
  }
  return 0;
}
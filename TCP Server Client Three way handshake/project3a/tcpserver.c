#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

/* This is a reference socket server implementation that prints out the messages
 * received from clients. */

#define MAX_PENDING 20
#define MAX_LINE 20

int main(int argc, char *argv[]) {
  char* host_addr = "127.0.0.1";
  int port = atoi(argv[1]);

  /*setup passive open*/
  int s;
  if((s = socket(PF_INET, SOCK_STREAM, 0)) <0) {
    perror("simplex-talk: socket");
    exit(1);
  }

  /* Config the server address */
  struct sockaddr_in sin;
  sin.sin_family = AF_INET; 
  sin.sin_addr.s_addr = inet_addr(host_addr);
  sin.sin_port = htons(port);
  // Set all bits of the padding field to 0
  memset(sin.sin_zero, '\0', sizeof(sin.sin_zero));

  /* Bind the socket to the address */
  if((bind(s, (struct sockaddr*)&sin, sizeof(sin)))<0) {
    perror("simplex-talk: bind");
    exit(1);
  }

  // connections can be pending if many concurrent client requests
  listen(s, MAX_PENDING);  

  /* wait for connection, then receive and print text */
  int new_s;
  socklen_t len = sizeof(sin);
  char buf[MAX_LINE];
  while(1) {
    if((new_s = accept(s, (struct sockaddr *)&sin, &len)) <0){
      perror("simplex-talk: accept");
      exit(1);
    }

    //receive and print hello x
    recv(new_s, buf, sizeof(buf), 0);
    fputs(buf, stdout);
    fputc('\n', stdout);
    fflush(stdout);

//send hello x+1 back to server
  int num_rcvd;
  int num_send;

  char hello[10];
  sscanf(buf, "%s %d", hello, &num_rcvd);
  num_send = num_rcvd+1;
  sprintf(buf, "%s %d", "HELLO ", num_send);

//printf("buf to send back %s\n", buf);

  int len = strlen(buf)+1;
  send(new_s, buf, len, 0);

    //receive, check and print hello x+2
    recv(new_s, buf, sizeof(buf), 0);
    sscanf(buf, "%s %d", hello, &num_rcvd);
    if ((num_rcvd - 1) != num_send){
      perror("ERROR ");
      close(new_s);
      exit(1);
    	}
    
    fputs(buf, stdout);
    fputc('\n', stdout);
    fflush(stdout);
    
    close(new_s);
  }

  return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#define MAX_LINE 20

int main (int argc, char *argv[]) {
  char* host_addr = argv[1];
  int port = atoi(argv[2]);
  int hello_num = atoi(argv[3]);
  int num_rcvd;
  char hello[10];

//printf("%d on line 16 is :\n", hello_num);

  /* Open a socket */
  int s;
  if((s = socket(PF_INET, SOCK_STREAM, 0)) <0){
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

  /* Connect to the server */
  if(connect(s, (struct sockaddr *)&sin, sizeof(sin))<0){
    perror("simplex-talk: connect");
    close(s);
    exit(1);
  }

  char buf[MAX_LINE];
  //sending hello X to server
     sprintf(buf, "%s %d", "HELLO ", hello_num);
   // buf[MAX_LINE-1] = '\0';
    int len = strlen(buf)+1;
    send(s, buf, len, 0);
  
  //Receiving hello x+1 from server,checking it and printing it 
    recv(s, buf, sizeof(buf), 0);

    sscanf(buf, "%s %d", hello, &num_rcvd);
    if ((num_rcvd - 1) != hello_num){
      perror("ERROR ");
      close(s);
      exit(1);
    }
     
    fputs(buf, stdout);
    fputc('\n', stdout);
    fflush(stdout);

    int num_send = num_rcvd+1;
     sprintf(buf, "%s %d", "HELLO ", num_send);

//sending hello X+2 to server
     sprintf(buf, "%s %d", "HELLO ", num_send);
     len = strlen(buf)+1;
    send(s, buf, len, 0);

// closing connection on client side
    close(s);

  return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

/* This is a reference socket server implementation that prints out the messages
 * received from clients. */

//#define DEBUG
#define MAX_PENDING 100
#define MAX_LINE 20

int fhCt = 0;
int shCt = 0;

// define client_status struct
  typedef struct {
	int fd;
	int status;
	int sent;
  } client_status ;

int handle_first_shake(client_status* client);
void handle_second_shake(client_status* client);

int main(int argc, char *argv[]) {
  char* host_addr = "127.0.0.1";
  int port = atoi(argv[1]);

  /*setup passive open*/
  int s;
  int opt =1;
  if((s = socket(PF_INET, SOCK_STREAM, 0)) <0) {
    perror("simplex-talk: socket");
    exit(1);
  }
   
   setsockopt(s, SOL_SOCKET, SO_REUSEADDR|SO_REUSEPORT, &opt, sizeof(opt));
   	   
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

	int fd_max = s;

//printf("line 61 check for fd_max: %d, check for s: %d\n", fd_max,s);
//create and initialize client-state array
  client_status* clientArray = malloc(MAX_PENDING* sizeof(*clientArray));
  for(int x =0; x < MAX_PENDING; x++){
  	  clientArray[x].status = -1;
  	  clientArray[x].fd = -1;
  	  clientArray[x].sent = -1;
  }

//set s to non-blocking using fcntl
  fcntl(s, F_SETFL, O_NONBLOCK);
  int clientArrayIndex = 0;
  
  fd_set all_fd_set;          //master set
  fd_set ready_set;
  FD_SET(s, &all_fd_set);

  	  
  /* wait for connection, then receive and print text */
  int new_s;
  socklen_t len = sizeof(sin);

  
  while(1) {
  	  FD_ZERO(&ready_set);       //initialize ready_set using FD_ZERO
  	  ready_set = all_fd_set;        // copy master_set to ready_set	    
 	  int readyfd_count = pselect(fd_max+1, &ready_set, NULL, NULL, NULL, NULL);          //ready_set now only contains fds that are ready for read 
  	  
  	  for (int i=0; i <= fd_max; i++){
  	  	if (FD_ISSET(i, &ready_set)){        //check if i is in ready_set
  	  		if(i==s){
  	 //new_s = (int*)calloc(1, sizeof(int));
    			if((new_s = accept(s, (struct sockaddr *)&sin, &len)) <0){
      				perror("simplex-talk: accept");
      				exit(1);
   				 }    
 			fcntl(new_s, F_SETFL, O_NONBLOCK);      //set new_s to non-blocking
 			FD_SET(new_s, &all_fd_set);               //add new_s to master_set
 			//add new_s to client_state_array
 			clientArray[clientArrayIndex].fd = new_s;
//printf("check on line 101 for clientArrayIndex : %d, clientArray[clientArrayIndex].fd: %d, new_s: %d\n ", clientArrayIndex , clientArray[clientArrayIndex].fd, new_s  );

 			clientArray[clientArrayIndex].status = 0;         //ready for first handshake
 			clientArrayIndex++;
 			  if(new_s > fd_max){
 			  	  fd_max = new_s;
              }
          }
          //else iterate through client status array for fd ==i
          
          for (int j=0; j < clientArrayIndex; j++){
//printf("check line 109 for valueclientArray[j].fd:  %d  value of j: %d value of i: %d\n ", clientArray[j].fd, j, i);
              if(clientArray[j].fd == i){
          	  	  //check the status of the file descriptor
          	  	  if(clientArray[j].status == 0){
                    int num_sent = handle_first_shake(&clientArray[j]);
//printf("line 117 check for j: %d\n", j)  ;   
//fhCt++;   
//printf("first handshake counts : %d\n", fhCt);         
                      clientArray[j].status = 1; 
                      clientArray[j].sent = num_sent;             
          	  	     }
          	  	  else if(clientArray[j].status == 1){
          	  	  	  handle_second_shake(&clientArray[j]);
//shCt++;
//printf("Second handshake counts : %d\n", shCt); 
          	  	  	  close(i);
          	  	  	  FD_CLR(i, &all_fd_set);       //remove i from master set
          	  	  	  clientArray[j].fd = -1;
          	  	  	  clientArray[j].status = -1;   //why? we need either 0 or 1
          	  	  	  clientArray[j].sent = -1;
          	  	     }
      		 }
      	  }
        }
        if(i==fd_max){
        	break;
        }
      }                //break the for loop and go to the while loop if i == fd_max
  	}                     //finish while loop
  return 0;
}

//thread behaviour 
int handle_first_shake(client_status* client){
	client_status entry = *client;
	//entry.status = 1;
	int new_s = entry.fd;
	char buf[MAX_LINE];
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
   int len = strlen(buf)+1;
   send(new_s, buf, len, 0);
   return num_send;
	
}
	
void handle_second_shake(client_status* client){	
    //receive, check and print hello x+2
    client_status entry = *client;
    int new_s = entry.fd;
	char buf[MAX_LINE];
	int num_rcvd;
  char hello[10];
    recv(new_s, buf, sizeof(buf), 0);
    sscanf(buf, "%s %d", hello, &num_rcvd);
    if ((num_rcvd - 1) != entry.sent){
      perror("ERROR ");
      close(new_s);
      exit(1);
    	}
    
    fputs(buf, stdout);
    fputc('\n', stdout);
    fflush(stdout);
    
    //close(new_s);
	//free(new_s);	
	//pthread_exit(NULL);
}
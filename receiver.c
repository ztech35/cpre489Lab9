#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "ccitt16.h"

#define DEBUG 1

#define PACKET_LENGTH 6
#define ACK_LENGTH    2
#define MAX_ADDRESS 100
#define MAX_NAME    100

/*
 * Connection to the server
 * parameters:
 * sock: the client socket
 * server_IP: the server IP address
 * return value:
 * 0 if success, -1 if failure
 */
int socket_connect(int sock, char server_IP[]){
  int port = 3212;
  struct sockaddr_in remote;

  // Setup remote connection
  remote.sin_addr.s_addr = inet_addr(server_IP); 
  remote.sin_family = PF_INET;
  remote.sin_port = htons(port);

  // Connection
  return connect(sock, (struct sockaddr *)&remote, sizeof(remote));
}

/*
 * Main function
 */
int main(int argc, char *argv[]){
  int sock;
  unsigned char packet[PACKET_LENGTH] = {0};
  char ack[ACK_LENGTH] = {0};
  char server_IP[MAX_ADDRESS] = {0};
  char new_file[MAX_NAME] = {0};
  FILE *file = NULL;
  int rec;


  // Arguments test
  if(argc != 3){
    fprintf(stderr, "2 arguments needed: server_IP new_filename\n");
    return EXIT_FAILURE;
  }else{
    strcpy(server_IP, argv[1]);
    strcpy(new_file, argv[2]);
  }
 
  // Creating the socket
  if((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0){
    perror("Socket creation failure");
    return EXIT_FAILURE;
  }

#if DEBUG
  fprintf(stdout, "DEBUG: Sucessfully created the socket\n");
#endif
 
  // Connection to remote server
  if(socket_connect(sock, server_IP) < 0){
    perror("Remote connection failure");
    return EXIT_FAILURE;
  }

#if DEBUG
  fprintf(stdout, "DEBUG: Sucessfully conected to the server\n");
#endif
  
  // Output file creation
  file = fopen(new_file, "w");
  if(file == NULL){
    fprintf(stdout, "I/O Error: Output file creation failure\n");
    return EXIT_FAILURE;
  }

#if DEBUG
  fprintf(stdout, "DEBUG: Sucessfully created the output file\n");
#endif
  
  // Receiving the data from the server
  while(1){
    rec = recv(sock, packet, PACKET_LENGTH , 0);

    // Error during transmission
    if(rec != PACKET_LENGTH){
      perror("Receiving data failure");
      return EXIT_FAILURE;
    }else{ // Transmission OK
      // CRC checking
      if(calculate_CCITT16(packet, PACKET_LENGTH, CHECK_CRC) == CRC_CHECK_SUCCESSFUL){
	sleep(1);

	// Sequence number test
	if((packet[0] | packet[1] << 8) < 0)
	  break;

	// ACK setting
	ack[0] = packet[0];
	ack[1] = packet[1];

#if DEBUG
	fprintf(stdout, "Packet received: sn=%d, d='%c%c'\n",
	  (packet[0] | packet[1] << 8), packet[2], packet[3]);
#endif

	// Writing to output file
	fprintf(file, "%c%c", packet[2], packet[3]);

	// Sending back ACK to server
	if(send(sock, ack, ACK_LENGTH, 0) < 0){
	  perror("Sending ACK failure");
	  return EXIT_FAILURE;
	}
      }
    }
  }

#if DEBUG
  fprintf(stdout, "DEBUG: Sucessfully received data file from remote server\n");
#endif

  // Output file closing
  if(fclose(file) < 0){
  perror("Closing output file failure");
  return EXIT_FAILURE;
}

#if DEBUG
  fprintf(stdout, "DEBUG: Sucessfully closed the output file\n");
#endif

  // Shutdown the connection
  if(shutdown(sock, SHUT_RDWR) < 0){
  perror("Shutdown connection failure");
  return EXIT_FAILURE;
}

#if DEBUG
  fprintf(stdout, "DEBUG: Sucessfully shutdown the connection\nExiting successfully\n");
#endif

  // Closing the socket
  if(close(sock) < 0){
  perror("Closing socket failure");
  return EXIT_FAILURE;
}

  return EXIT_SUCCESS;
}

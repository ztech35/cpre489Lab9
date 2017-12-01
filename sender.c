#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include
#include "AddCongestion.h"
#incldue "ccitt16.h"

#define DEBUG 0
#define MAX_ADDRESS 100
#define PACKET_SIZE 6

int socket_connect(int sock, char server_IP[])
{
    int ServerPort = 3212; // University ID as indicated in lab description
    struct sockaddr_in remote;

    // Setup remote connection
    remote.sin_addr.s_addr = inet_addr(server_IP); 
    remote.sin_family = PF_INET;
    remote.sin_port = htons(ServerPort);

    // Connection
    return connect(sock, (struct sockaddr *)&remote, sizeof(remote));
}
//senderprogram
int main(int argc, char *argv[])
{
  int clisock;
  int len = sizeof(remote_addr);
  char IP_ADDRESS[MAX_ADDRESS];//input arg
  char filename[1024];//input arg
  char input[1024];
  FILE *input_file;
  char *packet;
  double BER;//input arg
  short int crc;
  

  //check for correct arguments
  if(argc == 4)
    {
      strcpy(IP_ADDRESS, argv[1]);
      strcpy(filename, argv[2]);
      BER = atoi(argv[3]);
    }
  else
    {
      fprintf(stderr, "./sender <IP_ADDRESS> <filname> <BER>\n");
      return EXIT_FAILURE;
    }

  //read the input file
  input_file = fopen(filename, "r");
  fgets(input, 1024, (FILE*)input_file);
  fclose(input_file);
  //add file not found logic
#if DEBUG
  fprintf(stdout, "DEBUG: contents of input: %s\n", input);
#endif

  // Creating the socket
  if((clisock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
      perror("Socket creation failure");
      return EXIT_FAILURE;
    }
#if DEBUG
    fprintf(stdout, "DEBUG: Sucessfully created the socket\n");
#endif

  // Connection to remote server
  if(socket_connect(sock, server_IP) < 0)
    {
      perror("Remote connection failure");
      return EXIT_FAILURE;
    }
#if DEBUG
    fprintf(stdout, "DEBUG: Sucessfully conected to the server\n");
#endif

    crc = calculate_CCITT16(input,2,GENERATE_CRC);//only need to generate the crc once

    while(1)//loop over the entire input string
      {
	//create packets
	

	//add congestion
	AddCongestion(packet, BER);

	//RTO timer and 

	//Start communication between server and client
	
	  //send data

	  //recieve ack data

	
	//record cwnd
      }

}//end of main

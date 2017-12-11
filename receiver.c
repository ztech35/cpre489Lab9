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

int socket_connect(int sock, char server_IP[])
{
    int port = 3212;
    struct sockaddr_in remote;

    // Setup remote connection
    remote.sin_addr.s_addr = inet_addr(server_IP); 
    remote.sin_family = PF_INET;
    remote.sin_port = htons(port);

    // Connection
    return connect(sock, (struct sockaddr *)&remote, sizeof(remote));
}

void create_socket(int *sock, char *server_IP)
{
    // Creating the socket
    if((*sock = socket(PF_INET, SOCK_STREAM, 0)) < 0){
        perror("Socket creation failure");
        exit(EXIT_FAILURE);
    }

#if DEBUG
    fprintf(stdout, "DEBUG: Sucessfully created the socket\n");
#endif
 
    // Connection to remote server
    if(socket_connect(*sock, server_IP) < 0){
        perror("Remote connection failure");
        exit(EXIT_FAILURE);
    }

#if DEBUG
    fprintf(stdout, "DEBUG: Sucessfully connected to the server\n");
#endif
}

// Main program
int main(int argc, char *argv[]){
    int sock;
    unsigned char packet[PACKET_LENGTH] = {0};
    char server_IP[MAX_ADDRESS] = {0};
    unsigned char ack[ACK_LENGTH] = {0};
    int rec;
    int sn, highest_sn = -1, expected = -1;


    // Arguments test
    if(argc != 3){
        fprintf(stderr, "1 argument needed: sender_IP\n");
        return EXIT_FAILURE;
    }else{
        strcpy(server_IP, argv[1]);
    }

    create_socket(&sock, server_IP);
  
    // Receiving the data from the server
    while(1){
        rec = recv(sock, packet, PACKET_LENGTH , 0);

#if DEBUG
        if((packet[1] | packet[0] << 8) <= 0)
            printf("receiving end packet\n");
        else
            printf("receiving %d: %d %d %d %d ('%c' '%c')\n", (packet[1] | packet[0] << 8), packet[0],
                   packet[1], packet[2], packet[3], packet[2], packet[3]);
#endif

        // Error during transmission
        if(rec == 0){
            break;
        }else if(rec != PACKET_LENGTH){
            perror("Receiving data failure");
            return EXIT_FAILURE;
            
        }else{ // Transmission OK
            // CRC checking
            if(calculate_CCITT16(packet, PACKET_LENGTH, CHECK_CRC) == CRC_CHECK_SUCCESSFUL){
                sleep(1);

                sn = (packet[1] | packet[0] << 8);

                if(sn > highest_sn)
                    highest_sn = sn;

                // Sequence number test
                if(sn <= 0){
                    ack[0] = 0;
                    ack[1] = 0;
                }else if(sn == expected || expected == -1){
                    ack[0] = (highest_sn + 1) >> 8;
                    ack[1] = (highest_sn + 1) & 0xff;

                    expected = highest_sn + 1;
                }else{
                    ack[0] = expected >> 8;
                    ack[1] = expected & 0xff;
                }

                printf("ACK: %d\n", (ack[1] | ack[0] << 8));
                // Sending back ACK to server
                if(send(sock, ack, ACK_LENGTH, 0) < 0){
                    perror("Sending ACK failure");
                    return EXIT_FAILURE;
                }

                if(sn <= 0)
                    break;
            }
        }
    }

#if DEBUG
    fprintf(stdout, "DEBUG: Sucessfully received data file from remote server\n");
#endif

    // Closing the socket
    if(close(sock) < 0){
        perror("Closing socket failure");
        return EXIT_FAILURE;
    }

#if DEBUG
    fprintf(stdout, "DEBUG: Sucessfully closed the socket\nExiting successfully\n");
#endif

    return EXIT_SUCCESS;
}

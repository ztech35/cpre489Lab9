#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>

#include "AddCongestion.h"
#include "ccitt16.h"

#define DEBUG 1
#define BUFFER_SIZE 1024
#define MAX_ADDRESS 100
#define MAX_LENGTH 100
#define MAX_PACKETS 10000
#define PACKET_SIZE 6
#define ACK_SIZE 2
#define TIMEOUT 3.0

enum {SS, CA};

int bind_created_socket(int sock)
{

    int client_port = 3212;
    struct sockaddr_in  remote;

    // Setup connection
    remote.sin_family = PF_INET;
    remote.sin_addr.s_addr = htonl(INADDR_ANY);
    remote.sin_port = htons(client_port);

    // Binding
    return bind(sock, (struct sockaddr *)&remote, sizeof(remote));
}

void open_file(char *filename, char *buffer)
{
    FILE *file;
    
    //read the input file
    file = fopen(filename, "r");
    if(file == NULL){
        fprintf(stderr, "I/O Error: Output file creation failure\n");
        exit(EXIT_FAILURE);
    }

#if DEBUG
    fprintf(stdout, "DEBUG: Sucessfully opened the input file\n");
#endif

    fgets(buffer, BUFFER_SIZE, file);

    // Output file closing
    if(fclose(file) < 0){
        perror("Closing input file failure");
        exit(EXIT_FAILURE);
    }

#if DEBUG
    fprintf(stdout, "DEBUG: Sucessfully closed the input file\n");
#endif
}

void create_socket(int *sendsock)
{
    // Creating the socket
    if((*sendsock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation failure");
        exit(EXIT_FAILURE);
    }
    
#if DEBUG
    fprintf(stdout, "DEBUG: Sucessfully created the socket\n");
#endif

    // Binding
    if(bind_created_socket(*sendsock) < 0)
    {
        perror("Socket binding failure");
        exit(EXIT_FAILURE);
    }

#if DEBUG
    fprintf(stdout, "DEBUG: Sucessfully bound the socket\n");
#endif

    // Listening
    if(listen(*sendsock, 10) < 0)
    {
        perror("Socket listening failure");
        exit(EXIT_FAILURE);
    }

#if DEBUG
    fprintf(stdout, "DEBUG: Sucessfully listening for connections\n");
#endif
}

void end_connection(int sendsock, int recsock)
{
    // Closing the socket
    if(close(recsock) < 0){
        perror("Closing connection socket failure");
        exit(EXIT_FAILURE);
    }

#if DEBUG
    fprintf(stdout, "DEBUG: Sucessfully closed the connection socket\n");
#endif

    // Closing the socket
    if(close(sendsock) < 0){
        perror("Closing socket failure");
        exit(EXIT_FAILURE);
    }

#if DEBUG
    fprintf(stdout, "DEBUG: Sucessfully closed the socket\nExiting successfully\n");
#endif
}

void send_packet(int recsock, int sequence_num, unsigned char *packet, char *buffer,
                 double BER, int *end)
{
    int tmp;
    short int crc;

    packet[0] = sequence_num >> 8;
    packet[1] = sequence_num & 0xff;

    //data from file
    if(sequence_num >= 1000){
        tmp = (sequence_num - 1000) * 2;
        packet[2] = buffer[tmp];
        packet[3] = buffer[tmp + 1];

        if(packet[2] == 0 || packet[3] == 0)
            *end = 1;

        //crc
        crc = calculate_CCITT16(packet, PACKET_SIZE - 2, GENERATE_CRC);
        packet[4] = crc >> 8;
        packet[5] = crc & 0xff;

        //null terminated
        packet[6] = '\0';
        
        //adding congestion
        AddCongestion((char *)packet, BER);
    }

    printf("sending %d: %d %d %d %d ('%c' '%c')\n", sequence_num, packet[0], packet[1],
           packet[2], packet[3], packet[2], packet[3]);
	
    //send data
    if(send(recsock, packet, PACKET_SIZE, 0) < 0){
        perror("Sending packet failure");
        exit(EXIT_FAILURE);
    }
}

//Sender program
int main(int argc, char *argv[])
{
    int sendsock, recsock;
    struct sockaddr_in client;
    socklen_t client_len;
    
    char address[MAX_ADDRESS]; //input arg
    char filename[MAX_LENGTH]; //input arg
    double BER; //input arg

    char buffer[BUFFER_SIZE];
    
    unsigned char packet[PACKET_SIZE + 1];
    unsigned char ack[ACK_SIZE];
    int acks[MAX_PACKETS];;
    
    int sequence_num = 1000;
    int rec;
    double time_1 = 0.0, time_2;
    int i;
    
    int count = 0;
    double cwnd = 1.0;
    int left_bound_cwnd = sequence_num;
    double ssthresh = 16.0;
    int state = SS;
    int end = 0;
    unsigned int new_ack = 0;

    srand(0);
  
    //check for correct arguments
    if(argc == 4)
    {
        strcpy(address, argv[1]);
        strcpy(filename, argv[2]);
        BER = atof(argv[3]);
    }
    else
    {
        fprintf(stderr, "%s <client_IP> <filename> <BER>\n", argv[0]);
        return EXIT_FAILURE;
    }

    open_file(filename, buffer);

    create_socket(&sendsock);
 
    // Accepting connections from the receiver
    client_len = sizeof(client);
    if((recsock = accept(sendsock, (struct sockaddr *)&client, &client_len)) < 0)
    {
        perror("Accepting connection failure");
        return EXIT_FAILURE;
    }

#if DEBUG
    fprintf(stdout, "DEBUG: Successfully accepted connection\n");
#endif

    for(i = 0; i < MAX_PACKETS; i++)
        acks[i] = -1;

    int print = 0;

    while(end != 2) //loop over the entire input string
    {        
        if(end)
            sequence_num = 0;

        if(!print){
            printf("sn: %d, left_bound: %d, cwnd: %d\n", sequence_num, left_bound_cwnd,(int) cwnd);
            print = 1;
        }

        if(end != 2
           && acks[sequence_num - 1000] < 0
           && sequence_num <= left_bound_cwnd + cwnd - 1){
            time_1 = clock() * 1000 / CLOCKS_PER_SEC;
            send_packet(recsock, sequence_num, packet, buffer, BER, &end);
            acks[sequence_num - 1000] = 0;
            sequence_num++;
        }

        rec = recv(recsock, ack, ACK_SIZE, MSG_DONTWAIT);
        time_2 = clock() * 1000 / CLOCKS_PER_SEC;

        if(rec == 0) //error
        {
            end = 2;
            break;
        }
        else if(rec == ACK_SIZE) //ack
        {
            left_bound_cwnd = sequence_num;
            print = 0;
            
            time_2 = 0;
            
            new_ack = (ack[1] | ack[0] << 8);
                
            printf("ACK: %d\n", new_ack);
            new_ack--;
                
            acks[new_ack - 1000]++;

            if(new_ack <= 0){
                end = 2;
                break;
            }

            if(acks[new_ack - 1000] < 3) // non dup ack
            {   
                if(state == SS)
                {
                    cwnd += 1.0;

                    if(cwnd >= ssthresh)
                        state = CA;
                }
                else if(state == CA)
                {
                    cwnd += 1 / (floor(cwnd));
                }
            }
            else //dup ack
            {
                printf("3rd dup ack %d!\n", new_ack + 1);
                
                //retransmit
                sequence_num = new_ack + 1;
                acks[sequence_num - 1000] = -1;
                acks[new_ack - 1000] = -1;

                state = SS;
                ssthresh = cwnd / 2;
                cwnd = 1.0;
            }

            if(time_2 >= time_1 + TIMEOUT * 1000)
            {
                printf("timeout!\n");

                acks[sequence_num - 1000] = -1;

                state = SS;
                ssthresh = cwnd / 2;
                cwnd = 1.0;

            }
        }

        count++;
    }

    end_connection(sendsock, recsock);

    return EXIT_SUCCESS;
} //end of main

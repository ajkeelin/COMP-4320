/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>

#include <arpa/inet.h>

#define PORT "10010" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes;  
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	char *hello = "Client is connected"; 
    char buffer[1024] = {0}; 
    int valread;

    
	if (argc != 2) {
	    fprintf(stderr,"usage: client hostname\n");
	    exit(1);
	}


	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	//printf("client: received '%s'\n",buf);

    clock_t start_t, end_t, total_t;
    start_t = clock();


    //Read opcode request from server
    memset(buffer, 0, sizeof(buffer));
    valread = read( sockfd , buffer, 1024);
    printf("%s\n", buffer);

    //Input and send opcode
    unsigned int opcode;
    scanf("%X", &opcode);
    send(sockfd , &opcode , sizeof(opcode) , 0);


    //Read num of operands request
    memset(buffer, 0, sizeof(buffer));
    valread = read (sockfd , buffer, 1024);
    printf("%s\n", buffer);

    //Input and send num of operands
    unsigned int numops;
    scanf("%X", &numops);
    send(sockfd, &numops , sizeof(numops) , 0);


    //Read first operand request
    memset(buffer, 0, sizeof(buffer));
    valread = read ( sockfd , buffer, 1024);
    printf("%s\n", buffer);

    //Input and send first operand
    unsigned int op1;
    scanf("%X", &op1);
    send(sockfd, &op1 , sizeof(op1) , 0);

    
    //Read second operand, if necessary
    unsigned int op2;
    if(numops == 2) {
        memset(buffer, 0, sizeof(buffer));
        valread = read (sockfd , buffer, 1024);
        printf("%s\n", buffer);

        //Input and send second operand
        scanf("%X", &op2);
        send(sockfd, &op2 , sizeof(op2) , 0);
    }


    //Read results from server
    printf("\nHere are the results of the operation\n");

    //Read TML
    //unsigned int tml = 0x00;
    memset(buffer, 0, sizeof(buffer));
    //printf("TML: ");
    valread = read( sockfd , buffer, 1024);
    int tml = 0x00;
    sscanf(buffer, "%X", &tml);
    printf("TML: ");
    printf("%02X\n", tml);

    //valread = read( sockfd , &tml, sizeof(tml));
    //printf("%02X\n", tml);


    //Read Req ID
    unsigned int res_id = 0x00;
    printf("Req ID: ");
    valread = read( sockfd , &res_id, sizeof(res_id));
    printf("%02X\n", res_id);

    //Read Error Code
    unsigned int res_error = 0x000;
    printf("Error Code: ");
    valread = read( sockfd , &res_error, sizeof(res_error));
    printf("%03X\n", res_error);

    //Read Result
    unsigned int res_result = 0x00000000;
    printf("Result: ");
    valread = read( sockfd, &res_result, sizeof(res_result));
    printf("%08X\n", res_result);


    end_t = clock();
    total_t = (double)(end_t - start_t);
    printf("Time: %f\n", total_t);


	close(sockfd);

	return 0;
}


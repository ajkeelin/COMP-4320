
// Client side implementation of UDP client-server model 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <errno.h>
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <netdb.h>
#include <time.h>
  
#define MYPORT     10010
#define MAXLINE 1024 
  
// Driver code 
int main(int argc, char *argv[]) { 
    int sockfd; 
    char buffer[MAXLINE]; 
    char *hello = "Hello from client"; 
    struct sockaddr_in     servaddr; 
    struct addrinfo hints, *servinfo, *p;
    int rv;

    /*
    //New
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(argv[1], MYPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    //End of new
    */
  
    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
  
    memset(&servaddr, 0, sizeof(servaddr)); 
      
    // Filling server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(MYPORT); 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
      
    int n, len; 

    sendto(sockfd, (const char *)hello, strlen(hello), 
        MSG_CONFIRM, (const struct sockaddr *) &servaddr,  
            sizeof(servaddr)); 
    //printf("Hello message sent.\n"); 
          
    n = recvfrom(sockfd, (char *)buffer, MAXLINE,  
                MSG_WAITALL, (struct sockaddr *) &servaddr, 
                &len); 
    buffer[n] = '\0'; 
    //printf("Server : %s\n", buffer); 
    printf("Connected to server\n");

    //Using the clock freezes the client
    /*
    clock_t start_t, end_t, total_t;
    start_t = clock();
    */


    //Receive Opcode request
    memset(buffer, 0, sizeof(buffer));
    n = recvfrom (sockfd, (char *)buffer, MAXLINE, 
        MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
    printf("%s\n", buffer);

    //Input and send Opcode
    unsigned int opcode;
    scanf("%X", &opcode);
    sendto(sockfd, &opcode, sizeof(opcode), 0, 
        (const struct sockaddr *) &servaddr, sizeof(servaddr));


    //Receive num of operands request
    memset(buffer, 0, sizeof(buffer));
    n = recvfrom (sockfd, (char *)buffer, MAXLINE, 
        MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
    printf("%s\n", buffer);

    //Input and send num of operands
    unsigned int numops;
    scanf("%X", &numops);
    sendto(sockfd, &numops, sizeof(numops), 0, 
        (const struct sockaddr *) &servaddr, sizeof(servaddr));


    //Receive first operand request
    memset(buffer, 0, sizeof(buffer));
    n = recvfrom (sockfd, (char *)buffer, MAXLINE, 
        MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
    printf("%s\n", buffer);

    //Input and send first operand
    unsigned int op1;
    scanf("%X", &op1);
    sendto(sockfd, &op1, sizeof(op1), 0, 
        (const struct sockaddr *) &servaddr, sizeof(servaddr));


    //Receive second operand request, if necessary
    unsigned int op2;
    if (numops == 2) {
        memset(buffer, 0, sizeof(buffer));
        n = recvfrom (sockfd, (char *)buffer, MAXLINE, 
            MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
        printf("%s\n", buffer);

        //Input and send second operand
        scanf("%X", &op2);
        sendto(sockfd, &op2, sizeof(op2), 0, 
            (const struct sockaddr *) &servaddr, sizeof(servaddr));
    }


    //Read results from server
    printf("\nHere are the results of the operation\n");

    //Read TML
    unsigned int tml = 0x00;
    printf("TML: ");
    n = recvfrom(sockfd, &tml, MAXLINE, 
        MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
    printf("%02X\n", tml);


    //Read Req ID
    unsigned int res_id = 0x00;
    printf("Req ID: ");
    n = recvfrom(sockfd, &res_id, MAXLINE, 
        MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
    printf("%02X\n", res_id);

    //Read Error Code
    unsigned int res_error = 0x000;
    printf("Error Code: ");
    n = recvfrom(sockfd, &res_error, MAXLINE, 
        MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
    printf("%03X\n", res_error);

    //Read Result
    unsigned int res_result = 0x00000000;
    printf("Result: ");
    n = recvfrom(sockfd, &res_result, MAXLINE, 
        MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
    printf("%08X\n", res_result);

    //Using the clock freezes the client
    /*
    end_t = clock();
    total_t = (double)(end_t - start_t);
    printf("Time: %f\n", total_t);
    */
  
    close(sockfd); 
    return 0; 
} 

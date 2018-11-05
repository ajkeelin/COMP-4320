
// Server side implementation of UDP client-server model 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
  
#define MYPORT     10010
#define MAXLINE 1024 
  
// Driver code 
int main() { 
    int sockfd; 
    char buffer[MAXLINE]; 
    char *hello = "Hello from server"; 
    struct sockaddr_in servaddr, cliaddr; 
    unsigned int req_id = 0x00;
      
    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
      
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
      
    // Filling server information 
    servaddr.sin_family    = AF_INET; // IPv4 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(MYPORT); 
      
    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
      
    int len, n;

    n = recvfrom(sockfd, (char *)buffer, MAXLINE,  
                MSG_WAITALL, ( struct sockaddr *) &cliaddr, 
                &len); 
    buffer[n] = '\0'; 
    //printf("Client : %s\n", buffer); 
    sendto(sockfd, (const char *)hello, strlen(hello),  
        MSG_CONFIRM, (const struct sockaddr *) &cliaddr, 
            len); 
    //printf("Hello message sent.\n"); 
    printf("Connected to client\n");


    /*
    //Send TML request
    char *req_tml = "Please enter the TML:";
    sendto(sockfd, (const char *)req_tml, strlen(req_tml), 
        MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
    printf("Request for TML sent\nClient returned: ");

    //Receive TML from client
    unsigned int tml = 0x00;
    n = recvfrom(sockfd, &tml, MAXLINE, 
        MSG_WAITALL, (struct sockaddr *) &cliaddr, &len);
    printf("%X\n", tml);
    */


    //Send Opcode request
    char *req_opcode = "Please enter the Opcode:";
    sendto(sockfd, (const char *)req_opcode, strlen(req_opcode), 
        MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
    printf("Request for Opcode sent\nClient returned: ");

    //Receive Opcode from client
    unsigned int opcode = 0x00;
    n = recvfrom(sockfd, &opcode, MAXLINE, 
        MSG_WAITALL, (struct sockaddr *) &cliaddr, &len);
    printf("%X\n", opcode);


    //Send number of operands request
    char *req_numops = "Please enter the number of operands:";
    sendto(sockfd, (const char *)req_numops, strlen(req_numops), 
        MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
    printf("Request for number of operands sent\nClient returned: ");

    //Receive the number of operands from client
    unsigned int numops = 0x00;
    n = recvfrom(sockfd, &numops, MAXLINE, 
        MSG_WAITALL, (struct sockaddr *) &cliaddr, &len);
    printf("%X\n", numops);


    //TML
    unsigned int tml = 0x00;
    if (numops == 2) {
        tml = 0x08;
    }
    else {
        tml = 0x06;
    }


    //Send first operand request
    char *req_op1 = "Please enter the first operand:";
    sendto(sockfd, (const char *)req_op1, strlen(req_op1), 
        MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
    printf("Request for the first operand sent\nClient returned: ");

    //Receive first operand from client
    unsigned int op1 = 0x0000;
    n = recvfrom(sockfd, &op1, MAXLINE, 
        MSG_WAITALL, (struct sockaddr *) &cliaddr, &len);
    printf("%X\n", op1);

    //Send second operand request, if necessary
    char *req_op2 = "Please enter the second operand:";
    unsigned int op2 = 0x0000;
    if (numops == 2) {
        sendto(sockfd, (const char *)req_op2, strlen(req_op2), 
            MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
        printf("Request for second operand sent\nClient returned: ");

        n = recvfrom(sockfd, &op2, MAXLINE, 
            MSG_WAITALL, (struct sockaddr *) &cliaddr, &len);
        printf("%X\n", op2);

    }

    unsigned int result;
    unsigned int error_code = 0x127;

    if(opcode == 0 && numops == 2) {
        result = op1 + op2;
        error_code = 0x000;
    }

    else if(opcode == 1 && numops == 2) {
        result = op1 - op2;
        error_code = 0x000;
    }

    else if(opcode == 2 && numops == 2) {
        result = op1 | op2;
        error_code = 0x000;
    }

    else if(opcode == 3 && numops == 2) {
        result = op1 & op2;
        error_code = 0x000;
    }

    else if(opcode == 4 && numops == 2) {
        result = op1 >> op2;
        error_code = 0x000; 
    }

    else if(opcode == 5 && numops == 2) {
        result = op1 << op2;
        error_code = 0x000;
    }

    else if(opcode == 6 && numops == 1) {
        result = ~op1;
        error_code = 0x000;
    }


    //Send results back to client
    printf("Sending results to client\n");
        
    //TML
    unsigned int res_tml = 0x07;
    sendto(sockfd, &res_tml, sizeof(res_tml), 0, 
        (const struct sockaddr *) &cliaddr, sizeof(cliaddr));

    //Request ID
    req_id++;
    sendto(sockfd, &req_id, sizeof(req_id), 0, 
        (const struct sockaddr *) &cliaddr, sizeof(cliaddr));

    //Error Code
    sendto(sockfd, &error_code, sizeof(error_code), 0, 
        (const struct sockaddr *) &cliaddr, sizeof(cliaddr));

    //Results
    sendto(sockfd, &result, sizeof(result), 0, 
        (const struct sockaddr *) &cliaddr, sizeof(cliaddr));


      
    return 0; 
} 

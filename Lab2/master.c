#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <signal.h>
#include <arpa/inet.h>

#define BACKLOG 10
#define MAXDATASIZE 100
#define MASTER_IP 0x0100007f

const uint8_t MASTER_GID = 30; //Don't have a group number, so have just been using 30 for testing
const uint32_t MAGIC_NUMBER = 0x4A6F7921;

struct join_request
{
 uint8_t gid;
 int magic_number;
} __attribute__((__packed__));

struct master_response {
 uint8_t gid;
 uint32_t magic_number;
 uint8_t nextRID;
 uint32_t nextSlaveIP;
}__attribute__((__packed__));

struct Slave {
 uint8_t GID;
 uint32_t IP;
 uint8_t RID;
 uint32_t nextSlaveIP;
 struct Slave *next;		
};

struct Master {
 uint8_t GID;
 uint32_t IP;
 uint8_t myRID;
 uint8_t nextRID;
 uint32_t nextSlaveIP;
 struct Slave *next;
};

void sigchld_handler(int s)
{
	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

//Add new Slave to the ring
void newSlave(struct Master* master, struct Slave* slave) {
	slave->RID = master->nextRID;
	slave->nextSlaveIP = master->next->IP;
	slave->next = master->next;
	master->nextSlaveIP = slave->IP;
	master->next = slave;
	master->nextRID += 1;
}

int main(int argc, char *argv[])
{
	//Variables from beej template
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv, listener;

	//Added variables for ring
	char *portNumber;
	char nextSlaveIP[15];
	struct join_request req; //request
	struct master_response res; //response

	//Initialize the master
	struct Master* master = malloc(sizeof(struct Master));
	master->GID = MASTER_GID;
	master->IP = MASTER_IP;
	master->myRID = 0;
	master->nextRID = 1;
	master->next = master;
	master->nextSlaveIP = master->next->IP;

	//Error for incorrect command line argument
	if (argc != 2) {
	    fprintf(stderr,"Command line argument: master masterPortNumber\n");
	    exit(1);
	}
	//Set port number from arguments
	portNumber = argv[1];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, portNumber, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("Waiting for connections...\n");
	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("----------------------------------------------------\n");
		printf("Got connection from %s\n", s);

		//Listener is a throwaway var, just used for req to get the join request
		listener = recv(new_fd, &req, MAXDATASIZE-1, 0);

		req.magic_number = ntohl(req.magic_number);

		//Check the message is the correct size
		if(sizeof(req) != 5) {
			perror("The message is not 5 bytes\n");
			exit(1);
		}
		//Check the magic number is correct
		if(req.magic_number != MAGIC_NUMBER) {
			perror("The magic number is not correct\n");
			exit(1);
		}

		//Print message received from client(slave)
		printf("\nSlave Join Request:\n");
		printf("Magic Number: %#04x\n", req.magic_number);
		printf("GID: %d\n\n", req.gid);

		//Construct the slave node with the info from the message
		struct Slave* slave = malloc(sizeof(struct Slave));
		struct sockaddr_in *get_ip = (struct sockaddr_in *)&their_addr;
		slave->GID = req.gid;
		slave->IP = get_ip->sin_addr.s_addr;

		//Add the slave node to the ring
		newSlave(master, slave);
		printf("Request accepted. A new Slave has been added to the ring\n");
		printf("Formulating and sending response\n\n");

		//Formluate the response which will be sent to the slave
		res.gid = master->GID;
		res.magic_number = MAGIC_NUMBER;
		res.nextRID = master->next->RID;
		res.nextSlaveIP = slave->nextSlaveIP;

		//Print the response values in decimal
		printf("Response values:\n");
		printf("Master GID: %d\n", res.gid);
		printf("Master Magic Number: %#04x\n", res.magic_number);
		printf("RID: %d\n", res.nextRID);
		printf("IP: %s\n", s); //From lines 181 - 185

		//Print the response values in hex
		printf("\nResponse in Hex:\n");
		printf("%#04x\\", res.gid);
		printf("%#04x\\", res.magic_number);
		printf("%#04x\\", res.nextRID);
		printf("%#04x\\", res.nextSlaveIP);
		printf("\n\n");

		if (send(new_fd, &res, sizeof(res), 0) == -1)
		{
			perror("send");
			exit(1);
		}

		printf("Response sent\n");
		printf("----------------------------------------------------\n\n");
		printf("Waiting for next Slave join request...\n\n");
	}
	close(sockfd);  // parent doesn't need this

	return 0;
}
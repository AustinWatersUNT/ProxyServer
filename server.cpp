
	/* Multiple simultaneous clients handled by threads; */
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>	/* system type defintions */
#include <sys/socket.h>	/* network system functions */
#include <netinet/in.h>	/* protocol & struct definitions */
#include <arpa/inet.h>
#include <iostream>

#define BACKLOG	5
#define BUF_SIZE	1024
#define LISTEN_PORT	10000

int threadCount = BACKLOG;
void *client_handler(void *arg);

using namespace std;

int main(int argc, char *argv[]){
    int status, *sock_tmp;
    pthread_t a_thread;
    void *thread_result;

    struct sockaddr_in addr_mine;
    struct sockaddr_in addr_remote;
    int sock_listen;
    int sock_aClient;
    int addr_size;
    int reuseaddr = 1;


    sock_listen = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_listen < 0) {
        perror("socket() failed");
        exit(0);
    }

    memset(&addr_mine, 0, sizeof (addr_mine));
    addr_mine.sin_family = AF_INET;
    addr_mine.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_mine.sin_port = htons((unsigned short)LISTEN_PORT);

    // /* Enable the socket to reuse the address */
    // if (setsockopt(sock_listen, SOL_SOCKET, SO_REUSEADDR, &reuseaddr,
    // 	sizeof(int)) == -1) {
    //     perror("setsockopt");
    // 	close(sock_listen);
    //     exit(1);
    // }

    status = bind(sock_listen, (struct sockaddr *) &addr_mine,
    	sizeof (addr_mine));
    if (status < 0) {
        perror("bind() failed");
        close(sock_listen);
        exit(1);
    }

    status = listen(sock_listen, 5);
    if (status < 0) {
        perror("listen() failed");
        close(sock_listen);
        exit(1);
    }

    addr_size = sizeof(struct sockaddr_in);
    printf("waiting for a client\n");
    while(1) {
    	if (threadCount < 1) {
    		sleep(1);
    	}

    	sock_aClient = accept(sock_listen, (struct sockaddr *) &addr_remote, (socklen_t*) &addr_size);
    	if (sock_aClient == -1){
    		close(sock_listen);
        	exit(1);
    	}

	cout << "Got a connection from " << inet_ntoa(addr_remote.sin_addr) << " on port " << htons(addr_remote.sin_port) << endl;

    	sock_tmp = (int*) malloc(1);
    	*sock_tmp = sock_aClient;
    	printf("thread count = %d\n", threadCount);
    	threadCount--;
 		status = pthread_create(&a_thread, NULL, client_handler,
            (void *) sock_tmp);
 		if (status != 0) {
 			perror("Thread creation failed");
 			close(sock_listen);
 			close(sock_aClient);
 			free(sock_tmp);
        	exit(1);
 		}
    }

    return 0;
}

void *client_handler(void *sock_desc) {
	int msg_size;
	int send_len;
	int bytes_sent;
	char buf[BUF_SIZE];
	int sock = *(int*)sock_desc;

	printf("In client_handler\n");
	while ((msg_size = recv(sock, buf, BUF_SIZE, 0)) > 0) {
        buf[msg_size] = 0;
	
	printf("Recieved: %s\n", buf);
	
	strcpy(buf, "GET /cse.unt.edu HTTP/1.1");
	send_len=strlen(buf);
     	bytes_sent=send(sock,buf,send_len,0);
    }

    close(sock);
	free(sock_desc);
	threadCount++;
    // pthread_exit("Thank you for the CPU time");
}

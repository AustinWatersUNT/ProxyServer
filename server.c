/* Multiple simultaneous clients handled by threads; */
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>  /* system type defintions */
#include <sys/socket.h> /* network system functions */
#include <netinet/in.h> /* protocol & struct definitions */
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#define BACKLOG 	   5
#define BUF_SIZE	   1024
#define LISTEN_PORT 	30000
#define SERVER_PORT 	80

int threadCount = BACKLOG;
void *client_handler(void *arg);
int getIpAddress(char*, char*);
int destinationSock(char*);

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
   //   sizeof(int)) == -1) {
   // perror("setsockopt");
   //   close(sock_listen);
   // exit(1);
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

   sock_aClient = accept(sock_listen, (struct sockaddr *) &addr_remote,
   &addr_size);
   if (sock_aClient == -1){
      close(sock_listen);
      exit(1);
   }

   printf("Got a connection from %s on port %d\n",
   inet_ntoa(addr_remote.sin_addr),
   htons(addr_remote.sin_port));
   sock_tmp = malloc(1);
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
   char ip[100];
   int sock_send;
   int i;
   int send_len, bytes_sent;
   char buf[BUF_SIZE];
   int sock = *(int*)sock_desc;

   printf("In client_handler\n");

   while ((msg_size = recv(sock, buf, BUF_SIZE, 0)) > 0) { //This grabs a from the browser

      buf[msg_size] = 0;
      printf("Message:\n%s\n\n\n", buf); //This prints what the browser sends to server

      //The concated strings below is the what the command from the browser looks like
      //Except get includes the website and the host is the 129.120.151.94:portNumber
      //We want to change it below with were google is to be the webside name and where the 
      //    '/' would be everything that comes after .com so like google.com/drive the string
      // would be GET /drive /HTTP/1.1 and host would be http://www.google.com
     
      getIpAddress("google.com", ip); //retrives the ip address for website and saves it to ip
      printf("\nSending IP: %s\n", ip); //prints the ip
      sock_send = destinationSock(ip); //creates a socket to that ip and port 80

      strcpy(buf, "GET /?gws_rd=ssl HTTP/1.1\r\n");
      strcat(buf, "Host: www.google.com\r\n"); //want to replace google.com with any website
      strcat(buf, "Connection: keep-alive\r\n");
      strcat(buf, "Cache-Control: max-age=0\r\n");
      strcat(buf, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n");
      strcat(buf, "User-Agent: Mozilla/5.0 (Windows NT 6.3; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/42.0.2311.90 Safari/537.36\r\n");
      strcat(buf, "Accept-Encoding: gzip, deflate, sdch\r\n");
      strcat(buf, "Accept-Language: en-US,en;q=0.8\r\n\r\n");

      send_len=strlen(buf);
      bytes_sent=send(sock_send,buf,send_len,0);

      printf("\n\n");
       //this reads in the response 1024 characters and sends that 1024 characters to the browser
      //this is also where we would stream buf to a file as well for reading from cache
      while ((msg_size = recv(sock_send, buf, BUF_SIZE, 0)) > 0) {
         buf[msg_size] = 0;
         printf("%s", buf);
         send_len=strlen(buf);
         bytes_sent=send(sock,buf,send_len,0);
         strcpy(buf, "\0");
      }
      printf("\n\nOut of Loop\n");
      strcpy(buf, "");
      close(sock_send);
   }
   close(sock);
   free(sock_desc);
   threadCount++;
   // pthread_exit("Thank you for the CPU time");
}

int getIpAddress(char* hostname, char* ip)
{
   int sockfd;  
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_in *h;
    int rv;
 
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // use AF_INET6 to force IPv6
    hints.ai_socktype = SOCK_STREAM;
 
    if ( (rv = getaddrinfo( hostname , NULL , NULL , &servinfo)) != 0) 
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
 
    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) 
    {
        h = (struct sockaddr_in *) p->ai_addr;
        strcpy(ip , inet_ntoa( h->sin_addr ) );
    }
     
    freeaddrinfo(servinfo); // all done with this structure
    return 0;
}

int destinationSock(char* ip)
{
   int sock_send;
   struct sockaddr_in addr_send;
   int i;

   sock_send=socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);
   if (sock_send < 0) {
      printf("socket() failed\n");
      exit(0);
      return -1;
    }
    else {
      printf("socket created\n");
   }

   /* create socket address structure to connect to */
   memset(&addr_send, 0, sizeof (addr_send)); /* zero out structure */
   addr_send.sin_family = AF_INET; /* address family */
   addr_send.sin_addr.s_addr = inet_addr(ip);
   addr_send.sin_port = htons((unsigned short)SERVER_PORT);

   /* connect to the server */
   i=connect(sock_send, (struct sockaddr *) &addr_send, sizeof (addr_send));
   if (i < 0) {
      printf("connect() failed\n");
      close(sock_send);
      exit(0);
    }
    else {
      printf("connect successful\n");
   }
   return sock_send;
}

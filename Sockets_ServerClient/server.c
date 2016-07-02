#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h> 
#include <time.h> 

#define PORT 5555

char *strrev (char *A)
{
	int i, j;
	char temp;

	char *str = (char *) malloc (sizeof(char) * strlen(A));
	strcpy (str, A);

	i = 0;
	j = strlen(str) - 1;

	while (i < j) {
	temp = str[i];
	str[i] = str[j];
	str[j] = temp;
	i++;
	j--;
	}

	return str;
}

int main(int argc, char *argv[])
{
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 

    char sendBuff[500];
    
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr)); 
    memset(sendBuff, '0', sizeof(sendBuff)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT); 

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    listen(listenfd, 5); 
    printf("\nServer listening for connections!\n");
    
    time_t t_seconds;

    while(1)
    {
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
        printf("\nAccepted a connection\n");
        t_seconds = time(0);

        read (connfd, sendBuff, strlen(sendBuff));

        printf ("Message Received: %s\n", sendBuff);

        //snprintf(sendBuff, sizeof(sendBuff), "%s\n", ctime(&t_seconds));
        write(connfd, strrev (sendBuff), strlen(sendBuff)); 

        close(connfd);
        
     }
}

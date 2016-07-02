#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h> 
#include <sys/socket.h>

#define PORT 5555

int main (int argc, char *argv[])
{
	int client_fd;
	int rc;
	
	struct sockaddr_in server_addr;
	bzero (&server_addr, sizeof (server_addr));

	if (argc != 2)
	{
		printf ("ERROR:\n"
				"Usage: %s <server ip address>\n", argv[0]);
		return -1;
	}

	rc = client_fd = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (rc < 0)
	{
		printf ("ERROR: Unable to create a socket\n");
		return -1;
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons (PORT);
	rc = inet_pton (AF_INET, argv[1], &server_addr.sin_addr);
	if (rc < 0)
	{
		printf ("ERROR: The given IP Address is Invalid\n");
		return -1;
	}
	// bzero (&server_addr.sin_zero, sizeof(server_addr.sin_zero));

	rc = connect (client_fd, (struct sockaddr *)(&server_addr), sizeof (server_addr));
	if (rc < 0)
	{
		printf ("ERROR: Cannot connect to the server\n");
		return -1;
	}

	/**********************************************************************************/

	printf ("CONNECTION SUCCESSFUL\n\n");

	char buffer[1024];
	const char *exitString = "quit";

	printf ("Query: ");
	bzero (buffer, 1024);
	fgets (buffer, 1024, stdin);

	// if (strcmp (buffer, exitString) == 0)
		// break;

	strtok (buffer, "\r\n");

	rc = write (client_fd, buffer, strlen (buffer));
	if (rc < 0)
	{
		printf ("ERROR: An error occured while sending query to the server\n");
		// break;
	}

	bzero (buffer, 1024);
	rc = read (client_fd, buffer, 1024);
	if (rc < 0)
	{
		printf ("ERROR: An error occured while sending query to the server\n");
		// break;
	}

	printf ("Reply: %s\n", buffer);

	close (client_fd);
	return 0;
}
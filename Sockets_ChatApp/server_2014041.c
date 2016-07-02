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

typedef struct _queueNode
{
	pthread_t *thread;
	pthread_mutex_t cond_lock;
	pthread_cond_t cond;
	struct _queueNode *prev;
	struct _queueNode *next;
} mqNode;

typedef struct _queue
{
	mqNode *front, *rear;
	pthread_mutex_t queueLock;
} Queue;

Queue Q;

void initQ ()
{
	Q.front = NULL;
	Q.rear = NULL;
	pthread_mutex_init (&Q.queueLock, NULL);
}

void initNode (mqNode *node, pthread_t *thread)
{
	node -> thread = thread;
	pthread_mutex_init (&(node -> cond_lock), NULL);
	pthread_cond_init (&(node -> cond), NULL);
	node -> prev = NULL;
	node -> next = NULL;
}

int isQEmpty ()
{
	pthread_mutex_lock (&Q.queueLock);

	int flag;
	if (Q.front == NULL && Q.rear == NULL)
		flag = 1;
	else
		flag = 0;

	pthread_mutex_unlock (&Q.queueLock);

	return flag;
}

int addToQueue (mqNode *node)
{
	if (isQEmpty ())
	{
		pthread_mutex_lock (&Q.queueLock);
		Q.front = node;
		Q.rear = Q.front;
		pthread_mutex_unlock (&Q.queueLock);
		return 1;
	}
	else
	{
		pthread_mutex_lock (&Q.queueLock);
		Q.front -> prev = node;
		node -> next = Q.front;
		Q.front = node;
		pthread_mutex_unlock (&Q.queueLock);
		return 1;
	}
	return 0;
}

mqNode *removeFromQueue ()
{
	int flag;
	mqNode *node;

	if (isQEmpty ())
		return NULL;
	else
	{
		pthread_mutex_lock (&Q.queueLock);
		if (Q.front == Q.rear)
		{
			node = Q.rear;
			node -> next = NULL;
			node -> prev = NULL;

			Q.front = Q.rear = NULL;
		}
		else
		{
			node = Q.rear;

			Q.rear = node -> prev;
			Q.rear -> next = NULL;

			node -> prev = NULL;
		}
		pthread_mutex_unlock (&Q.queueLock);
	}

	return node;
}

struct connectorInput
{
	int serverfd;
	struct sockaddr_in *server_addr;
};

struct threadInput
{
	int connfd;
	pthread_t *thread;
	mqNode *selfNode;
};

void *customerCare (void *input)
{
	int connfd;
	int n;
	char buffer[1024];
	pthread_t *thread;
	mqNode *selfNode;

	struct threadInput *custIp = (struct threadInput *)(input);
	connfd = custIp -> connfd;
	thread = custIp -> thread;
	selfNode = custIp -> selfNode;

	free (input);
	custIp = NULL;

	bzero (buffer, 1024);
	n = read (connfd, buffer, 1024);
	if (n > 0)
	{
		strtok (buffer, "\r\n");
		printf ("clientfd:%d :: %s\n", connfd, buffer);
	}

	addToQueue (selfNode);

	pthread_mutex_lock (&(selfNode -> cond_lock));
	pthread_cond_wait (&(selfNode -> cond), &(selfNode -> cond_lock));
	pthread_mutex_unlock (&(selfNode -> cond_lock));

	bzero (buffer, 1024);
	// printf ("Reply to clientfd:%d :: ", connfd);
	fflush (stdin);
	fgets (buffer, 1024, stdin);
	if (strlen (buffer) > 0)
		strtok (buffer, "\r\n");
	n = write (connfd, buffer, strlen (buffer));
	if (n < 0)
	{
		printf ("ERROR: Cant write to clientfd:%d\n", connfd);
	}

	return NULL;
}

void *connector (void *input)
{
	int serverfd, connfd;
	int clientLenght, rc;
	struct sockaddr_in *server_addr, client_addr;
	pthread_t *thread;

	struct connectorInput *connIp = (struct connectorInput *)(input);
	serverfd = connIp -> serverfd;
	server_addr = connIp -> server_addr;

	struct threadInput *custIp;

	while (1)
	{
		clientLenght = sizeof (client_addr);
		connfd = accept (serverfd, (struct sockaddr *)(&client_addr), (&clientLenght));

		printf ("Accepted a connection __ fd:%d\n", connfd);

		thread = (pthread_t *) malloc (sizeof (pthread_t));

		mqNode *node = (mqNode *) malloc (sizeof (mqNode));
		initNode (node, thread);

		custIp = (struct threadInput *) malloc (sizeof (struct threadInput));
		custIp -> connfd = connfd;
		custIp -> thread = thread;
		custIp -> selfNode = node;

		rc = pthread_create (thread, NULL, customerCare, (void *)(custIp));
		if (rc < 0)
		{
			printf ("ERROR: Cannot create thread for connfd: %d\n", connfd);
			break;
		}
	}

	return NULL;
}

int main ()
{
	int serverfd, rc;
	pthread_t *thread;
	struct sockaddr_in server_addr;

	bzero (&server_addr, sizeof (server_addr));

	serverfd = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons (PORT);
	server_addr.sin_addr.s_addr = htonl (INADDR_ANY);
	// bzero (&server_addr.sin_zero, sizeof(server_addr.sin_zero));

	bind (serverfd, (struct sockaddr *)(&server_addr), sizeof (server_addr));

	listen (serverfd, 10);
	printf ("SERVER READY TO ACCEPT CONNECTIONS...\n");

	thread = (pthread_t *) malloc (sizeof (pthread_t));

	struct connectorInput connIp;
	connIp.serverfd = serverfd;
	connIp.server_addr = &server_addr;

	rc = pthread_create (thread, NULL, connector, (void *)(&connIp));
	if (rc < 0)
	{
		printf ("ERROR: Cant create connector thread\n");
		return -1;
	}

	while (1)
	{
		if (!isQEmpty ())
		{
			mqNode *node = removeFromQueue ();
			pthread_mutex_lock (&(node -> cond_lock));
			pthread_cond_signal (&(node -> cond));
			pthread_mutex_unlock (&(node -> cond_lock));
			free (node);
		}
	}

	return 0;
}
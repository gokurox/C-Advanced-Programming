#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _directoryNode
{
	char *dirID;
	struct _directoryNode *child;
	struct _directoryNode *sibling;
} dirNode;

void initDirectory (dirNode *theDirectory, char *dirID)
{
	theDirectory -> dirID = strdup(dirID);
	theDirectory -> child = NULL;
	theDirectory -> sibling = NULL;
}

char **parseAddress (char *absoluteAddress)
{
	int seperatorCount, temp, i;
	char **splitAddress;
	const char *delim = "/";
	char *absAddress = (char *) malloc (sizeof(char) * strlen(absoluteAddress));
	char *token;

	strcpy (absAddress, absoluteAddress);	

	seperatorCount = 0;
	temp = strlen (absoluteAddress);
	for (i = 0; i < temp; i++)
	{
		if (absoluteAddress[i] == '/')
			seperatorCount ++;
	}

	i = 0;
	splitAddress = (char **) malloc (sizeof (char *) * (seperatorCount +1));

	splitAddress[i] = strtok (absAddress, delim);
	while (splitAddress[i] != NULL)
	{
		i ++;
		splitAddress[i] = strtok (NULL, delim);
	}

	return splitAddress;
}

void addInTree (dirNode *parent, char **splitAddress, int index)
{
	if (splitAddress[index] == NULL)
		return;

	if (parent -> child == NULL)
	{
		parent -> child = (dirNode *) malloc (sizeof(dirNode));
		initDirectory (parent -> child, splitAddress[index]);
		addInTree (parent -> child, splitAddress, index +1);
	}
	else
	{
		dirNode *childList = (parent -> child);
		while (childList != NULL)
		{
			if (strcmp (splitAddress[index], childList -> dirID) == 0)
			{
				addInTree (childList, splitAddress, index +1);
				return;
			}
			childList = childList -> sibling;
		}
		
		dirNode *temp = (dirNode *) malloc (sizeof(dirNode));
		initDirectory (temp, splitAddress[index]);

		temp -> sibling = parent -> child;
		parent -> child = temp;

		addInTree (temp, splitAddress, index +1);
	}
	return;
}

dirNode *searchInTree (dirNode *parent, char **splitAddress, int index)
{
	if (splitAddress[index] == NULL)
		return parent;

	dirNode *childList = (parent -> child);

	if (childList == NULL)
		return NULL;
	else
	{
		while (childList != NULL)
		{
			if (strcmp (splitAddress[index], childList -> dirID) == 0)
				return searchInTree (childList, splitAddress, index +1);
			else
				childList = childList -> sibling;
		}
		return NULL;
	}
}

int read (dirNode *root, char *filename)
{
	FILE *fptr = fopen (filename, "r");
	char buffer[1024];
	char *tempBuffer;
	char **splitAddress;

	while ((fgets (buffer, 1024, fptr) != NULL) && (strlen (buffer) > 2))
	{
		tempBuffer = strtok (buffer, "\r\n");
		printf ("ADDING: %s\n", tempBuffer);
		
		splitAddress = parseAddress (tempBuffer);
		addInTree (root, splitAddress, 0);
	}

	while (fgets (buffer, 1024, fptr) != NULL)
	{
		tempBuffer = strtok (buffer, "\r\n");
		printf ("SEARCHING: %s\t\t", tempBuffer);
		
		splitAddress = parseAddress (tempBuffer);
		
		dirNode *temp = searchInTree (root, splitAddress, 0);
		if (temp)
			printf ("Found\n");
		else
			printf ("Not Found\n");
	}

	fclose (fptr);
	return 0;
}

int main()
{
	dirNode root;
	initDirectory (&root, "");
	read (&root, "directory.in");
	return 0;
}
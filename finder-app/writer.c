#include <stdio.h>
#include <syslog.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main (int argc, char *argv[])
{
	int fd;

	if(argc != 3)
	{
		syslog(LOG_ERR, "Invalid number of arguments.\n");
		exit(1);
	}

	else
	{
		syslog(LOG_DEBUG, "Writing %s to %s\n", argv[2], argv[1]);
		
		fd = open(argv[1], O_RDWR | O_CREAT, 0644);
		
		if ( fd == -1 )
		{
			syslog(LOG_ERR, "Error opening file");
			exit(1);
		}
		else
		{
			write(fd, argv[2], strlen(argv[2]));\
			exit(0);			
		}
	}

	return 0;
}

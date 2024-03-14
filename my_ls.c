//Used for basic input/output stream
#include <stdio.h>
//Used for handling directory files
#include <dirent.h>
//For EXIT codes and error handling
#include <errno.h>
#include <stdlib.h>

void _ls(const char *dir)
{
	//Here we will list the directory
	struct dirent *d;
	DIR *dh = opendir(dir);
	if (!dh)
	{
		if (errno = ENOENT)
		{
			//If the directory is not found
			perror("Directory doesn't exist");
		}
		else
		{
			//If the directory is not readable then throw error and exit
			perror("Unable to read directory");
		}
		exit(EXIT_FAILURE);
	}
	//While the next entry is not readable we will print directory files
    while ((d = readdir(dh)) != NULL)
	{
		//If hidden files are found we continue
		if (d->d_name[0] == '.')
			continue;
		printf("%s  ", d->d_name);
		
    }
    printf("\n");
}
int main(int argc, const char *argv[])
{
	if (argc == 1)
	{
		_ls(".");
	}
	
	return 0;
}



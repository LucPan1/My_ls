//Used for basic input/output stream
#include <stdio.h>
//Used for handling directory files
#include <dirent.h>
//For EXIT codes and error handling
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void _ls(const char *dir, int op_d)
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

	 if (op_d)
    {
		// if(argc == 0) {
			printf("%s\n", dir);
		// }
		// char path[4096];
        // if (getcwd(path, sizeof(path)) != NULL) {
        // printf(".:\nLe chemin du répertoire actuel est : %s\n", path);
		// }
        
        return;
    }

	// if(op_r)
	// {
	// 	printf("%s\n", dir);
	// }
    

	//While the next entry is not readable we will print directory files
    while ((d = readdir(dh)) != NULL)
	{
		//If hidden files are found we continue
		if (d->d_name[0] == '.')
			continue;
		printf("%s  ", d->d_name);
		
    }
    printf("\n");
	closedir(dh); // Close directory after reading
}
int main(int argc, const char *argv[])
{
	int op_d = 0;
	int op_r = 0;

	const char *dir = ".";
	
	if (argc == 1)
	{
		_ls(".", 0);
	}
   else {
        for (int i = 1; i < argc; i++) {
            if (argv[i][0] == '-') {
                char *p = (char *)(argv[i] + 1);
                while (*p) {
                    switch ((*p)) {
                        case 'd':
        				for (int i = 1; i < argc; i++) {
            				if (strcmp(argv[i], "-d") == 0) {
            	    			op_d = 1;
            				} else {
                				dir = argv[i];
            				}
       					}
        					_ls(dir, op_d);
                            // _ls(".", 1); // If -d option is given, call _ls with op_d = 1
                            break;
					

                        default:
                            fprintf(stderr, "Option '%c' not available\n", *p);
                            exit(EXIT_FAILURE);
                    }
                    p++;
                }
            }
        }
   }
	
	return 0;
}



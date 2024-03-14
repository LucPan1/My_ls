#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <limits.h> // Pour PATH_MAX

// Definition of the comparison function for sorting file names by modification time
int compare_mtime(const void *a, const void *b)
{
	const char *filenameA = *(const char **)a;
	const char *filenameB = *(const char **)b;

	struct stat statA, statB;
	stat(filenameA, &statA);
	stat(filenameB, &statB);

	if (statA.st_mtime > statB.st_mtime)
		return -1;
	else if (statA.st_mtime < statB.st_mtime)
		return 1;
	else
		return 0;
}

int compare(const void *a, const void *b)
{
	const char *filenameA = *(const char **)a;
	const char *filenameB = *(const char **)b;
	return strcmp(filenameA, filenameB);
}

void print_file_info(const char *filename, struct stat *statbuf);

void _ls(const char *dir, int op_a, int op_l, int op_R, int op_A, int op_L, int op_d, int op_r, int op_t)
{
	// Remove trailing '/' if present in the directory path
	char directory[PATH_MAX];
	strncpy(directory, dir, PATH_MAX);
	size_t len = strlen(directory);
	if (len > 0 && directory[len - 1] == '/')
	{
		directory[len - 1] = '\0';
	}

	struct stat dir_stat; // Variable pour stocker les informations sur le répertoire
	if (stat(dir, &dir_stat) == -1)
	{
		perror("Cannot access directory");
		exit(EXIT_FAILURE);
	}

	if (S_ISDIR(dir_stat.st_mode))
	{
		// Si le chemin est un répertoire, imprimez son nom seulement s'il n'est pas le répertoire courant
		if (strcmp(dir, ".") != 0)
		{
			printf("%s:\n", dir);
		}
	}
	else
	{
		// Si le chemin n'est pas un répertoire, affichez simplement le nom du fichier
		printf("%s", dir);
		return; // Sortie de la fonction si le chemin n'est pas un répertoire
	}
	// If -d is specified, print only directory name and return
	if (op_d)
	{
		printf("%s\n", dir);
		return;
	}

	// Open the directory
	DIR *dh = opendir(dir);
	if (!dh)
	{
		// Handle errors
		perror("Cannot read directory");
		exit(EXIT_FAILURE);
	}

	struct dirent *d;
	int printed = 0; // Track if anything is printed

	// Array to store file names
	char *files[1024];
	int count = 0;

	// Read file names in the directory
	while ((d = readdir(dh)) != NULL)
	{
		// Include hidden entries if -A option is specified, but exclude . and ..
		if (op_A && strcmp(d->d_name, ".") != 0 && strcmp(d->d_name, "..") != 0)
		{
			files[count++] = strdup(d->d_name);
		}
		// Include . and .. entries if -a option is specified and -A is not specified
		else if (op_a && !op_A && (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0))
		{
			files[count++] = strdup(d->d_name);
		}
		// Include hidden entries if -a option is specified and -A is not specified
		else if (op_a && !op_A && d->d_name[0] == '.')
		{
			files[count++] = strdup(d->d_name);
		}
		// Exclude hidden entries if neither -a nor -A option is specified
		else if (!op_a && !op_A && strcmp(d->d_name, ".") != 0 && strcmp(d->d_name, "..") != 0)
		{
			files[count++] = strdup(d->d_name);
		}
	}

	// Close the directory
	closedir(dh);

	// Sort file names by modification time if -t option is specified
	if (op_t)
	{
		qsort(files, count, sizeof(char *), compare_mtime);
	}
	else
	{
		// Sort file names alphabetically
		qsort(files, count, sizeof(char *), compare);
	}

	// Reverse the order if -r option is specified
	if (op_r)
	{
		for (int i = 0; i < count / 2; i++)
		{
			char *temp = files[i];
			files[i] = files[count - i - 1];
			files[count - i - 1] = temp;
		}
	}

	// Print sorted file names or file details based on option
	for (int i = 0; i < count; i++)
	{
		if (op_l)
		{
			// If -l option is enabled, print file details
			char full_path[PATH_MAX];
			snprintf(full_path, sizeof(full_path), "%s/%s", dir, files[i]);

			struct stat statbuf;
			if (stat(full_path, &statbuf) == -1)
			{
				// Handle error
				perror("Failed to get file status");
				exit(EXIT_FAILURE);
			}

			print_file_info(files[i], &statbuf);
		}
		else
		{
			// If -l option is not enabled, print only file names
			printf("%-10s ", files[i]); // Use %-10s to left-align with width of 10 characters
			printed = 1;
		}
	}

	// Add a new line at the end if -l option is not specified and something was printed
	if (!op_l && printed)
		printf("\n");

	// Free memory allocated for file names
	for (int i = 0; i < count; i++)
	{
		free(files[i]);
	}

	// Recursively list subdirectories if -R option is specified
	if (op_R)
	{
		for (int i = 0; i < count; i++)
		{
			char full_path[PATH_MAX];
			snprintf(full_path, sizeof(full_path), "%s/%s", dir, files[i]);

			struct stat statbuf;
			if (stat(full_path, &statbuf) == -1)
			{
				// Handle error
				perror("Failed to get file status");
				exit(EXIT_FAILURE);
			}

			if (S_ISDIR(statbuf.st_mode))
			{
				printf("\n");
				_ls(full_path, op_a, op_l, op_R, op_A, op_L, op_d, op_r, op_t);
			}
		}
	}
}

void print_file_info(const char *filename, struct stat *statbuf)
{
	// Declare variables needed for date and time
	char time_str[20];
	struct tm *tm_info;

	// Check if the file is a directory
	if (S_ISDIR(statbuf->st_mode))
		printf("d");
	else
		printf("-");

	// Check permissions for user, group, and others
	printf("%c%c%c%c%c%c%c%c%c ",
		   (statbuf->st_mode & S_IRUSR) ? 'r' : '-',
		   (statbuf->st_mode & S_IWUSR) ? 'w' : '-',
		   (statbuf->st_mode & S_IXUSR) ? 'x' : '-',
		   (statbuf->st_mode & S_IRGRP) ? 'r' : '-',
		   (statbuf->st_mode & S_IWGRP) ? 'w' : '-',
		   (statbuf->st_mode & S_IXGRP) ? 'x' : '-',
		   (statbuf->st_mode & S_IROTH) ? 'r' : '-',
		   (statbuf->st_mode & S_IWOTH) ? 'w' : '-',
		   (statbuf->st_mode & S_IXOTH) ? 'x' : '-');

	// Display number of hard links with a field width of 2 characters
	printf("%2ld ", (long)statbuf->st_nlink);

	// Display user and group name with a field width of 8 characters each
	struct passwd *pwd = getpwuid(statbuf->st_uid);
	struct group *grp = getgrgid(statbuf->st_gid);
	printf("%-6s %-8s ", pwd ? pwd->pw_name : "unknown", grp ? grp->gr_name : "unknown");

	// Display file size with a field width of 8 characters
	printf("%8lld ", (long long)statbuf->st_size);

	// Get last modification date and time
	tm_info = localtime(&(statbuf->st_mtime));
	strftime(time_str, 20, "%b %e %H:%M", tm_info);

	// Display date and time
	printf("%s ", time_str);

	// Display file name
	printf("%s\n", filename);
}

int main(int argc, const char *argv[])
{
	int op_a = 0, op_l = 0, op_R = 0, op_A = 0, op_L = 0, op_d = 0, op_r = 0, op_t = 0;

	if (argc == 1)
	{
		// If no option is provided, default behavior is to list directory contents without details
		_ls(".", op_a, op_l, op_R, op_A, op_L, op_d, op_r, op_t);
	}
	else
	{
		// Parse command-line options provided
		for (int i = 1; i < argc; i++)
		{
			if (argv[i][0] == '-')
			{
				char *p = (char *)(argv[i] + 1);
				while (*p)
				{
					switch (*p)
					{
					case 'a':
						op_a = 1;
						break;
					case 'l':
						op_l = 1;
						break;
					case 'r':
						op_r = 1;
						break;
					case 'R':
						op_R = 1;
						break;
					case 'A':
						op_A = 1;
						break;
					case 'L':
						op_L = 1;
						break;
					case 'd':
						op_d = 1; // Mark -d option as enabled
						break;
					case 't':
						op_t = 1;
						break;
					default:
						fprintf(stderr, "Option '%c' not available\n", *p);
						exit(EXIT_FAILURE);
					}
					p++;
				}
			}
			else
			{
				// If a non-option argument is encountered, treat it as the directory to list
				_ls(argv[i], op_a, op_l, op_R, op_A, op_L, op_d, op_r, op_t);
				// Exit after processing the specified directory
				return 0;
			}
		}

		// Call _ls with specified options
		_ls(".", op_a, op_l, op_R, op_A, op_L, op_d, op_r, op_t);
	}

	return 0;
}
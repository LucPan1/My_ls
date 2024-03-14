#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>

// Fonction de comparaison pour qsort
int compare(const void *a, const void *b)
{
	const struct dirent *entryA = *(const struct dirent **)a;
	const struct dirent *entryB = *(const struct dirent **)b;

	// Utilisation de strcasecmp pour ignorer la casse des lettres
	return strcasecmp(entryA->d_name, entryB->d_name);
}

void list_files(const char *dir, struct dirent ***entries, int *num_entries)
{
	*num_entries = scandir(dir, entries, NULL, alphasort);
	if (*num_entries == -1)
	{
		perror("Erreur lors de la lecture du répertoire");
		exit(EXIT_FAILURE);
	}
	// Trier les entrées
	qsort(*entries, *num_entries, sizeof(struct dirent *), compare);
}

void ls(const char *dir, int detailed)
{
	struct dirent **entries;
	int num_entries;
	struct stat fileStat;

	list_files(dir, &entries, &num_entries);

	for (int i = 0; i < num_entries; ++i)
	{
		if (strcmp(entries[i]->d_name, ".") == 0 || strcmp(entries[i]->d_name, "..") == 0 || strcmp(entries[i]->d_name, ".git") == 0)
		{
			continue;
		}

		char filename[1024];
		snprintf(filename, sizeof(filename), "%s/%s", dir, entries[i]->d_name);

		if (lstat(filename, &fileStat) == -1)
		{
			perror("Erreur lors de la lecture des informations du fichier");
			continue;
		}

		if (detailed)
		{
			// Affichage détaillé des fichiers
			char perms[11];
			snprintf(perms, sizeof(perms), "%c%c%c%c%c%c%c%c%c%c",
					 (S_ISDIR(fileStat.st_mode)) ? 'd' : '-',
					 (fileStat.st_mode & S_IRUSR) ? 'r' : '-',
					 (fileStat.st_mode & S_IWUSR) ? 'w' : '-',
					 (fileStat.st_mode & S_IXUSR) ? 'x' : '-',
					 (fileStat.st_mode & S_IRGRP) ? 'r' : '-',
					 (fileStat.st_mode & S_IWGRP) ? 'w' : '-',
					 (fileStat.st_mode & S_IXGRP) ? 'x' : '-',
					 (fileStat.st_mode & S_IROTH) ? 'r' : '-',
					 (fileStat.st_mode & S_IWOTH) ? 'w' : '-',
					 (fileStat.st_mode & S_IXOTH) ? 'x' : '-');

			struct passwd *user = getpwuid(fileStat.st_uid);
			struct group *group = getgrgid(fileStat.st_gid);

			char date[20];
			strftime(date, sizeof(date), "%b %d %H:%M", localtime(&fileStat.st_mtime));

			printf("%s %ld %s %s %ld %s %s\n", perms, (long)fileStat.st_nlink, user->pw_name, group->gr_name, (long)fileStat.st_size, date, entries[i]->d_name);
		}
		else
		{
			// Affichage simple des noms de fichiers
			printf("%s  ", entries[i]->d_name);
		}
	}

	for (int i = 0; i < num_entries; ++i)
	{
		free(entries[i]);
	}
	free(entries);
}

int main(int argc, const char *argv[])
{
	int detailed = 0;
	const char *dir = ".";

	if (argc > 1)
	{
		if (strcmp(argv[1], "-l") == 0)
		{
			detailed = 1;
			if (argc > 2)
			{
				dir = argv[2];
			}
		}
		else
		{
			dir = argv[1];
		}
	}

	ls(dir, detailed);

	return 0;
}
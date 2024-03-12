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

void _ls(const char *dir, int op_a, int op_l, int op_R, int op_A, int op_L);

void print_file_info(const char *filename, struct stat *statbuf);

// Comparateur pour la fonction de tri
int compare(const void *a, const void *b)
{
    return strcmp(*(const char **)b, *(const char **)a);
}

void _ls(const char *dir, int op_a, int op_l, int op_R, int op_A, int op_L)
{
    DIR *dh = opendir(dir);
    if (!dh)
    {
        perror("Unable to read directory");
        exit(EXIT_FAILURE);
    }

    struct dirent *d;
    int printed = 0;
    int num_entries = 0;
    char **entries = NULL;

    // Lire les noms de fichiers dans un tableau
    while ((d = readdir(dh)) != NULL)
    {
        if (!op_a && d->d_name[0] == '.')
        {
            if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
                continue;

            continue;
        }

        entries = realloc(entries, (num_entries + 1) * sizeof(char *));
        entries[num_entries] = strdup(d->d_name);
        num_entries++;
    }

    // Fermer le répertoire
    closedir(dh);

    // Trier les noms de fichiers
    qsort(entries, num_entries, sizeof(char *), compare);

    // Afficher les noms de fichiers triés
    for (int i = 0; i < num_entries; i++)
    {
        if (op_l)
        {
            char path[PATH_MAX];
            snprintf(path, sizeof(path), "%s/%s", dir, entries[i]);

            struct stat statbuf;
            if (stat(path, &statbuf) == -1)
            {
                perror("Failed to get file status");
                exit(EXIT_FAILURE);
            }

            print_file_info(entries[i], &statbuf);
        }
        else
        {
            printf("%s  ", entries[i]);
            printed = 1;
        }

        free(entries[i]);
    }

    free(entries);

    if (!op_l && printed)
        printf("\n");
}

void print_file_info(const char *filename, struct stat *statbuf)
{
    // Print file type and permissions
    printf((S_ISDIR(statbuf->st_mode)) ? "d" : "-");
    printf((statbuf->st_mode & S_IRUSR) ? "r" : "-");
    printf((statbuf->st_mode & S_IWUSR) ? "w" : "-");
    printf((statbuf->st_mode & S_IXUSR) ? "x" : "-");
    printf((statbuf->st_mode & S_IRGRP) ? "r" : "-");
    printf((statbuf->st_mode & S_IWGRP) ? "w" : "-");
    printf((statbuf->st_mode & S_IXGRP) ? "x" : "-");
    printf((statbuf->st_mode & S_IROTH) ? "r" : "-");
    printf((statbuf->st_mode & S_IWOTH) ? "w" : "-");
    printf((statbuf->st_mode & S_IXOTH) ? "x" : "-");

    // Print number of links
    printf(" %ld", (long)statbuf->st_nlink);

    // Print owner name
    struct passwd *pw = getpwuid(statbuf->st_uid);
    if (pw != NULL)
        printf(" %s", pw->pw_name);
    else
        printf(" %d", statbuf->st_uid);

    // Print group name
    struct group *gr = getgrgid(statbuf->st_gid);
    if (gr != NULL)
        printf(" %s", gr->gr_name);
    else
        printf(" %d", statbuf->st_gid);

    // Print file size
    printf(" %lld ", (long long)statbuf->st_size);

    // Print last modification time
    struct tm *tm_info;
    char time_str[20];
    tm_info = localtime(&(statbuf->st_mtime));
    strftime(time_str, sizeof(time_str), "%b %d %H:%M", tm_info);
    printf(" %s ", time_str);

    // Print file name
    printf(" %s\n", filename);
}

int main(int argc, const char *argv[])
{
    int op_a = 0, op_l = 0, op_R = 0, op_A = 0, op_L = 0;

    if (argc == 1)
    {
        // Si aucune option n'est fournie, le comportement par défaut est de lister le contenu du répertoire sans les détails
        _ls(".", op_a, op_l, op_R, op_A, op_L);
    }
    else
    {
        // Analyser les options fournies en ligne de commande
        for (int i = 1; i < argc; i++)
        {
            if (argv[i][0] == '-')
            {
                char *p = (char *)(argv[i] + 1);
                while (*p)
                {
                    switch (tolower(*p)) // Convertir l'option en minuscule
                    {
                    case 'a':
                        op_a = 1;
                        break;
                    case 'l':
                        op_l = 1;
                        break;
                    case 'r':
                        fprintf(stderr, "Option '-r' is not supported.\n");
                        exit(EXIT_FAILURE);
                    case 'R':
                        op_R = 1;
                        break;
                    case 'A':
                        op_A = 1;
                        break;
                    case 'L':
                        op_L = 1;
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
                fprintf(stderr, "Usage: %s [-a] [-l] [-R] [-A] [-L]\n", argv[0]);
                exit(EXIT_FAILURE);
            }
        }

        // Vérifier si l'option -l est activée
        if (op_l)
        {
            // Si l'option -l est activée, lister le contenu du répertoire avec les détails
            _ls(".", op_a, op_l, op_R, op_A, op_L);
        }
        else
        {
            // Si l'option -l n'est pas activée, lister le contenu du répertoire sans les détails
            _ls(".", op_a, op_l, op_R, op_A, op_L);
        }
    }

    return 0;
}

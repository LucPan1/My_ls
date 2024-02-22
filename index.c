#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

void _ls(const char *dir, int op_a, int op_l, int op_R, int depth);

void print_file_info(const char *filename, struct stat *statbuf);

void _ls(const char *dir, int op_a, int op_l, int op_R, int depth)
{
    int total_size = 0; // Variable pour stocker le total des tailles des fichiers

    // Ouvrir le répertoire
    DIR *dh = opendir(dir);
    if (!dh)
    {
        // Gérer les erreurs
        perror("Unable to read directory");
        exit(EXIT_FAILURE);
    }

    struct dirent *d;

    // Lire tous les fichiers du répertoire
    while ((d = readdir(dh)) != NULL)
    {
        if (!op_a && d->d_name[0] == '.')
            continue;

        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dir, d->d_name);

        struct stat statbuf;
        if (lstat(path, &statbuf) == -1)
        {
            // Gérer les erreurs
            perror("Failed to get file status");
            continue;
        }

        // Ajouter la taille du fichier à la somme totale
        total_size += statbuf.st_size;
        // Afficher les informations du fichier si -l est spécifié
        if (op_l)
        {
            print_file_info(d->d_name, &statbuf);
        }
        else
        {
            // Afficher le nom du fichier
            printf("%s  ", d->d_name);
        }
    }

    // Fermer le répertoire
    closedir(dh);
}

void print_file_info(const char *filename, struct stat *statbuf)
{
    // Print file permissions
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
    printf(" ");

    // Print number of links²
    printf("%lu ", statbuf->st_nlink);

    // Print owner name
    struct passwd *pw = getpwuid(statbuf->st_uid);
    if (pw)
    {
        printf("%s ", pw->pw_name);
    }
    else
    {
        printf("%d ", statbuf->st_uid);
    }

    // Print group name
    struct group *gr = getgrgid(statbuf->st_gid);
    if (gr)
    {
        printf("%s ", gr->gr_name);
    }
    else
    {
        printf("%d ", statbuf->st_gid);
    }

    // Print file size
    printf("%lld ", (long long)statbuf->st_size);

    // Print last modification time
    struct tm *tm_info;
    char time_str[20];
    tm_info = localtime(&(statbuf->st_mtime));
    strftime(time_str, 20, "%b %d %H:%M", tm_info);
    printf("%s ", time_str);

    // Print file name
    printf("%s\n", filename);
}

int main(int argc, const char *argv[])
{
    int op_a = 0, op_l = 0, op_R = 0;

    if (argc == 1)
    {
        _ls(".", op_a, op_l, op_R, 0);
    }
    else
    {
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
                    case 'R':
                        op_R = 1;
                        break;
                    case 'r':
                        fprintf(stderr, "Option '-r' is not supported.\n");
                        exit(EXIT_FAILURE);
                    default:
                        fprintf(stderr, "Option '%c' not available\n", *p);
                        exit(EXIT_FAILURE);
                    }
                    p++;
                }
            }
            else
            {
                fprintf(stderr, "Usage: %s [-a] [-l] [-R]\n", argv[0]);
                exit(EXIT_FAILURE);
            }
        }
        _ls(".", op_a, op_l, op_R, 0);
    }

    return 0;
}

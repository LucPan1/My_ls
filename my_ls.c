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

void _ls(const char *dir, int op_a, int op_l, int op_R, int op_A, int op_L, int op_d);

void print_file_info(const char *filename, struct stat *statbuf);

void _ls(const char *dir, int op_a, int op_l, int op_R, int op_A, int op_L, int op_d)
{
    // If -d option is specified, print only directory name and exit
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
        perror("Unable to read directory");
        exit(EXIT_FAILURE);
    }

    struct dirent *d;
    int printed = 0; // Track if anything is printed

    // Print filenames or file details depending on the option
    while ((d = readdir(dh)) != NULL)
    {
        // Exclude hidden entries if -a option is not provided
        if (!op_a && d->d_name[0] == '.')
        {
            // Exclude . and .. entries
            if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
                continue;

            // Exclude other hidden entries
            continue;
        }

        if (op_l)
        {
            // If -l option is enabled, print file details
            char path[PATH_MAX];
            snprintf(path, sizeof(path), "%s/%s", dir, d->d_name);

            struct stat statbuf;
            if (stat(path, &statbuf) == -1)
            {
                // Handle error
                perror("Failed to get file status");
                exit(EXIT_FAILURE);
            }

            print_file_info(d->d_name, &statbuf);
        }
        else
        {
            // If -l option is not enabled, print only filenames
            printf("%-10s ", d->d_name); // Utilisation de %-10s pour aligner à gauche avec une largeur de 10 caractères
            printed = 1;
        }
    }

    // Close the directory
    closedir(dh);

    // Ajouter une nouvelle ligne à la fin si -l option n'est pas spécifiée et si quelque chose a été imprimé
    if (!op_l && printed)
        printf("\n");
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

    // Calculate the maximum width for the number of links
    int nlink_width = snprintf(NULL, 0, "%ld", statbuf->st_nlink);

    // Print number of links with dynamic width
    printf("%*ld ", nlink_width, statbuf->st_nlink);

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

    // Calculate the maximum width for the file size
    int size_width = snprintf(NULL, 0, "%lld", (long long)statbuf->st_size);

    // Print file size with dynamic width
    printf("%*lld ", size_width, (long long)statbuf->st_size);

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
    int op_a = 0, op_l = 0, op_R = 0, op_A = 0, op_L = 0, op_d = 0;

    if (argc == 1)
    {
        // Si aucune option n'est fournie, le comportement par défaut est de lister le contenu du répertoire sans les détails
        _ls(".", op_a, op_l, op_R, op_A, op_L, op_d);
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
                    case 'd':
                        op_d = 1;
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
                fprintf(stderr, "Usage: %s [-a] [-l] [-R] [-A] [-L] [-d]\n", argv[0]);
                exit(EXIT_FAILURE);
            }
        }

        // Vérifier si l'option -l est activée
        if (op_l)
        {
            // Si l'option -l est activée, lister le contenu du répertoire avec les détails
            _ls(".", op_a, op_l, op_R, op_A, op_L, op_d);
        }
        else
        {
            // Si l'option -l n'est pas activée, lister le contenu du répertoire sans les détails
            _ls(".", op_a, op_l, op_R, op_A, op_L, op_d);
        }
    }

    return 0;
}

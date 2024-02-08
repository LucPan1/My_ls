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

void _ls(const char *dir, int op_a, int op_l, int op_R, int depth)
{
    struct dirent *d;
    DIR *dh = opendir(dir);
    if (!dh)
    {
        if (errno == ENOENT)
        {
            perror("Directory doesn't exist");
        }
        else
        {
            perror("Unable to read directory");
        }
        exit(EXIT_FAILURE);
    }

    // Array to store directory entries
    struct dirent **entries = NULL;
    int num_entries = 0;

    // Read all directory entries and store them in the array
    while ((d = readdir(dh)) != NULL)
    {
        if (!op_a && d->d_name[0] == '.')
            continue;

        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dir, d->d_name);

        struct stat statbuf;
        if (lstat(path, &statbuf) == -1)
        {
            perror("Failed to get file status");
            continue;
        }

        if (S_ISDIR(statbuf.st_mode))
        {
            if (op_R && strcmp(d->d_name, ".") != 0 && strcmp(d->d_name, "..") != 0)
            {
                printf("\n%*s%s:\n", depth * 2, "", d->d_name); // Print subdirectory name
                _ls(path, op_a, op_l, op_R, depth + 1);         // Recursively list subdirectory
            }
        }
        else
        {
            // Store non-directory entries in the array
            entries = realloc(entries, (num_entries + 1) * sizeof(struct dirent *));
            entries[num_entries++] = d;
        }
    }

    // List the directory entries (files) after exploring subdirectories
    for (int i = 0; i < num_entries; i++)
    {
        d = entries[i];
        if (op_l)
        {
            char path[1024];
            snprintf(path, sizeof(path), "%s/%s", dir, d->d_name);

            struct stat statbuf;
            if (lstat(path, &statbuf) == -1)
            {
                perror("Failed to get file status");
                continue;
            }
            print_file_info(d->d_name, &statbuf);
        }
        else
        {
            printf("%*s%s  ", depth * 2, "", d->d_name);
        }
    }

    // Free the array
    free(entries);

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

    // Print number of links
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

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

void _ls(const char *dir, int op_a, int op_l, int op_R, int op_A, int op_L)
{
    // Open the directory
    DIR *dh = opendir(dir);
    if (!dh)
    {
        // Handle errors
        perror("Unable to read directory");
        exit(EXIT_FAILURE);
    }

    struct dirent *d;

    // Print filenames or file details depending on the option
    while ((d = readdir(dh)) != NULL)
    {
        if ((!op_a && d->d_name[0] == '.') || (!op_A && (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)))
            continue;

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
            printf("%s\n", d->d_name);
        }
    }

    // Close the directory
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

int main(int argc, const char *argv[])
{
    int op_a = 0, op_l = 0, op_R = 0, op_A = 0, op_L = 0;

    if (argc == 1)
    {
        _ls(".", op_a, op_l, op_R, op_A, op_L);
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
                    switch (tolower(*p)) // Convert the option to lowercase
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
        _ls(".", op_a, op_l, op_R, op_A, op_L);
    }

    return 0;
}

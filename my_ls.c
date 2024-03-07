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

    DIR *dh = opendir(dir);
    if (!dh)
    {

        perror("Unable to read directory");
        exit(EXIT_FAILURE);
    }

    if (op_d)
    {
        printf("%s\n", dir);
        closedir(dh);
        return;
    }
}

void print_file_info(const char *filename, struct stat *statbuf)
{

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

    int nlink_width = snprintf(NULL, 0, "%ld", statbuf->st_nlink);

    printf("%*ld ", nlink_width, statbuf->st_nlink);

    struct passwd *pw = getpwuid(statbuf->st_uid);
    if (pw)
    {
        printf("%s ", pw->pw_name);
    }
    else
    {
        printf("%d ", statbuf->st_uid);
    }

    struct group *gr = getgrgid(statbuf->st_gid);
    if (gr)
    {
        printf("%s ", gr->gr_name);
    }
    else
    {
        printf("%d ", statbuf->st_gid);
    }

    int size_width = snprintf(NULL, 0, "%lld", (long long)statbuf->st_size);

    printf("%*lld ", size_width, (long long)statbuf->st_size);

    struct tm *tm_info;
    char time_str[20];
    tm_info = localtime(&(statbuf->st_mtime));
    strftime(time_str, 20, "%b %d %H:%M", tm_info);
    printf("%s ", time_str);

    printf("%s\n", filename);
}

int main(int argc, const char *argv[])
{
    int op_a = 0, op_l = 0, op_R = 0, op_A = 0, op_L = 0, op_d = 0;

    if (argc == 1)
    {

        _ls(".", op_a, op_l, op_R, op_A, op_L, op_d);
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
                    switch (tolower(*p))
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

        if (op_l)
        {

            _ls(".", op_a, op_l, op_R, op_A, op_L, op_d);
        }
        else
        {
            _ls(".", op_a, op_l, op_R, op_A, op_L, op_d);
        }
    }

    return 0;
}

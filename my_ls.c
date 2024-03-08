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
#include <limits.h>

// Définition de la fonction de comparaison pour le tri des noms de fichiers
int compare(const void *a, const void *b)
{
    return strcmp(*(const char **)a, *(const char **)b);
}

void _ls(const char *dir, int op_a, int op_l, int op_R, int op_A, int op_L, int op_d);

void print_file_info(const char *filename, struct stat *statbuf);

void _ls(const char *dir, int op_a, int op_l, int op_R, int op_A, int op_L, int op_d)
{
    // Autres déclarations et initialisations...

    long total_blocks = 0; // Variable pour stocker le nombre total de blocs utilisés

    DIR *dh; // Variable pour stocker le pointeur vers le répertoire
    struct dirent *d; // Variable pour stocker les informations sur les fichiers

    // Ouvrir le répertoire
    dh = opendir(dir);
    if (!dh)
    {
        // Gérer les erreurs
        perror("Impossible de lire le répertoire");
        exit(EXIT_FAILURE);
    }

    char path[PATH_MAX]; // Variable pour stocker le chemin complet du fichier

    // Lire les noms de fichiers dans le répertoire
    while ((d = readdir(dh)) != NULL)
    {
        // Exclure les entrées cachées si l'option -a n'est pas fournie
        // ...

        // Obtenir les informations sur le fichier
        snprintf(path, sizeof(path), "%s/%s", dir, d->d_name);
        struct stat statbuf;
        if (stat(path, &statbuf) == -1)
        {
            // Gérer l'erreur
            perror("Échec de l'obtention du statut du fichier");
            exit(EXIT_FAILURE);
        }

        // Ajouter la taille du fichier à la variable du total des blocs utilisés
        total_blocks += statbuf.st_blocks;
    }

    // Calculer le nombre total de blocs utilisés en divisant le total par la taille d'un bloc
    long total_blocks_in_kb = total_blocks / 2; // La taille d'un bloc est généralement de 512 octets

    // Afficher le nombre total de blocs utilisés
    printf("total %ld\n", total_blocks_in_kb);

    // Si -d est spécifié, imprimer uniquement le nom du répertoire et sortir
    if (op_d)
    {
        printf("%s\n", dir);
        return;
    }

    // Retourner au début du répertoire pour lire les noms de fichiers à nouveau
    rewinddir(dh);

    // Tableau pour stocker les noms de fichiers
    char *files[1024];
    int count = 0;

    // Lire les noms de fichiers dans le répertoire
    while ((d = readdir(dh)) != NULL)
    {
        // Exclure les entrées cachées si l'option -a n'est pas fournie
        if (!op_a && d->d_name[0] == '.')
        {
            // Exclure les entrées . et ..
            if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
                continue;

            // Exclure les autres entrées cachées
            continue;
        }

        // Stocker le nom du fichier dans le tableau
        files[count++] = strdup(d->d_name);
    }

    // Fermer le répertoire
    closedir(dh);

    // Trier les noms de fichiers
    qsort(files, count, sizeof(char *), compare);

    // Imprimer les noms de fichiers triés ou les détails des fichiers selon l'option
    for (int i = 0; i < count; i++)
    {
        if (op_l)
        {
            // Si l'option -l est activée, imprimer les détails du fichier
            snprintf(path, sizeof(path), "%s/%s", dir, files[i]);
            struct stat statbuf;
            if (stat(path, &statbuf) == -1)
            {
                // Gérer l'erreur
                perror("Échec de l'obtention du statut du fichier");
                exit(EXIT_FAILURE);
            }

            print_file_info(files[i], &statbuf);
        }
        else
        {
            // Si l'option -l n'est pas activée, imprimer uniquement les noms de fichiers
            printf("%-10s ", files[i]); // Utilisation de %-10s pour aligner à gauche avec une largeur de 10 caractères
        }
    }

    // Ajouter une nouvelle ligne à la fin si l'option -l n'est pas spécifiée
    if (!op_l)
        printf("\n");

    // Libérer la mémoire allouée pour les noms de fichiers
    for (int i = 0; i < count; i++)
    {
        free(files[i]);
    }
}

void print_file_info(const char *filename, struct stat *statbuf)
{
    // Déclarer les variables nécessaires pour la date et l'heure
    char time_str[20];
    struct tm *tm_info;

    // Vérifier si le fichier est un répertoire
    if (S_ISDIR(statbuf->st_mode))
        printf("d");
    else
        printf("-");

    // Vérifier les autorisations pour l'utilisateur, le groupe et les autres
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

    // Afficher le nombre de liens durs avec une largeur de champ de 2 caractères
    printf("%2ld ", (long)statbuf->st_nlink);

    // Afficher le nom de l'utilisateur et du groupe avec une largeur de champ de 4 caractères chacun
    struct passwd *pwd = getpwuid(statbuf->st_uid);
    struct group *grp = getgrgid(statbuf->st_gid);
    printf("%-4s %-4s", pwd ? pwd->pw_name : "unknown", grp ? grp->gr_name : "unknown");

    // Afficher la taille du fichier avec une largeur de champ de 8 caractères
    printf("%8lld ", (long long)statbuf->st_size);

    // Obtenir la date et l'heure de la dernière modification
    tm_info = localtime(&(statbuf->st_mtime));
    strftime(time_str, 20, "%b %e %H:%M", tm_info);

    // Afficher la date et l'heure
    printf("%s ", time_str);

    // Afficher le nom du fichier
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

        // Appeler _ls avec les options spécifiées
        _ls(".", op_a, op_l, op_R, op_A, op_L, op_d);
    }

    return 0;
}

#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>

// op_a -> file
// op_l -> list
void ls_function(const char *dir, int op_a, int op_l)
{
    struct dirent *d;
    DIR *dh = opendir(dir);
    if (!dh)
    {
        if (errno = ENOENT)
        {
            // If the directory is not found
            perror("Directory doesn't exist");
        }
        else
        {
            // If the directory is not readable then throw error and exit
            perror("Unable to read directory");
        }
        exit(EXIT_FAILURE);
    }
    // Cette boucle while est utilisée pour parcourir les entrées du répertoire. Voici comment chaque partie fonctionne :

    //     d = readdir(dh) : La fonction readdir est utilisée pour lire la prochaine entrée du répertoire. Elle renvoie un pointeur vers une structure dirent, qui contient des informations sur l'entrée du répertoire.

    //     while ((d = readdir(dh)) != NULL) : Cette boucle s'exécute tant que la fonction readdir renvoie un pointeur non nul, ce qui signifie qu'il y a encore des entrées dans le répertoire à lire.

    //     if (!op_a && d->d_name[0] == '.') continue; : Cette instruction vérifie si l'option -a n'est pas activée (!op_a) et si le nom de l'entrée commence par un point (d->d_name[0] == '.'). Si c'est le cas, cela signifie que l'entrée est un fichier caché, et dans ce cas, la boucle passe à l'itération suivante sans exécuter le reste du code à l'intérieur de la boucle.

    //     printf("%s ", d->d_name); : Cette instruction imprime le nom de l'entrée du répertoire à l'écran suivi d'un espace.

    //     if (op_l) printf("\n"); : Si l'option -l est activée (op_l est vrai), alors après avoir imprimé le nom de l'entrée, un caractère de nouvelle ligne (\n) est imprimé. Cela a pour effet de placer chaque nom d'entrée sur une nouvelle ligne lorsque l'option -l est activée.

    // En résumé, cette boucle parcourt les entrées du répertoire, ignore les fichiers cachés si l'option -a n'est pas activée, et affiche les noms des entrées sur la même ligne ou sur des lignes séparées, en fonction de la présence de l'option -l.
    while ((d = readdir(dh)) != NULL)
    {
        if (!op_a && d->d_name[0] == '.')
            continue;
        printf("%s  ", d->d_name);
        if (op_l)
            printf("\n");
    }
    if (!op_l)
        printf("\n");
}

int main(int argc, const char *argv[])
{
    if (argc == 1)
    {
        ls_function(".", 0, 0);
    }
    else if (argc == 2)
    {
        if (argv[1][0] == '-')
        {
            // Checking if option is passed
            // Options supporting: a, l
            int op_a = 0, op_l = 0;
            char *p = (char *)(argv[1] + 1);
            while (*p)
            {
                if (*p == 'a')
                    op_a = 1;
                else if (*p == 'l')
                    op_l = 1;
                else
                {
                    perror("Option not available");
                    exit(EXIT_FAILURE);
                }
                p++;
            }
            ls_function(".", op_a, op_l);
        }
    }
    return 0;
}

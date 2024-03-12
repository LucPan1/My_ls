#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

int main(int argc, char *argv[]) {
    // Vérifier le nombre d'arguments
    if (argc != 2) {
        fprintf(stderr, "Utilisation: %s <répertoire>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Ouvrir le répertoire spécifié en argument
    DIR *dir = opendir(argv[1]);

    // Vérifier si le répertoire est ouvert avec succès
    if (dir == NULL) {
        perror("Erreur lors de l'ouverture du répertoire");
        exit(EXIT_FAILURE);
    }

    // Lire les entrées du répertoire
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        printf("%s\n", entry->d_name);
    }

    // Fermer le répertoire
    closedir(dir);

    return EXIT_SUCCESS;
}
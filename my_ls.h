#ifndef MY_LS_H
#define MY_LS_H

// Inclusion des bibliothèques nécessaires
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// Déclaration de la fonction _ls
void _ls(const char *dir, int op_d, int op_r);

#endif /* MY_LS_H */

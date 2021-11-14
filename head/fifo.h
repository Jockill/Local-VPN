#ifndef __FIFO_H__
#define __FIFO_H__

#include <stdio.h>
#include <stdlib.h>
#include "../head/utils.h"


typedef struct maillon
{
        paquet* element;
        struct maillon* precedent;
        struct maillon* suivant;
} maillon;


typedef struct fifo
{
        maillon* debut;
        maillon* fin;
        ssize_t taille;
} fifo;


fifo* cree_fifo();

int push_fifo(fifo* f, paquet* p);

maillon* premier_fifo(fifo* f);

paquet* pop_fifo(fifo* f);

int est_vide_fifo(fifo* f);

int envoi_fifo(fifo* f, int socket, struct sockaddr_in* dest);

void supprimer_fifo(fifo* f);

void affiche_fifo(fifo* f);

#endif

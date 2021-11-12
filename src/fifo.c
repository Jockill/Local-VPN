#include "../head/fifo.h"
#include "../head/utils.h"

fifo* cree_fifo()
{
        fifo* file = (fifo*)malloc(sizeof(fifo));
        if (file == NULL)
                return NULL;
        file->debut = NULL;
        file->fin = NULL;
        file->taille = 0;
        return file;
}

int push_fifo(fifo* f, paquet* p)
{
        maillon* fin = f->fin;

        maillon* new = (maillon*)malloc(sizeof(maillon));
        if (new == NULL)
                return -1;
        new->element = p;
        new->precedent = fin;
        new->suivant = NULL;

        if (fin != NULL)
                fin->suivant = new;

        if (f->taille == 0)
                f->debut = new;
        f->fin = new;
        (f->taille)++;
        return 0;
}

maillon* premier_fifo(fifo* f) {return f->debut;}

maillon* pop_fifo(fifo* f)
{
        maillon* tete = f->debut;
        maillon* nouveauDebut = f->debut->suivant;

        printf("nouvD = %p\n", nouveauDebut);

        f->debut = nouveauDebut;
        (f->taille)--;
        if (f->taille == 0)
                f->fin = NULL;

        return tete;
}

int est_vide_fifo(fifo* f) {return f->taille>0 ? 1:0;}

int envoi_fifo(fifo* f, int socket, struct sockaddr_in* dest)
{
        maillon* courant = f->debut;
        for (ssize_t i=0; i<f->taille; i++)
        {
                if (courant == NULL)
                {
                        fprintf(stderr, "envoi_fifo: La file ne semble pas correctement chainée.\n");
                        return -1;
                }
                envoi_paquet(socket, dest, courant);
                courant = courant->suivant;
        }
        return 0;
}

void affiche_fifo(fifo* f)
{
        printf("============= FILE ============\n");
        printf("Debut : %p\n", f->debut);
        printf("Fin:    %p\n", f->fin);
        printf("Taille: %ld\n", f->taille);
        printf("===============================\n");
}

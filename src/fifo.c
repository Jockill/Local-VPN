#include "../head/fifo.h"

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
        f->taille+=1;
        return 0;
}

maillon* premier_fifo(fifo* f) {return f->debut;}

paquet* pop_fifo(fifo* f)
{
        maillon* tete = f->debut;
        maillon* nouveauDebut = f->debut->suivant;

        f->debut = nouveauDebut;
        f->taille -= 1;
        if (f->taille == 0)
                f->fin = NULL;
        paquet * res = tete->element;
        free(tete);
        return res;
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
                envoie_paquet(socket,(struct sockaddr*)dest, courant->element);
                courant = courant->suivant;
        }
        return 0;
}

void supprimer_fifo(fifo* f)
{
        while (f->taille > 0)
                pop_fifo(f);

        free(f);
}

void affiche_fifo(fifo* f)
{       
        printf("============= FILE ============\n");
        maillon* tmp = f->debut;
        int compteur = 0;
        while(tmp != NULL){
                printf("Paquet : %hu\n",tmp->element->numSeq);
                tmp = tmp->suivant;
                compteur++;
        }
        printf("Taille reel: %d\n", compteur);
        printf("Taille: %ld\n", f->taille);
        printf("===============================\n");
}

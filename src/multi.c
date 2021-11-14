#include <stdlib.h>
#include <string.h>
#include "../head/utils.h"
#include "../head/fifo.h"

typedef struct fifo flux;

#define NBR_FLUX 3

char* flux_to_str(flux* flux)
{
        ssize_t taille = flux->taille;
        maillon* courant = flux->debut;
        int strPos = 0;
        char* str = (char*)malloc(sizeof(char) * taille * TAILLE_DONNEES);

        for (ssize_t i=0; i<taille; i++)
        {
                for (ssize_t j=0; j<TAILLE_DONNEES; j++)
                        str[strPos++] = courant->element->donnees[j];
                courant = courant->suivant;

                printf("\t%s\n", str);
        }

        return str;
}

flux* creer_flux(unsigned char idFlux, char* donnee)
{
        int tailleDonnee = strlen(donnee);
        int nbrPaquets = 0;
        int donnePos = 0;
        flux* f = (flux*)malloc(sizeof(flux));

        //Si la taille des donnees remplit exactement n paquets
        if (tailleDonnee%TAILLE_DONNEES == 0)
                nbrPaquets = tailleDonnee/TAILLE_DONNEES;
        else
                nbrPaquets = tailleDonnee/TAILLE_DONNEES +1;

        // Créer les paquets et les inserer dans le flux
        for (int i=0; i<nbrPaquets; i++)
        {
                //Recuperer les TAILLE_DONNEES prochains caracteres de `donnee`
                char str[TAILLE_DONNEES] = {0};
                strncpy(str, &donnee[donnePos], TAILLE_DONNEES);
                donnePos += TAILLE_DONNEES;

                paquet* p = (paquet*)malloc(sizeof(paquet));
                *p = cree_paquet(idFlux, DATA, i, 0, 0, 0, str);

                if (push_fifo(f, p) == -1)
                        tue_moi("creer_flux, push_fifo", 0);
        }

        return f;
}

int main()
{
        flux* tousLesFlux[NBR_FLUX];

        for (int i=0; i<NBR_FLUX; i++)
                tousLesFlux[i] = cree_fifo();

        char* donnee1 = "1234567890 azertyuio qsdfghjkl wxcvbn,;: Donnee1";
        char* buf;

        tousLesFlux[0] = creer_flux(1, donnee1);
        buf = flux_to_str(tousLesFlux[0]);

        printf("%s\n", buf);

        return 0;
}

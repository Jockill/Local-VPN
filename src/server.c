#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "../head/utils.h"

void check_args_dst(int argc, char** argv)
{
        if (argc != 4)
        {
                fprintf(stderr, "Usage: %s <IP_distante> <port_local> <port_medium>\n", argv[0]);
                exit(1);
        }

        if (ipv4_valide(argv[1]) == 0)
        {
                fprintf(stderr, "L'adresse IP donnée n'est pas une adresse IPv4 valide.\n");
                exit(1);
        }
        if (2048 > port || port > 49151)
        {
                fprintf(stderr, "Le port local doit etre compris entre 2048 et 49151 exclus\n");
                exit(1);
        }
        if (2048 > port || port > 49151)
        {
                fprintf(stderr, "Le port du medium doit etre compris entre 2048 et 49151 exclus\n");
                exit(1);
        }

}

int negociation_dst(int* sockServer, int* sockClient,
                    struct sockaddr_in* client, fenetre* fenetre, int* mode)
{
        //Preconditions
        if (sockClient < 0)
        {
                fprintf(stderr, "negociation_dst: sockClient < 0.\n");
                exit(1);
        }
        if (sockServer < 0)
        {
                fprintf(stderr, "negociation_dst: sockServer < 0.\n");
                exit(1);
        }
        if (client == NULL)
                tue_moi("negociation_dst: client NULL", 2, sockClient,
                        sockServer);


        paquet paquet = {0};
        ssize_t tmp = 0;
        socklen_t lenAddrClient = sizeof(addrClient);
        unsigned char type = 0;
        fd_set sockSet;
        FD_ZERO(&sockSet);
        FD_SET(sockServer, &sockSet);

        //reception SYN
        while ((paquet.type != SYN) && (tmp != 52))
        {
                tmp = recvfrom(sockClient, paquet, TAILLE_PAQUET, NULL,
                                  (struct sockaddr*)&addrClient, lenAddrClient);
                if (tmp == -1)
                        tue_moi("negociation_dst, recvfrom", 2,
                                sockServer, sockClient);
        }

        //Configuration du mode
        if (paquet.tailleFenetre == 0)
                *mode = STOP_N_WAIT;
        else
                *mode = GO_BACK_N;

        //envoi SYN-ACK
        paquet = cree_paquet(paquet.idFlux, SYN+ACK, (short)rand(),
                             paquet.numSeq+1, 0, TAILLE_FENETRE_SERVEUR, NULL);
        tmp = 0;
        while((paquet.type != ACK) && (tmp != 52))
        {
                //TODO
        }

        return 0;
}


/*void stop_and_wait_ecoute(struct sockaddr_in* client)
{
        //Preconditions
        if (client == NULL)
        {
                fprintf(stderr, "stop_and_wait_ecoute: client NULL.\n");
                exit(1);
        }

}*/

/*void go_back_n_ecoute(struct sockaddr_in* client, fenetre* fen)
{
        if (client == NULL)
        {
                fprintf(stderr, "go_back_n_ecoute: client NULL.\n");
                exit(1);
        }
}*/

int main(int argc, char** argc)
{
        check_args_dst(argc, argv);

        srand(time(NULL));
        //Adresses
        struct sockaddr_in addrServeur, addrClient;
        init_addr(&addrServeur, NULL, argv[3]);
        init_addr(&addrClient, argv[2], argv[4]);
        //Sockets
        int sockServeur = socket(PF_INET, SOCK_DGRAM, 0);
        if (bind(sockServeur, (struct sockaddr*)&addrServeur,
                 sizeof(addrServeur)) == -1)
        {
                tue_moi("main, bind serveur", 1, sockServer);
        }
        int sockClient = socket(PF_INET, SOCK_DGRAM, 0);
        if (bind(sockClient, (struct sockaddr*)&addrClient,
                 sizeof(addrClient)) == -1)
        {
                tue_moi("main, bind client", 2, sockServer, sockClient);
        }
        int mode = 0;
        //Fenetre
        fenetre fenetre = {0,0,1,0};


        negociation_dst(&sockServer, &sockClient, &addrClient, &fenetre, &mode);
        return 0;
}

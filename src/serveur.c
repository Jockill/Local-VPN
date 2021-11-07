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
    if(argc < 3){
        fprintf(stderr,"Erreur : Argument manquant.\n");
        fprintf(stderr,"Syntaxe attendu : ./destination <IP_distante> <port_local> <port_ecoute_dst_pertubateur>\n");
        exit(1);
    } else if(argc > 3){
        fprintf(stderr,"Erreur : Trop d'argument.\n");
        fprintf(stderr,"Syntaxe attendu : ./destination <IP_distante> <port_local> <port_ecoute_dst_pertubateur>\n");
        exit(1);
    }
    if(!ipv4_valide(argv[1])){
        fprintf(stderr,"Erreur : IP non valide.\n");
        fprintf(stderr,"Rappel Format IPV4 : xxx.xxx.xxx.xxx .\n");
        exit(1);
    }
    for(int i = 2; i<=3;i++){
        int testPort = strtol(argv[i],NULL,0);
        if(testPort <2049 || testPort> 49151){
            fprintf(stderr,"Erreur : numéro de port non valide.\n");
            fprintf(stderr,"Rappel port : 2048 < port < 49151.\n");
            exit(1);
        }
    }
    return;
}

//??
/*void check_args_src(int argc, char** argv){



    int mode = strtol(argv[1],NULL,0);
    if(mode != STOP_N_WAIT && mode != GO_BACK_N){
        fprintf(stderr,"Erreur : mode non valide.\n");
        fprintf(stderr,"Rappel mode : 1=\'stop and wait\', 2=\'go back n\'.\n");
        exit(1);
    }

}*/


//Pour pouvoir gérer le multiflux, il faudra utiliser FD_SET et jouer avec les
//fd pour savoir quel flux est pret.
int negociation_dst(int* sockServer, int* sockClient,
                    struct sockaddr_in* addrClient, fenetre* fenetre, int* mode)
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
        socklen_t lenAddrClient = sizeof(&addrClient);
        unsigned char type = 0;
        // fd_set sockSet;
        // FD_ZERO(&sockSet);
        // FD_SET(sockServer, &sockSet);

        //reception SYN
        while ((paquet.type != SYN) && (tmp != 52))
        {
                tmp = recvfrom(sockClient, (void*)&paquet, TAILLE_PAQUET, NULL,
                                  (struct sockaddr*)addrClient, lenAddrClient);
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
                tmp = sendto(sockClient, (void*)&paquet, TAILLE_PAQUET, NULL,
                            (struct sockaddr*)addrClient, lenAddrClient);
                if (tmp == -1)
                        tue_moi("negociation_dst, sendto", 2,
                                sockServer, sockClient);
        }

        //Reception ACK
        tmp = 0;
        paquet = cree_paquet(0,0,0,0,0,0,NULL);
        while((paquet.type != ACK) && (tp != 52))
        {
                tmp = recvfrom(sockClient, (void*)&paquet, TAILLE_PAQUET, NULL,
                              (struct sockaddr*)addrClient, lenAddrClient);
                if (tmp == -1)
                        tue_moi("negociation_dst, recvfrom", 2,
                                sockClient, sockServer);
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
        int sockClient = socket(PF_INET, SOCK_DGRAM, 0);
        if(sockClient == -1){
                tue_moi("socket",0);
        }
        int sockServeur = socket(PF_INET, SOCK_DGRAM, 0);
        if(sockServeur == -1){
                tue_moi("socket",1,sockClient);
        }
        if (bind(sockServeur, (struct sockaddr*)&addrServeur,
                 sizeof(addrServeur)) == -1)
        {
                tue_moi("main, bind serveur", 2,sockClient, sockServer);
        }        
        int mode = 0;
        //Fenetre
        fenetre fenetre = {0,0,1,0};


        negociation_dst(&sockServer, &sockClient, &addrClient, &fenetre, &mode);
        return 0;
}

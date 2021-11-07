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
    if(argc < 4){
        fprintf(stderr,"Erreur : Argument manquant.\n");
        fprintf(stderr,"Syntaxe attendu : ./destination <IP_distante> <port_local> <port_ecoute_dst_pertubateur>\n");
        exit(1);
    } else if(argc > 4){
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

//Pour pouvoir gérer le multiflux, il faudra utiliser FD_SET et jouer avec les
//fd pour savoir quel flux est pret.
int negociation_dst(int* sockServer, int* sockClient,
                    struct sockaddr_in* addrClient, fenetre* fen, int* mode)
{
        //Preconditions
        if (*sockClient < 0)
        {
                fprintf(stderr, "negociation_dst: sockClient < 0.\n");
                exit(1);
        }
        if (*sockServer < 0)
        {
                fprintf(stderr, "negociation_dst: sockServer < 0.\n");
                exit(1);
        }
        if (addrClient == NULL)
                tue_moi("negociation_dst: client NULL", 2, *sockClient,
                        *sockServer);
        short randAck =(short) rand();
        paquet paquetEnv = {0};
        paquet paquetRecv= {0};
        ssize_t tmp = 0;
        socklen_t lenAddrClient = sizeof(&addrClient);
        fd_set sockSet;

        //reception SYN
        while ((paquetRecv.type != SYN) && (tmp != 52))
        {
                tmp = recvfrom(*sockServer, (void*)&paquetRecv, TAILLE_PAQUET, 0,
                                  (struct sockaddr*)addrClient, &lenAddrClient);
                if (tmp == -1)
                        tue_moi("negociation_dst, recvfrom", 2,
                                *sockServer, *sockClient);
        }

        //Configuration du mode
        if (paquetRecv.tailleFenetre == 0)
                *mode = STOP_N_WAIT;
        else
                *mode = GO_BACK_N;

        //envoi SYN-ACK
        paquetEnv = cree_paquet(paquetRecv.idFlux, SYN+ACK, randAck,
                             paquetRecv.numSeq+1, 0, fen->tailleEnvoi, NULL);
        tmp = 0; // on reinitialise tmp
        while((paquetRecv.type != ACK) && (tmp != 52) && (paquetRecv.numAck != randAck+1))
        {
                tmp = sendto(*sockClient, (void*)&paquetEnv, TAILLE_PAQUET, 0,
                            (struct sockaddr*)addrClient, lenAddrClient);
                if (tmp == -1)
                        tue_moi("negociation_dst, sendto", 2,
                                *sockServer, *sockClient); 
                //Reception ACK               
                FD_ZERO(&sockSet);
                FD_SET(*sockServer, &sockSet);
                struct timeval timer = {1,0};
                if(select(FD_SETSIZE,&sockSet,NULL,NULL,&timer) ==-1)
                        tue_moi("sendto",2,*sockServer,*sockClient);
                
                if(FD_ISSET(*sockServer,&sockSet)){
                        if((tmp = recvfrom(*sockClient,(void *)&paquetRecv,TAILLE_PAQUET,0,
                                          (struct sockaddr*)addrClient,&lenAddrClient))==-1)
                                tue_moi("sendto",2,*sockServer,*sockClient);
                }
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

int main(int argc, char** argv)
{
        check_args_dst(argc, argv);

        srand(time(NULL));
        //Adresses
        struct sockaddr_in addrServeur, addrClient;
        init_addr(&addrServeur, NULL, argv[2]);
        init_addr(&addrClient, argv[1], argv[3]);
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
                tue_moi("main, bind serveur", 2,sockClient, sockServeur);
        }        
        int mode = 0;
        //Fenetre
        fenetre fen = {0,0,TAILLE_FENETRE_SERVEUR,0};


        negociation_dst(&sockServeur, &sockClient, &addrClient, &fen, &mode);
        return 0;
}

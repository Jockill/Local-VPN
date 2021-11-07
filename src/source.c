#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "../head/utils.h"

void check_args_src(int argc, char** argv){

    if(argc < 5){
        fprintf(stderr,"Erreur : Argument manquant.\n");
        fprintf(stderr,"Syntaxe attendu : ./source <mode> <IP_distante> <port_local> <port_ecoute_src_pertubateur>\n");
        exit(1);
    } else if(argc > 5){
        fprintf(stderr,"Erreur : Trop d'argument.\n");
        fprintf(stderr,"Syntaxe attendu : ./source <mode> <IP_distante> <port_local> <port_ecoute_src_pertubateur>\n");
        exit(1);
    }
    if(!ipv4_valide(argv[2])){
        fprintf(stderr,"Erreur : IP non valide.\n");
        fprintf(stderr,"Rappel Format IPV4 : xxx.xxx.xxx.xxx .\n");
        exit(1);
    }
    int mode = strtol(argv[1],NULL,0);
    if(mode != STOP_N_WAIT && mode != GO_BACK_N){
        fprintf(stderr,"Erreur : mode non valide.\n");
        fprintf(stderr,"Rappel mode : 1=\'stop and wait\', 2=\'go back n\'.\n");
        exit(1);
    }
    for(int i = 3; i<=4;i++){
        int testPort = strtol(argv[i],NULL,0);
        if(testPort <2049 || testPort> 49151){
            fprintf(stderr,"Erreur : numéro de port non valide.\n");
            fprintf(stderr,"Rappel port : 2048 < port < 49151.\n");
            exit(1);
        }
    }
    return;
}

int negociation_src(int sockServeur, int sockClient,struct sockaddr_in* serveur, int mode, fenetre* fen){
    unsigned short numA =(unsigned short) rand();
    unsigned short numB;
    ssize_t tmp;
    socklen_t tailleServ = sizeof(*serveur);

    if(mode != GO_BACK_N && mode != STOP_N_WAIT){
        fprintf(stderr,"mode incorrecte\n");
        close(sockClient);
        close(sockServeur);
        exit(1);
    }
    paquet premierHandShake = cree_paquet(0,SYN,numA,0,0,mode-1,NULL);
    //On donne le mode de la communication en utilisisant habilement le champ taille fenetre
    //Si la taille de la fenetre est a 0 quand le client ouvre la connection alors
    //la communication sera en stop and wait et si elle est a 1 alors go back n
    paquet ack;
    fd_set acquittement;
    int ack1Recu = 0;
    while(!ack1Recu){
        if((tmp = sendto(sockServeur,(void *)&premierHandShake,
            sizeof(premierHandShake),0,(struct sockaddr*)serveur,tailleServ)) == -1){
            tue_moi("sendto",2,sockServeur,sockClient);
        }
        affiche_paquet(&premierHandShake);
        printf("1\n");
        if(tmp !=52) continue; //si on a pas reussi a toute envoyer on renvoie
        FD_ZERO(&acquittement);
        FD_SET(sockClient,&acquittement);
        struct timeval timer = {5,0};
        if(select(FD_SETSIZE,&acquittement,NULL,NULL,&timer) ==-1){
            tue_moi("sendto",2,sockServeur,sockClient);
        }
        if(FD_ISSET(sockClient,&acquittement)){
            if((tmp = recvfrom(sockClient,(void *)&ack,52,0,(struct sockaddr*)serveur,&tailleServ))==-1){
                tue_moi("sendto",2,sockServeur,sockClient);
            }
            affiche_paquet(&ack);
            printf("2\n");
            if(tmp != 52) continue; //si on a pas reussi a tout recevoir
            if((ack.type & (SYN|ACK))==0) continue;
            printf("2.5\n");
            if(ack.numAck != numA+1) continue;
            printf("3\n");
            numB = ack.numSeq;
            fen->tailleEnvoi= ack.tailleFenetre;
            ack1Recu=1;
        }
    }
    paquet dernierHandShake = cree_paquet(0,ACK,numA+1,numB+1,0,0,NULL);
    if((tmp = sendto(sockServeur,(void*)&dernierHandShake,
            52,0,(struct sockaddr*)serveur,tailleServ)) == -1){
            tue_moi("sendto",2,sockServeur,sockClient);
    }
    if(tmp !=52){
        close(sockClient);
        close(sockServeur);
        fprintf(stderr,"plantage handsake\n");
        exit(1);
    }

    return 0;
}

int main(int argc, char** argv){
    check_args_src(argc,argv);

    int mode = strtol(argv[1],NULL,0);
    fenetre fen;
    fen.debut = fen.fin = 0;
    struct sockaddr_in serveur, client;

    srand(time(NULL));
    init_addr(&serveur,argv[2],argv[4]);
    init_addr(&client,NULL,argv[3]);
    int sockServeur = socket(PF_INET,SOCK_DGRAM,0); // socket avec laquelle j'envoie
    if(sockServeur == -1){
        tue_moi("socket",0);
    }
    int sockClient = socket(PF_INET,SOCK_DGRAM,0); // socket avec laquelle je reçoit
    if(sockClient == -1){
        tue_moi("socket",1,sockClient);
    }
    if(bind(sockClient,(struct sockaddr*)&client,sizeof(client))==-1){
        tue_moi("bind",0);
    }
    negociation_src(sockServeur,sockClient,&serveur,mode,&fen);
}

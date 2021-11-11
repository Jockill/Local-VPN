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
        fprintf(stderr,"Syntaxe attendu : ./%s <mode> <IP_distante> <port_local> <port_ecoute_src_pertubateur>\n",argv[0]);
        exit(1);
    } else if(argc > 5){
        fprintf(stderr,"Erreur : Trop d'argument.\n");
        fprintf(stderr,"Syntaxe attendu : ./%s <mode> <IP_distante> <port_local> <port_ecoute_src_pertubateur>\n",argv[0]);
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

unsigned short negociation_src(int sockClient,struct sockaddr_in * serveur, int mode, fenetre* fen){
    unsigned short numA =(unsigned short) rand();
    unsigned short numB;
    ssize_t tmp;

    if(mode != GO_BACK_N && mode != STOP_N_WAIT){
        fprintf(stderr,"mode incorrecte\n");
        close(sockClient);
        exit(1);
    }
    paquet premierHandShake = cree_paquet(0,SYN,numA,0,0,mode-1,NULL);
    //On donne le mode de la communication en utilisisant habilement le champ taille fenetre
    //Si la taille de la fenetre est a 0 quand le client ouvre la connection alors
    //la communication sera en stop and wait et si elle est a 1 alors go back n
    paquet ack;
    int ack1Recu = 0;
    while(!ack1Recu){
        if((tmp = sendto(sockClient,(void *)&premierHandShake,
            TAILLE_PAQUET,0,(struct sockaddr *)serveur,TAILLE_ADRESSE)) == -1){
            tue_moi("sendto",1,sockClient);
        }
        affiche_paquet(&premierHandShake);
        if(tmp !=TAILLE_PAQUET) continue; //si on a pas reussi a toute envoyer on renvoie
        
        if(attend_paquet(sockClient,(struct sockaddr *)serveur,&ack) == 0) continue;
        if((ack.type & (SYN|ACK))==0) continue;
        if(ack.numAck != numA+1) continue;
        numB = ack.numSeq;
        fen->tailleEnvoi= ack.tailleFenetre;
        ack1Recu=1;
    }
    paquet dernierHandShake = cree_paquet(0,ACK,numA+1,numB+1,0,0,NULL);
    if((tmp = sendto(sockClient,(void *)&dernierHandShake,
            TAILLE_PAQUET,0,(struct sockaddr *)serveur,TAILLE_ADRESSE)) == -1){
            tue_moi("sendto",1,sockClient);
    }
    if(tmp !=TAILLE_PAQUET){
        close(sockClient);
        fprintf(stderr,"plantage handsake\n");
        exit(1);
    }

    return numA+1;
}

void fin_src(int sockClient,struct sockaddr_in * serveur,unsigned short numSec){
    paquet premierHandShake = cree_paquet(0,FIN,numSec,0,0,0,NULL);
    int numSeqack;
    paquet ack;
    int ack1recu = 0;
    int partiel=0;
    int compteur=0; //on ne tente d'envoyer notre FIN un certain nombre de fois avant d'abandonner
    while(!ack1recu && compteur<5){
        if((partiel = sendto(sockClient,(void *)&premierHandShake,
            sizeof(premierHandShake),0,(struct sockaddr *)serveur,TAILLE_ADRESSE)) == -1){
            tue_moi("sendto",1,sockClient);
        }
        if(partiel != TAILLE_PAQUET) continue;

        if((attend_paquet(sockClient,(struct sockaddr*)serveur,&ack)==0)
        || ((ack.type & (FIN|ACK))==0) || (ack.numAck != numSec+1)){ 
            compteur++;
            continue;
        }
        numSeqack=ack.numSeq;
        ack1recu=1;
    }
    paquet dernierHandShake = cree_paquet(0,ACK,0,numSeqack+1,0,0,NULL);
    if((partiel = sendto(sockClient,(void *)&dernierHandShake,
            TAILLE_PAQUET,0,(struct sockaddr *)serveur,TAILLE_ADRESSE)) == -1){
            tue_moi("sendto",1,sockClient);
    }
    if(partiel !=TAILLE_PAQUET){
        close(sockClient);
        fprintf(stderr,"plantage fin\n");
        exit(1);
    }
    return;
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
    int sockClient = socket(PF_INET,SOCK_DGRAM,0); // socket avec laquelle je reçoit
    if(sockClient == -1){
        tue_moi("socket",1,sockClient);
    }
    if(bind(sockClient,(struct sockaddr *)&client,sizeof(client))==-1){
        tue_moi("bind",0);
    }
    
    
    unsigned short premierNumSeq = negociation_src(sockClient,&serveur,mode,&fen)+1; //temporaire
    printf("negociation reussite\n");
    sleep(2);
    fin_src(sockClient,&serveur,premierNumSeq);
    printf("fin reussi!\n");
    return 0;
}

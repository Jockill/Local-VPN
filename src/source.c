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
#include "../head/fifo.h"

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

uint16_t negociation_src(int sockClient,struct sockaddr_in * serveur, int mode, fenetre* fen){
    uint16_t numA =(uint16_t) rand();
    uint16_t numB;
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
        if(tmp !=TAILLE_PAQUET) continue; //si on a pas reussi a toute envoyer on renvoie

        if(attend_paquet(sockClient,(struct sockaddr *)serveur,&ack) == 0) continue;
        if((ack.type & (SYN|ACK))==0) continue;
        if(ack.numAck != numA+1) continue;
        numB = ack.numSeq;
        fen->tailleEnvoi= ack.tailleFenetre*52;
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

void fin_src(int sockClient,struct sockaddr_in * serveur,uint16_t numSec){
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

void stop_and_wait(int socket,struct sockaddr_in * sevreur){
    uint16_t Seq = 0;
    int fin = 0;
    paquet paquetEnv;
    paquet paquetRecv = {0};

    while(!fin){
        paquetEnv= cree_paquet(0,DATA,Seq,0,0,0,NULL);
        int ackRecu = 0;
        while(!ackRecu){
            if(!envoie_paquet(socket,(struct sockaddr*)sevreur,&paquetEnv)){
                continue;
            }
            if(attend_paquet(socket,(struct sockaddr*)sevreur,&paquetRecv)==0){
                continue;
            }
            if(paquetRecv.type == (SYN|ACK)){ //mon ACK du handshake c'est perdu
                paquet finHandShake=cree_paquet(0,ACK,
                                    paquetRecv.numAck+1,
                                    paquetRecv.numSeq+1,0,0,NULL);
                //on renvoie le ACK;
                envoie_paquet(socket,(struct sockaddr*)sevreur,&finHandShake);
                continue;
            }else if(paquetRecv.type != ACK || paquetRecv.numAck != Seq){
                continue;
            }
            ackRecu = 1;
            Seq = (Seq+1)%2;
        }
        //mettre une condition d'aret pour fin
        //envoyer des données a un moment
    }
    fin_src(socket,sevreur,Seq);
    return;
}

void go_back_n(int socket, struct sockaddr_in * serveur, fenetre *fen,uint16_t premierNumSeq){
    uint16_t PNSNA = premierNumSeq; //Premier Numéro de séquence envoyé et non acquité
    uint16_t PNSU = premierNumSeq; //Premier numéro de séquence utilisable

    int fin = 0;
    paquet *paquetEnv;
    paquet paquetRecv = {0};
    uint16_t lastNumAckRecv = 0;
    int ackDupilque = 0;

    fifo * tampon =  cree_fifo();
    if(!tampon) tue_moi("creation tampon", 1, socket);
    fen->tailleCongestion = 52;

    while(!fin){
        unsigned int tailleFenetreReel=
                    (fen->tailleCongestion<=fen->tailleEnvoi)?
                    fen->tailleCongestion:fen->tailleEnvoi;
        printf("taille fenetre effective : %d\n",tailleFenetreReel);
        for(;PNSU< ((uint16_t) (PNSNA + (uint16_t) (tailleFenetreReel/52)));PNSU++){ // si il reste de la place dans ma fenetre
            paquetEnv = cree_paquet_gbn(0,DATA,PNSU,0,0,0,NULL);
            if(envoie_paquet(socket,(struct sockaddr*)serveur,paquetEnv)==0){
                tue_moi("envoie_paquet",1,socket);
            }else if(push_fifo(tampon,paquetEnv) == -1)tue_moi("push fifo",1,socket);
            //j'envoie un paquet (que je stock quelque part);
        }
        affiche_fifo(tampon);
        printf("PNSU : %hu,PNSNA : %hu\n",PNSU,PNSNA);

        if(attend_paquet(socket,(struct sockaddr *)serveur,&paquetRecv)==0){
            fen->tailleCongestion /= 2;
            envoi_fifo(tampon,socket,serveur);

            //on a eu une timeout et il faut renvoyer tous les paquets stocker et divisé tailleCongestion par 2
        }else if(paquetRecv.type==ACK){

            if(paquetRecv.ecn > 0){
                fen->tailleCongestion -= (fen->tailleCongestion/10);
            } // si le bit ECN est pas 0 la taille de la FC perd 10%

            if(paquetRecv.numAck==PNSNA+1){
                fen->tailleCongestion +=TAILLE_PAQUET;
                free(pop_fifo(tampon));
                PNSNA++;
                printf("test,1 2, test");
                continue;
            } // si on a reçu un acquitement en séquence on rajoute un paquet
            printf("numéro de ack recu %hu\n",paquetRecv.numAck);
            for(;PNSNA<paquetRecv.numAck;PNSNA++){
                printf("test\n");
                free(pop_fifo(tampon));
            } // acquiter tous les paquets 

            if(paquetRecv.numAck == lastNumAckRecv){
                ackDupilque++;
            }else{
                lastNumAckRecv = paquetRecv.numAck;
                ackDupilque=0;
            }
            if(ackDupilque == 3){
                printf("ack dupiqué\n");
                envoi_fifo(tampon,socket,serveur);
                fen->tailleCongestion = 52;
                ackDupilque=0;
            }
            //on check le numéro d'acquitement et on valide tous les paquets jusqu'au numAck+1
            //si le numéro d'acquitement est égale au premier seq stocker alors tailleCongestion++
            //si j'ai reçu trois fois le même acquis de suite je fais tailleCongestion = 52
        }

        if(fen->tailleCongestion<52) fen->tailleCongestion=52;
        //mettre une condition d'arete
    }
    fin_src(socket,serveur,PNSNA);
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
    
    uint16_t premierNumSeq = negociation_src(sockClient,&serveur,mode,&fen); //temporaire
    printf("negociation reussite\n");
    sleep(2);
    go_back_n(sockClient,&serveur,&fen,premierNumSeq);
    return 0;
}

#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

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

uint16_t negociation_src(int sockClient,struct sockaddr_in * serveur, int mode, fenetre * fen){
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
    uint16_t seq = 0;
    int fin = 0;
    paquet paquetEnv;
    paquet paquetRecv = {0};

    int fichier = open("./src/source.c", O_RDONLY);
    if(fichier == -1)
        tue_moi("go_back_n: open", 1, socket);

    while(!fin){
	int tmp = 0;
	char buf[TAILLE_DONNEES] = {0};
	int ackRecu = 0;

	tmp = read(fichier,buf,TAILLE_DONNEES);
	if(tmp ==-1)
	    tue_moi("go_back_n: read", 2, fichier, socket);
	//Si il n'y a plus rien a lire
	if (tmp == 0){
	    fin = 1;
	    break;
	}
	if (tmp < TAILLE_DONNEES)
		buf[tmp] = '\0';

        paquetEnv= cree_paquet(0,DATA,seq,0,0,0,buf);

        while(!ackRecu){
            if(!envoie_paquet(socket,(struct sockaddr*)sevreur,&paquetEnv))
                continue;

            if(attend_paquet(socket,(struct sockaddr*)sevreur,&paquetRecv)==0)
                continue;
	    //Le ACK du handshake est perdu donc on le renvoie
            if(paquetRecv.type == (SYN|ACK)){
                paquet finHandShake=cree_paquet(0,ACK,
                                    paquetRecv.numAck+1,
                                    paquetRecv.numSeq+1,0,0,NULL);
                //on renvoie le ACK;
                envoie_paquet(socket,(struct sockaddr*)sevreur,&finHandShake);
                continue;
	    }else if(paquetRecv.type != ACK || paquetRecv.numAck != seq){
                continue;
            }
            ackRecu = 1;
            seq = (seq+1)%2;
        }
    }
    fin_src(socket,sevreur,seq);
    return;
}

void go_back_n(int socket, struct sockaddr_in * serveur, fenetre *fen,uint16_t premierNumSeq){
    uint16_t PNSNA = premierNumSeq; //Premier Numéro de séquence envoyé et non acquité
    uint16_t PNSU = premierNumSeq; //Premier numéro de séquence utilisable

    int fin = 0;
    paquet * paquetEnv;
    paquet paquetRecv = {0};
    uint16_t lastNumAckRecv = 0;
    int ackDupilque = 0;
    /* CONTROLE DU TEMPS*/
    clock_t temps = clock();

    int fichier = open("./src/source.c",O_RDONLY);
    if(fichier == -1)
	tue_moi("go_back_n: open", 1, socket);

    fifo * tampon = cree_fifo();
    if(!tampon)
    	tue_moi("go_back_n: creation tampon", 1, socket);
    fen->tailleCongestion = TAILLE_PAQUET;

    while(!fin){
        //taille envoie est récuperé pendant l'handshake depusi la destination
        unsigned int tailleFenetreReel=
                    (fen->tailleCongestion<=fen->tailleEnvoi)?
                    fen->tailleCongestion:fen->tailleEnvoi;

	fprintf(stderr, "%ld %d\n", clock()-temps, fen->tailleCongestion);

	//temps qu'il reste de la place dans ma fenetre
        for(;PNSU< ((uint16_t) (PNSNA + (uint16_t) (tailleFenetreReel/52)));PNSU++)
	{

	    int tmp = 0;
	    char buf[TAILLE_DONNEES] = {0};
	    tmp = read(fichier,buf,TAILLE_DONNEES);
	    if(tmp ==-1)
                tue_moi("go_back_n: read", 2, fichier, socket);
	    if (tmp == 0)
	    {
		fin = 1;
		break;
	    }

            paquetEnv = cree_paquet_gbn(0,DATA,PNSU,0,0,0,buf);


	    // Si l'envoi à échoué, kill
            if(envoie_paquet(socket,(struct sockaddr*)serveur,paquetEnv)==0){
                tue_moi("envoie_paquet",1,socket);
	    // Sinon mettre le paquet dans la file pour permettre un renvoi
            }else if(push_fifo(tampon,paquetEnv) == -1)
                tue_moi("push fifo",1,socket);
        }

        //on a eu une timeout et il faut renvoyer tous les paquets stocker et divisé tailleCongestion par 2
        if(attend_paquet(socket,(struct sockaddr *)serveur,&paquetRecv)==0){
            fen->tailleCongestion /= 2;
            envoi_fifo(tampon,socket,serveur);

        }else if(paquetRecv.type==ACK){

	    // si le bit ECN est pas 0 la taille de la FC perd 10%
            if(paquetRecv.ecn > 0){
                fen->tailleCongestion -= (fen->tailleCongestion/10);
            }

	    // si on a reçu un acquitement en séquence on augment la taille
	    //de la fenetre de congestion de TAILLE_PAQUET
            if(paquetRecv.numAck==PNSNA+1){
                fen->tailleCongestion +=TAILLE_PAQUET;
                free(pop_fifo(tampon));
                PNSNA++;
                continue;
            }

	    // acquiter tous les paquets jusqu'a l'acquis recu - 1
            for(;PNSNA<paquetRecv.numAck;PNSNA++){
                free(pop_fifo(tampon));
            }

            if(paquetRecv.numAck == lastNumAckRecv){
                ackDupilque++;
            }else{
                lastNumAckRecv = paquetRecv.numAck;
                ackDupilque=0;
            }
	    //si j'ai reçu trois fois le même acquis de suite je fais tailleCongestion = 52
            if(ackDupilque == 3){
                envoi_fifo(tampon,socket,serveur);
                fen->tailleCongestion = TAILLE_PAQUET;
                ackDupilque=0;
            }
    	}else if (paquetRecv.type == (SYN|ACK)){
	    paquet finHandShake=cree_paquet(0,ACK,
				paquetRecv.numAck+1,
				paquetRecv.numSeq+1,0,0,NULL);
	    //on renvoie le ACK;
	    envoie_paquet(socket,(struct sockaddr*)serveur,&finHandShake);
	    envoi_fifo(tampon, socket, serveur);
	}


        if(fen->tailleCongestion<TAILLE_PAQUET)
	    fen->tailleCongestion=TAILLE_PAQUET;

	// TODO mettre une condition d'arete
    }
    fin_src(socket,serveur,PNSNA);
}


int main(int argc, char** argv){
    check_args_src(argc,argv);
    srand(time(NULL));

    int mode = strtol(argv[1],NULL,0);
    fenetre fen;
    fen.debut = fen.fin = 0;
    struct sockaddr_in serveur, client;
    init_addr(&serveur,argv[2],argv[4]);
    init_addr(&client,NULL,argv[3]);

    // Socket de reception
    int sockClient = socket(PF_INET,SOCK_DGRAM,0);
    if(sockClient == -1)
        tue_moi("socket",1,sockClient);
    if(bind(sockClient,(struct sockaddr *)&client,sizeof(client))==-1)
        tue_moi("bind",0);


    uint16_t premierNumSeq = negociation_src(sockClient,&serveur,mode,&fen);
    if (mode == GO_BACK_N)
	    go_back_n(sockClient,&serveur, &fen, premierNumSeq);
    else if (mode == STOP_N_WAIT)
    	    stop_and_wait(sockClient, &serveur);

    return 0;
}

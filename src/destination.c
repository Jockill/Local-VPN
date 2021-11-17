#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "../head/utils.h"


void check_args_dst(int argc, char** argv)
{
    if(argc < 4){
        fprintf(stderr,"Erreur : Argument manquant.\n");
        fprintf(stderr,"Syntaxe attendu : %s <IP_distante> <port_local> <port_ecoute_dst_pertubateur>\n", argv[0]);
        exit(1);
    } else if(argc > 4){
        fprintf(stderr,"Erreur : Trop d'argument.\n");
        fprintf(stderr,"Syntaxe attendu : %s <IP_distante> <port_local> <port_ecoute_dst_pertubateur>\n", argv[0]);
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
uint16_t negociation_dst(int* sockServer,
                    struct sockaddr_in* addrClient, fenetre* fen, int* mode)
{
        //Preconditions
        if ((sockServer == NULL) || (*sockServer < 0))
        {
                fprintf(stderr, "negociation_dst: sockServer < 0.\n");
                exit(1);
        }
        if (addrClient == NULL)
	{
		fprintf(stderr, "negociation_dst: client NULL");
		close(*sockServer);
		exit(1);
	}

        uint16_t randAck =(uint16_t) rand();
        paquet paquetEnv = {0};
        paquet paquetRecv= {0};
        ssize_t tmp = 0;
        socklen_t lenAddrClient = sizeof(*addrClient);

        //reception SYN
        while ((paquetRecv.type != SYN) || (tmp != 52))
        {
                tmp = recvfrom(*sockServer, (void*)&paquetRecv, TAILLE_PAQUET, 0,
                                  (struct sockaddr*)addrClient, &lenAddrClient);
                if (tmp == -1)
                        tue_moi("negociation_dst, recvfrom", 1, *sockServer);
        }

        //Configuration du mode
        if (paquetRecv.tailleFenetre == 0)
                *mode = STOP_N_WAIT;
        else
                *mode = GO_BACK_N;

        //envoi SYN-ACK et réception ACK
        paquetEnv = cree_paquet(paquetRecv.idFlux, SYN+ACK, randAck,
                             paquetRecv.numSeq+1, 0, fen->tailleEnvoi, NULL);
        tmp = 0; // on reinitialise tmp
        int ackRecu = 0;
        while (!ackRecu)
        {
                tmp = sendto(*sockServer, (void*)&paquetEnv, TAILLE_PAQUET, 0,
                            (struct sockaddr*)addrClient, lenAddrClient);
                if (tmp == -1)
                        tue_moi("negociation_dst, sendto", 1, *sockServer);

                //Reception ACK
                if(attend_paquet(*sockServer,(struct sockaddr*)addrClient,&paquetRecv)==0
                || paquetRecv.type != ACK || (paquetRecv.numAck != randAck+1)){
                        continue;
                }
                ackRecu=1;
        }
        return paquetRecv.numSeq;
}

void fin_dst(int* sockClient, struct sockaddr_in* addrClient)
{
	//Preconditions
	if ((sockClient == NULL) || (*sockClient < 0))
	{
		fprintf(stderr, "fin_dst: sockClient 0\n");
		exit(1);
	}
	if (addrClient == NULL)
	{
		fprintf(stderr, "fin_dst: addrClient NULL\n");
		close(*sockClient);
		exit(1);
	}

	paquet paquetRecv = {0};
	paquet paquetEnv = {0};
	int compteur = 0;

	//Envoi ACK + FIN et réception ACK
	paquetEnv = cree_paquet(paquetRecv.idFlux, ACK+FIN, 0,
                                paquetRecv.numSeq+1, 0, 0, NULL);
        int ackRecu=0;
	while ((!ackRecu && compteur<5))
	{
		//Envoi ACK+FIN
		if(sendto(*sockClient, (void*)&paquetEnv, TAILLE_PAQUET, 0,
                            (struct sockaddr*)addrClient, TAILLE_ADRESSE)==-1){
                        tue_moi("sendto",1,sockClient);
                }

		//Reception ACK
                if (attend_paquet(*sockClient, (struct sockaddr*)addrClient,
                                  &paquetRecv) == 0
                    || paquetRecv.type != ACK){
                        compteur++;
                        continue;
                }
                ackRecu=1;
	}
}

void stop_and_wait_ecoute(int socket,struct sockaddr_in* client)
{
        //Preconditions
        if (client == NULL)
        {
                fprintf(stderr, "stop_and_wait_ecoute: client NULL.\n");
                exit(1);
        }

        uint16_t lastNumSeq =-1;
        socklen_t taille = TAILLE_ADRESSE;
        paquet paquetEnv = {0};
        paquet paquetRecv = {0};
	int fichier = open("./transfert/destination.txt", O_CREAT|O_RDWR|O_TRUNC, 0666);
	if (fichier == -1)
		tue_moi("stop_and_wait_ecoute: open", 1, socket);

        while(paquetRecv.type != FIN){

		// TODO selectionner flux

                if(recvfrom(socket,&paquetRecv,TAILLE_PAQUET,0,
                            (struct sockaddr*)client,&taille)==-1){
                        tue_moi("stop and wait dest: recv",1,socket);
                }
                //Les numeros de sequence sont des 0 et des 1 alternés
                //Donc le dernier numero doit être l'inverse binaire du suivant
                if(lastNumSeq != paquetRecv.numSeq)
		{

                        lastNumSeq=paquetRecv.numSeq;
			//Pas tres beau mais évite de faire plusieurs appels
			int len = strlen(paquetRecv.donnees);
			if (len > TAILLE_DONNEES)
				len = TAILLE_DONNEES;

			if (write(fichier, paquetRecv.donnees, len) == -1)
				tue_moi("go_back_n_ecoute, write", 2, socket, fichier);
                }

		paquetEnv = cree_paquet(0,ACK,0,paquetRecv.numSeq,0,0,NULL);
                envoie_paquet(socket,(struct sockaddr*)client,&paquetEnv);
        }

        //TODO Reconstituer les flux

        fin_dst(&socket,client);
        return;
}

void go_back_n_ecoute(int socket,struct sockaddr_in* client,uint16_t premierNumSeq)
{
	//Precondition
        if (client == NULL)
        {
                fprintf(stderr, "go_back_n_ecoute: client NULL.\n");
                exit(1);
        }

	uint16_t nextNumSeq = premierNumSeq;
        socklen_t taille = TAILLE_ADRESSE;
        paquet paquetEnv;
        paquet paquetRecv = {0};
	int fichier = open("./transfert/destination.txt", O_CREAT|O_RDWR|O_TRUNC, 0666);
	if (fichier == -1)
		tue_moi("go_back_n_ecoute: open", 1, socket);


        while(paquetRecv.type != FIN)
	{
                if(recvfrom(socket,&paquetRecv,TAILLE_PAQUET,0,
                            (struct sockaddr*)client,&taille)==-1){
                        tue_moi("stop and wait : recv",1,socket);
                }

                if((nextNumSeq) == paquetRecv.numSeq){
                        nextNumSeq = (nextNumSeq+1);

			int len = strlen(paquetRecv.donnees);
			if (len > TAILLE_DONNEES)
				len = TAILLE_DONNEES;
			if (write(fichier, paquetRecv.donnees, len) == -1)
				tue_moi("go_back_n_ecoute, write", 2, socket, fichier);
                }

		paquetEnv = cree_paquet(0,ACK,0,nextNumSeq,paquetRecv.ecn,0,NULL);
                envoie_paquet(socket,(struct sockaddr*)client,&paquetEnv);
        }
        fin_dst(&socket,client);
}

int main(int argc, char** argv)
{
        check_args_dst(argc, argv);

	//Rand
        srand(time(NULL));
        //Adresses
        struct sockaddr_in addrServeur, addrClient;
        init_addr(&addrServeur, NULL, argv[2]);
        init_addr(&addrClient, argv[1], argv[3]);
        //Sockets
        int sockServeur = socket(PF_INET, SOCK_DGRAM, 0);
        if(sockServeur == -1)
                tue_moi("main, socket", 0);
        if (bind(sockServeur, (struct sockaddr*)&addrServeur,
                 sizeof(addrServeur)) == -1)
                tue_moi("main, bind serveur", 1, sockServeur);
        //Fenetre
        fenetre fen = {0,0,TAILLE_FENETRE_SERVEUR,0};
	//Misc
	int mode = 0;

	uint16_t lastSeq = negociation_dst(&sockServeur, &addrClient, &fen, &mode);
	if (mode == STOP_N_WAIT)
		stop_and_wait_ecoute(sockServeur, &addrClient);
	else if (mode == GO_BACK_N)
	        go_back_n_ecoute(sockServeur,&addrClient, lastSeq);

        return 0;
}

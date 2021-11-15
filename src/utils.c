#include "../head/utils.h"


void tue_moi(char* msg, int fdc, ...)
{
	va_list fdv;
	va_start(fdv, fdc);

	perror(msg);
	for (int i=0; i<fdc; i++)
		close(va_arg(fdv, int));
	exit(1);
}

/******************************************************************************/
/********************************** ADRESSES **********************************/
/******************************************************************************/

void init_addr(struct sockaddr_in* addr, char* ip, char* port)
{
	//Preconditions
	//addr
	if (addr == NULL)
		tue_moi("init_addr: addr == NULL.", 0);
	//ip
        if(!ip){
                addr->sin_addr.s_addr = INADDR_ANY;
        }
	else if(inet_pton(AF_INET, ip, &addr->sin_addr) != 1){
                tue_moi("init_addr: inet_pton",0);
        }
        //port
        uint16_t port_reel = (uint16_t) strtol(port,NULL,0);
	if (2048 > port_reel || port_reel > 49151)
		tue_moi("init_addr: port n'est pas dans le bon intervalle.", 0);

        addr->sin_family = AF_INET;
        addr->sin_port = htons(port_reel);
}

int ipv4_valide(char* ip){
	if(!ip) return 0;

	char ipCpy[20];
	int nombrePoint = 0;
	int nombrePlage;
	char* token;

	if(!strncpy(ipCpy,ip,20)){
		fprintf(stderr,"strncpy a echoué.\n");
		exit(-1);
	}

	token = strtok(ipCpy,".");

	while(token){
		nombrePlage = atoi(token);

        char* cpyToken = token;
        while(*cpyToken != '\0'){
            if(*cpyToken >= '0' && *cpyToken <= '9'){
                cpyToken++;
            }else{
                return 0;
            }
        }

		if(nombrePlage<0 || nombrePlage >255){
			return 0;
		}
		token = strtok(NULL,".");
		if(token) nombrePoint++;
	}
	if(nombrePoint != 3) return 0;
	return 1;
}


/******************************************************************************/
/********************************** PAQUETS ***********************************/
/******************************************************************************/

paquet cree_paquet(unsigned char idFlux, unsigned char type,
                  uint16_t numSeq, uint16_t numAck,
                  unsigned char ecn, unsigned char tailleFenetre,
                  char* donnees)
{
        paquet paquet;
        paquet.idFlux = idFlux;
        paquet.type = type;
        paquet.numSeq = numSeq;
        paquet.numAck = numAck;
        paquet.ecn = ecn;
        paquet.tailleFenetre = tailleFenetre;
        if(donnees!=NULL){
                if (strlen(donnees) > 44)
                        fprintf(stderr, "Attention, les donnees ont ete tronquees.\n");

                strncpy(paquet.donnees,donnees,44); // attention si le 44ième octets n'est pas un '\0' alors le string
                                            // ne terminera pas par le char sentinel
        }else{
                paquet.donnees[0]='\0';
        }
        return paquet;
}

paquet *cree_paquet_gbn(unsigned char idFlux, unsigned char type,
                  uint16_t numSeq, uint16_t numAck,
                  unsigned char ecn, unsigned char tailleFenetre,
                  char* donnees)
{
        paquet * paquet = malloc(TAILLE_PAQUET);
        paquet->idFlux = idFlux;
        paquet->type = type;
        paquet->numSeq = numSeq;
        paquet->numAck = numAck;
        paquet->ecn = ecn;
        paquet->tailleFenetre = tailleFenetre;
        if(donnees!=NULL){
                if (strlen(donnees) > 44)
                fprintf(stderr, "Attention, les donnees ont ete tronquees.\n");

                strncpy(paquet->donnees,donnees,44); // attention si le 44ième octets n'est pas un '\0' alors le string
                                            // ne terminera pas par le char sentinel
        }else{
                paquet->donnees[0]='\0';
        }
        return paquet;
}

void affiche_paquet(paquet* paquet)
{
	printf("=======================================\n");
	printf("Flux: %d\n", paquet->idFlux);
	printf("Type: ");
	if (paquet->type & SYN)
		printf("SYN\t");
	if (paquet->type & FIN)
		printf("FIN\t");
	if (paquet->type & RST)
		printf("RST\t");
	if (paquet->type & ACK)
		printf("ACK");
	printf("\n");
	printf("Numero de sequence: %d\n", paquet->numSeq);
	printf("Numero d'acquittement: %d\n", paquet->numAck);
	printf("ECN:%d \n", paquet->ecn);
	printf("Taille fenetre: %d\n", paquet->tailleFenetre);
	printf("Donnees:\n%s\n", paquet->donnees);
	printf("=======================================\n");
}

int attend_paquet(int socket,struct sockaddr* adresse, paquet * buf){
        if(!buf){
                fprintf(stderr,"buf doit être non null\n");
                close(socket);
                exit(1);
        }
        if(!adresse){
                fprintf(stderr,"adresse doit être non null\n");
                close(socket);
                exit(1);
        }

        socklen_t tailleAdresse= sizeof(*adresse);
        int partiel;
        fd_set sockSet;
        FD_ZERO(&sockSet);
        FD_SET(socket,&sockSet);
        struct timeval timer = TIMEOUT;

        if(select(FD_SETSIZE,&sockSet,NULL,NULL,&timer) ==-1)
            tue_moi("select",1,socket);

	//Si il y a eu écriture
        if(FD_ISSET(socket,&sockSet)){
                if((partiel = recvfrom(socket,(void *)buf,
                                   TAILLE_PAQUET,0,
                                   (struct sockaddr*)adresse,
                                   &tailleAdresse))==-1){
                tue_moi("recvfrom",1,socket);
        }
        if(partiel!=TAILLE_PAQUET) return 0;
        return 1;
        }
        return 0;
}

int envoie_paquet(int socket,struct sockaddr* adresse, paquet* buf){
        if(!adresse){
                fprintf(stderr,"envoi_paquet: adresse NULL\n");
                close(socket);
                exit(1);
        }
        int tmp=0;
        if((tmp = sendto(socket,buf,TAILLE_PAQUET
                         ,0,adresse,TAILLE_ADRESSE))==-1){
                tue_moi("sendto",1,socket);

        }
        if(tmp!=52){
                return 0;
        }
        return 1;
}

/******************************************************************************/
/*********************************** FENETRE **********************************/
/******************************************************************************/

void modif_taille_fenetre(fenetre* fen, unsigned int debut, unsigned int fin)
{
        //Preconditions
        //Fenetre
        if (fen == NULL)
        {
                fprintf(stderr, "modif_taille_fenetre: fen NULL.\n");
                exit(1);
        }
        //Fin
        if (fin < debut)
        {
                fprintf(stderr, "modif_taille_fenetre: fin doit etre plus grand que debut.\n");
                exit(1);
        }

        fen->debut = debut;
        fen->fin = fin;
}

int taille_fenetre(fenetre* fen)
{
        if (fen == NULL)
                return 0;
        else
                return (fen->fin - fen->debut);
}

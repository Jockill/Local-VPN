#include <arpa/inet.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>


#define SYN (1<<0)
#define FIN (1<<1)
#define RST (1<<2)
#define ACK (1<<4)

#define STOP_N_WAIT 1
#define GO_BACK_N 2

#define TAILLE_PAQUET 52
#define TAILLE_DONNEES 44
#define TAILLE_FENETRE_SERVEUR 5


typedef struct paquet
{ //utiliser _uint ? ==> mieux représenter l'intention.
    unsigned char idFlux;
    unsigned char type;
    unsigned short numSeq;
    unsigned short numAck;
    unsigned char ecn;
    unsigned char tailleFenetre; //en nombre d'octets
    unsigned char donnees[TAILLE_DONNEES];
} paquet;

typedef struct fenetre
{
    unsigned int debut;
    unsigned int fin;
    //Taille de la fenetre d'envoi en nombre de paquets, identique a la taille
    //de la fenetre de reception
    unsigned char tailleEnvoi;
    //Taille de la fenetre de congestion en nombre de paquets
    unsigned char tailleCongestion;
} fenetre;


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
	in_addr_t ipNetwork;
	int tmp;
	if (ip != NULL) tmp = inet_pton(AF_INET, ip, &ipNetwork);
	else tmp = inet_pton(AF_INET, INADDR_ANY, &ipNetwork);
        if (tmp == 0) tue_moi("init_addr: ip ne correspond pas à une adresse IPv4 valide.", 0);
	
        //port
        uint16_t port_reel = (uint16_t) strtol(port,NULL,0);
	if (2048 > port_reel || port_reel > 49151)
		tue_moi("init_addr: port n'est pas dans le bon intervalle.", 0);

        addr->sin_family = AF_INET;
        addr->sin_addr.s_addr = ipNetwork;
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
        printf("%d\n",nombrePlage);
	}
	if(nombrePoint != 3) return 0;
	return 1;
}


/******************************************************************************/
/********************************** PAQUETS ***********************************/
/******************************************************************************/

paquet cree_paquet(unsigned char idFlux, unsigned char type,
                  unsigned short numSeq, unsigned short numAck,
                  unsigned char ecn, unsigned char tailleFenetre,
                  unsigned char* donnees)
{
        paquet paquet;
        paquet.idFlux = idFlux;
        paquet.type = type;
        paquet.numSeq = numSeq;
        paquet.numAck = numAck;
        paquet.ecn = ecn;
        paquet.tailleFenetre = tailleFenetre;
        if (strlen(donnees) > 44)
                fprintf(stderr, "Attention, les donnees ont ete tronquees.\n");
        strncpy(paquet.donnees,donnees,44); // attention si le 44ième octets n'est pas un '\0' alors le string
                                            // ne terminera pas par le char sentinel
        return paquet;
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
        //Debut
        if (debut < 0)
        {
                fprintf(stderr, "modif_taille_fenetre: debut doit etre positif.\n");
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

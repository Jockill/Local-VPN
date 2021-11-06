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

typedef struct paquet
{ //utiliser _uint ? ==> mieux représenter l'intention.
    unsigned char idFlux;
    unsigned char type;
    unsigned short numSeq;
    unsigned short numAck;
    unsigned char ecn;
    unsigned char tailleFenetre; //en nombre d'octets
    unsigned char donnees[44];
} paquet;

typedef struct fenetre
{
    unsigned int debut;
    unsigned int fin;
} fenetre;

struct timeval timer = {1,0};

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
	//Gestion des preconditions
	//addr
	if (addr == NULL)
		tue_moi("init_addr: addr == NULL.", 0);
	//ip
	in_addr_t ipNetwork;
	int tmp;
	if (ip != NULL)
		tmp = inet_pton(AF_INET, ip, &addr);
	else
		tmp = inet_pton(AF_INET, INADDR_ANY, &addr);
	if (tmp == 0)
		tue_moi("init_addr: ip ne correspond pas à une adresse IPv4 valide.", 0);
	if (tmp == -1)
		tue_moi("init_addr: mauvaise address family", 0);
	//port
	if (port < 2048 || port > 49151)
		tue_moi("init_addr: port n'est pas dans le bon intervalle.", 0);

	return;
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

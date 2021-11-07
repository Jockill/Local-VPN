#ifndef __UTILS_H__
#define __UTILS_H__

#include <arpa/inet.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

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
    char donnees[TAILLE_DONNEES];
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



/*!
  \brief Termine le programme avec un message d'erreur et ferme `fdc` \
  descripteurs passés en argument
  \param char* msg Le message à afficher
  \param int fdc Le nombre de descripteurs à fermer
  \param ... La liste des descripteurs à fermer
*/
void tue_moi(char* msg, int fdc, ...);

/******************************************************************************/
/********************************** ADRESSES **********************************/
/******************************************************************************/
/*!
  \pre addr != NULL
  \pre `ip` doit être une IPv4 valide ou NULL
  \pre 2048 < port < 49151
  \brief Initialise la structure `addr` par mutation à l'aide des arguments.
  \param struct sockaddr_in* addr L'adresse à initialiser
  \param char* ip L'IP à utiliser
  \param char* port Le port à utiliser
*/
void init_addr(struct sockaddr_in* addr, char* ip, char* port);

/*!
  \brief Vérifie si `ip` est une IPv4 valide
  \param char* ip L'IP à vérifier
  \return 1 en cas de réussite, 0 sinon
*/
int ipv4_valide(char* ip);


/******************************************************************************/
/********************************** PAQUETS ***********************************/
/******************************************************************************/
/*!
  \brief Crée, initialise et renvoie un paquet à l'aide des arguments.
  \param unsigned char idFlux ID du flux ayant généré le paquet.
  \param unsigned char type Type de paquet (SYN, FIN, RST, ACK supportés)
  \param unsigned short numSeq Numéro de séquence
  \param unsigned short numAck Numéro d'acquittement
  \param unsigned char ecn Bit de controle de congestion
  \param unsigned char tailleFenetre Taille de la fenetre de l'envoyeur au \
  moment de la création du paquet.
  \param char* donnees Données à utiliser.
  \note Si `donnees` est plus grand que TAILLE_DONNEES, le tableau sera tronqué
*/
paquet cree_paquet(unsigned char idFlux, unsigned char type,
                  unsigned short numSeq, unsigned short numAck,
                  unsigned char ecn, unsigned char tailleFenetre,
                  char* donnees);


/******************************************************************************/
/*********************************** FENETRE **********************************/
/******************************************************************************/
void modif_taille_fenetre(fenetre* fen, unsigned int debut, unsigned int fin);
int taille_fenetre(fenetre* fen);

#endif

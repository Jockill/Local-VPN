# SOURCE (*client*)

* **void** check_args_src(**int** argc, **char**** argv)
	* Vérifie la validité des arguments et quitte en affichant un message d'erreur sur `stderr` si ils sont invalides.


* **int** negociation_src(**struct sockaddr_in*** serveur, **int*** mode, **fenetre*** fen)
	* **PRE** `serveur` initialisé
	* mode == 1 : stop and wait
	* mode == 2 : go back n
	* Négocie l'ouverture d'une communication avec un serveur
	* Initialise la fenêtre de congestion à partir du serveur
	* **RETURN** _Undefinded_
	* **POST** `tailleFenetre == NULL si mode == 1, > 0 sinon`
	* **NOTE** Termine le programme en cas d'erreur


* **int** go_back_n(**struct sockaddr_in*** addrDst, **int** sockSrc, **fenetre*** fen)
	* **PRE** `addrDst != NULL`
	* **PRE** `sockSrc >= 0`
	* Crée et envoie des paquets avec le protocole *go back n* puis termine la communication.
	* **POST** `close(sockSrc)`


* **int** stop_and_wait(**struct sockaddr_in*** addrDst, **int** sockSrc)
	* **PRE** `addrDst != NULL`
	* **PRE** `sockSrc >= 0`
	* Crée et envoie des paquets avec le protocole *stop and wait* puis termine la communication.
	* **POST** `close(sockSrc)`


* **void** fin_communication_src(**struct sockaddr_in*** serveur)
	* **PRE** `serveur` initialisé
	* Signale une fin de communication à la destination et attend sa réponse.
	* **NOTE** Termine le programme en cas d'erreur
---
# DESTINATION (*serveur*)

* **void** check_args_dst(**int** argc, **char**** argv)
	* Vérifie la validité des arguments et quitte en affichant un message d'erreur sur `stderr` si ils sont invalides.


* **int** negociation_dst(**struct sockaddr_in*** client, **fenetre*** fenetre, **int*** mode)
	* **PRE** `client` initialisé
	* Négocie l'ouverture d'une communication avec un client.
	* Initialise la fenêtre de congestion du client avec les memes proprietes que `fenetre`
	* **RETURN** _Undefinded_
	* **NOTE** Si `*tailleFenetre == 0` alors le mode de communication sera en stop and wait.
	* **NOTE** Termine le programme en cas d'erreur


* **void** stop_and_wait_ecoute(**struct sockaddr_in*** client)
	* **PRE** `client` initialisé
	* Gère la réception des paquets et l'envoi d'acquittements en mode *stop and wait*
	* **NOTE** Termine le programme en cas d'erreur


* **void** go_back_n_ecoute(**struct sockaddr_in*** client, **int** tailleFenetre)
	* **PRE** `client` initialisé
	* Gère la réception des paquets et l'envoi d'acquittements en mode *go back n*
	* **NOTE** Termine le programme en cas d'erreur


* **void** fin_communication_dst(**struct sockaddr_in*** client)
	* **PRE** `client` initialisé
	* Signale une fin de communication à la source.
	* **NOTE** Termine le programme en cas d'erreur
---
# UTILS

## Variables

```c
#define SYN (1<<0)
#define FIN (1<<1)
#define RST (1<<2)
#define ACK (1<<4)

#define STOP_N_WAIT 1
#define GO_BACK_N 2

#define TAILLE_PAQUET 52

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
```

## Adresses

* **void** init_addr(**struct sockaddr_in*** addr, **char*** ip, **char*** port)
	* **PRE** `addr != NULL`
	* **PRE** ip doit être une IPv4 valide (*n.n.n.n*) ou NULL
	* **PRE** `2048 < port < 49151`
	* Initialise la structure `addr` par mutation avec les arguments.
	* **POST** `addr.sin_addr = INADDR_ANY si ip == NULL`

## Paquets



<<<<<<< HEAD
* **paquet** cree_paquet(**unsigned char** idFlux, **unsigned char** type, **unsigned short** numSeq, **unsigned short** numAck, **unsigned char** ecn, **unsigned char** tailleFenetre, **unsigned char*** donnees)
=======
* **paquet** cree_paquet(**char** idFlux, **char** type, **short** numSeq, **short** numAck, **char** ecn, **char** tailleFenetre, **char*** donnees)
>>>>>>> server
	* Crée, initialise un paquet et le renvoie.
	* **RETURN** Le paquet initialisé.

## Fenetre

* **void** modif_taille_fenetre(**fenetre*** fen, **unsigned int** debut, **unsigned int** fin)
	* **PRE** `debut > 0`
	* **PRE** `fin > debut`
	* Initialise la structure `fen` par mutation avec les arguments.
	* **NOTE** `debut` et `fin` sont donnés en nombre de paquets au lieu de nombre d'octets.


* **int** taille_fenetre(**fenetre*** fen)
	* **RETURN** La taille de `fen`

## Gestion du processus

* **void** tue_moi(**char*** msg, **int** fdc, ...)
	* Ferme `fdc` descrpiteurs, affiche `msg` sur `stderr` puis termine le programme.

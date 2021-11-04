# SOURCE (*client*)

* **void** check_args_src(**int** argc, **char**** argv)
	* Vérifie la validité des arguments et quitte en affichant un message d'erreur sur `stderr` si ils sont invalides.


* **int** negociation_src(**struct sockaddr_in*** serveur, **int** mode, **int*** tailleFenetre)
	* **PRE** `serveur` initialisé
	* mode == 1 : stop and wait
	* mode == 2 : go back n
	* Négocie l'ouverture d'une communication avec un serveur
	* Initialise la fenêtre de congestion de la source
	* **RETURN** -1 en cas d'erreur, valeur du descripteur créé sinon.
	* **POST** `tailleFenetre == NULL si mode == 1, > 0 sinon`


* **int** go_back_n(**struct sockaddr_in*** addrDst, **int** sockSrc, **int** tailleFenetre)
	* **PRE** `addrDst != NULL`
	* **PRE** `sockSrc >= 0`
	* Crée et envoie des paquets avec le protocole *go back n* puis termine la communication.
	* **POST** `close(sockSrc)`


* **int** stop_and_wait(**struct sockaddr_in*** addrDst, **int** sockSrc)
	* **PRE** `addrDst != NULL`
	* **PRE** `sockSrc >= 0`
	* Crée et envoie des paquets avec le protocole *stop and wait* puis termine la communication.
	* **POST** `close(sockSrc)`


* **int** fin_communication_src(**struct sockaddr_in*** serveur)
	* **PRE** `serveur` initialisé
	* Signale une fin de communication à la destination et attend sa réponse.
	* **RETURN** -1 en cas d'erreur, 0 sinon
---
# DESTINATION (*serveur*)

* **void** check_args_dst(**int** argc, **char**** argv)
	* Vérifie la validité des arguments et quitte en affichant un message d'erreur sur `stderr` si ils sont invalides.


* **int** negociation_dst(**struct sockaddr_in*** client, **int*** tailleFenetre)
	* **PRE** `client` initialisé
	* Négocie l'ouverture d'une communication avec un client.
	* Initialise la fenêtre de congestion du client à la même taille.
	* **RETURN** -1 en cas d'erreur, valeur du descripteur créé sinon.
	* **NOTE** Si `*tailleFenetre < 0` alors le mode de communication sera en stop and wait.


* **int** stop_and_wait_ecoute(**struct sockaddr_in*** client)
	* **PRE** `client` initialisé
	* Gère la réception des paquets et l'envoi d'acquittements en mode *stop and wait*
	* **RETURN** -1 en cas d'erreur, 0 sinon


* **int** go_back_n_ecoute(**struct sockaddr_in*** client, **int** tailleFenetre)
	* **PRE** `client` initialisé
	* Gère la réception des paquets et l'envoi d'acquittements en mode *go back n*
	* **RETURN** -1 en cas d'erreur, 0 sinon


* **int** fin_communication_dst(**struct sockaddr_in*** client)
	* **PRE** `client` initialisé
	* Signale une fin de communication à la source.
	* **RETURN** -1 en cas d'erreur, 0 sinon
---
# UTILS

## Variables

```c
#define SYN (1<<0)
#define FIN (1<<1)
#define RST (2<<2)
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
```

## Adresses

* **void** init_addr(**struct sockaddr_in*** addr, **char*** ip, **char*** port)
	* **PRE** `addr != NULL`
	* **PRE** ip doit être une IPv4 valide (*n.n.n.n*) ou NULL
	* **PRE** `2048 < port < 49151`
	* Initialise la structure `addr` par mutation avec les arguments.
	* **POST** `addr.sin_addr = INADDR_ANY si ip == NULL`

## Paquets



* **paquet** cree_paquet(**char** idFlux, **char** type, **short** numSeq, **short** numAck, **char** ecn, **char** fenetre, **char*** donnees)
	* Crée, initialise un paquet et le renvoie.
	* **RETURN** Le paquet initialisé.

## Fenetre

* **void** modif_taille_fenetre(**fenetre** fen, **unsigned int** debut, **unsigned int** fin)
	* **PRE** `debut > 0`
	* **PRE** `fin > debut`
	* Initialise la structure `fen` par mutation avec les arguments.
	* **NOTE** `debut` et `fin` sont donnés en nombre de paquets au lieu de nombre d'octets.

## Gestion du processus

* **void** tue_moi(**char*** msg, **int** fdc, ...)
	* Ferme `fdc` descrpiteurs, affiche `msg` sur `stderr` puis termine le programme.

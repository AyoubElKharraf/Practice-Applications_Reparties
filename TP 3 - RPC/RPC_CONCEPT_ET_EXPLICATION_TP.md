# GUIDE COMPLET : LE CONCEPT DE RPC (Remote Procedure Call) & ÉTUDE DU TP 3

Ce document est un guide pédagogique exhaustif et autonome conçu pour comprendre en profondeur :
1. Les concepts fondamentaux des architectures distribuées et du paradigme **RPC** (*Remote Procedure Call*).
2. Le fonctionnement interne de **Sun RPC / ONC RPC** (génération de stubs, sérialisation XDR, résolveur portmap/rpcbind).
3. L'analyse détaillée ligne par ligne de **chaque fichier du TP 3** (`calcule.x`, `calcule_server.c`, `client_calcule.c`, `Makefile`, etc.).
4. La cinématique complète d'un appel réseau (du client au serveur et retour).
5. Les pièges classiques en C/RPC (pointeurs statiques, libération mémoire, compatibilité moderne `libtirpc`).

---

## TABLE DES MATIÈRES
1. [Introduction : Pourquoi le RPC ?](#1-introduction--pourquoi-le-rpc-)
2. [Notions et Définitions Essentielles](#2-notions-et-définitions-essentielles)
3. [Architecture et Fonctionnement Général du RPC](#3-architecture-et-fonctionnement-général-du-rpc)
4. [La Boîte à Outils Sun RPC / ONC RPC](#4-la-boîte-à-outils-sun-rpc--onc-rpc)
5. [Analyse Détaillée des Fichiers du TP](#5-analyse-détaillée-des-fichiers-du-tp)
   - [5.1 L'interface IDL : calcule.x](#51-linterface-idl--calcule-x)
   - [5.2 Le Serveur Réparti : calcule_server.c](#52-le-serveur-réparti--calcule_serverc)
   - [5.3 Le Client : client_calcule.c](#53-le-client--client_calculec)
   - [5.4 Les Fichiers Générés par rpcgen](#54-les-fichiers-générés-par-rpcgen)
   - [5.5 L'automatisation : Makefile](#55-lautomatisation--makefile)
6. [Cycle de Vie d'un Appel RPC (Cinématique Pas à Pas)](#6-cycle-de-vie-dun-appel-rpc-cinématique-pas-à-pas)
7. [Guide Pratique d'Exécution et Commandes Utiles](#7-guide-pratique-dexécution-et-commandes-utiles)
8. [Résumé & Schéma Récapitulatif](#8-résumé--schéma-récapitulatif)

---

## 1. Introduction : Pourquoi le RPC ?

Dans la programmation classique en C, lorsque la fonction `main()` a besoin d'effectuer un calcul, elle appelle une procédure locale :

```c
int res = addition(5, 7);
```

Cet appel est **local** :
- Les arguments sont empilés dans la pile d'exécution (stack) de la mémoire RAM du processus.
- Le processeur exécute les instructions de `addition`.
- Le résultat est renvoyé directement dans un registre du CPU.

### Le Problème des Systèmes Répartis
Dans une application distribuée, le code qui calcule le résultat ne se trouve **pas** sur la même machine que le code demandeur :
- Soit pour déporter des calculs lourds sur un serveur puissant.
- Soit pour centraliser des données ou des règles métiers.
- Soit pour faire coopérer plusieurs micro-services hétérogènes.

Sans RPC, les développeurs devaient gérer manuellement :
1. La création de sockets (`socket()`, `connect()`, `bind()`, `listen()`, `accept()`).
2. L'encodage des variables en suite d'octets bruts (gestion de l'Endianness / Big-Endian vs Little-Endian).
3. L'envoi/réception sur le réseau (`send()`, `recv()`).
4. Le décodage et la gestion des pannes réseau.

### La Solution RPC (Inventée par Birrell et Nelson en 1984)
Le RPC a pour but d'offrir une **transparence de localisation** :
> Permettre à un programme d'exécuter une procédure sur une machine distante **avec la même syntaxe apparente qu'un appel de fonction local**, en masquant tous les détails de communication réseau sous-jacents.

---

## 2. Notions et Définitions Essentielles

| Notion | Définition & Rôle |
| :--- | :--- |
| **RPC** (*Remote Procedure Call*) | Mécanisme de communication inter-processus (IPC) permettant à un client d'invoquer une sous-routine sur un serveur distant de manière synchrone. |
| **IDL** (*Interface Definition Language*) | Langage formel neutre (ici la syntaxe `.x` de RPCL) décrivant les types de données, les signatures de fonctions, les numéros de programmes et de versions, indépendamment du langage cible. |
| **rpcgen** | Compilateur de protocole. Il lit le fichier `.x` et génère automatiquement le code C nécessaire : stubs client, stubs serveur, sérialiseurs et headers. |
| **Client Stub** (`calcule_clnt.c`) | "Façade" locale côté client. Elle ressemble à la vraie fonction, mais son rôle est d'empaqueter les arguments et de les transmettre au serveur via le réseau. |
| **Server Skeleton / Stub** (`calcule_svc.c`) | Point d'entrée côté serveur (`main`). Il écoute le réseau, reçoit la requête, déballe les données, appelle la vraie fonction métier (`_svc`), puis renvoie la réponse. |
| **Marshalling / Sérialisation** | Conversion de structures de données en mémoire vive (avec types, pointeurs, padding) en un flux d'octets linéaires prêt à transiter sur le câble réseau. |
| **Unmarshalling / Désérialisation**| Opération inverse : reconstruction des structures de données en mémoire à partir du flux d'octets reçu. |
| **XDR** (*eXternal Data Representation*) | Format standard de représentation canonique des données (RFC 1014/4506) garantissant que deux machines d'architectures différentes (ex: ARM 64-bit et x86 32-bit, Little-Endian et Big-Endian) puissent échanger sans corruption de types. |
| **Portmap / rpcbind** | Annuaire dynamique de services RPC (tourne généralement sur le port TCP/UDP 111). Quand un serveur démarre, il s'enregistre auprès de `rpcbind`. Le client interroge d'abord `rpcbind` pour savoir sur quel port éphémère écoute le serveur demandé. |
| **Transparence d'accès & de localisation** | Propriété du RPC faisant en sorte que le développeur ne se soucie ni de l'adresse IP, ni du port, ni des détails des sockets. |

---

## 3. Architecture et Fonctionnement Général du RPC

L'architecture RPC repose sur un découpage en 4 couches symétriques :

```
+------------------------------------+        +------------------------------------+
|           CLIENT APPLICATION       |        |          SERVER APPLICATION        |
|           (client_calcule.c)       |        |          (calcule_server.c)        |
+------------------------------------+        +------------------------------------+
                  |                                              ^
       Appel de fonction normal                       Appel direct de la fonction
       res = multxy_1(&param, clnt)                   multxy_1_svc(&param, rqstp)
                  v                                              |
+------------------------------------+        +------------------------------------+
|            CLIENT STUB             |        |          SERVER SKELETON           |
|          (calcule_clnt.c)          |        |          (calcule_svc.c)           |
+------------------------------------+        +------------------------------------+
                  |                                              ^
         Marshalling (XDR)                             Unmarshalling (XDR)
                  v                                              |
+------------------------------------+        +------------------------------------+
|          MODULE XDR & RPC          |        |          MODULE XDR & RPC          |
|      (calcule_xdr.c / libtirpc)    |        |      (calcule_xdr.c / libtirpc)    |
+------------------------------------+        +------------------------------------+
                  |                                              ^
             Réseau (Sockets UDP / TCP via rpcbind sur port 111) |
                  +----------------------------------------------+
```

### Le Rôle Clé de `rpcbind` (Port Mapper)
Contrairement aux serveurs HTTP classiques qui écoutent toujours sur le port fixe 80 ou 443 :
1. Le serveur RPC demande au système d'exploitation un **port aléatoire disponible**.
2. Il contacte le démon système local `rpcbind` (port fixe 111) et lui dit : *"Je suis le programme numéro `0x20000002`, version 1, et j'écoute sur le port UDP 45678"*.
3. Quand le client veut exécuter une fonction de `0x20000002`, il interroge d'abord `rpcbind` de la machine distante pour obtenir le numéro de port actuel, puis envoie sa requête directement sur ce port.

---

## 4. La Boîte à Outils Sun RPC / ONC RPC

Ce TP utilise la référence historique **Sun RPC** (Open Network Computing RPC), normalisée par l'IETF.

### Workflow de développement Sun RPC :
```
                  [ calcule.x ]  (Fichier IDL créé par le développeur)
                        |
                        |  commande: rpcgen calcule.x
                        v
     +------------------+------------------+------------------+
     |                  |                  |                  |
     v                  v                  v                  v
[ calcule.h ]   [ calcule_xdr.c ]  [ calcule_clnt.c ] [ calcule_svc.c ]
  (Headers)       (Filtres XDR)      (Stub Client)     (Skeleton Serveur)
     |                  |                  |                  |
     |                  |                  |                  |
     |  +---------------+                  |                  |
     |  |                                  |                  |
     v  v                                  v                  |
[ client_calcule.c ] ----------------> [ client_calcule ]     |
 (Écrit par dév)       Compilation           (Binaire)        |
                                                              v
[ calcule_server.c ] ---------------------------------> [  serveur  ]
 (Écrit par dév)                       Compilation           (Binaire)
```

---

## 5. Analyse Détaillée des Fichiers du TP

Examinons chaque composant présent dans votre projet.

### 5.1 L'interface IDL : `calcule.x`

Le fichier [calcule.x](file:///c:/Users/DELL/Documents/Master/Master_M2I/SEMESTRE%203/Module%201%20-%20Applications%20R%C3%A9parties/TPs/TP%203%20-%20RPC/calcule.x) est le contrat d'interface. Il définit les types échangés et les signatures des procédures distantes.

```c
/* 1. Déclaration d'une structure de données échangée */
struct operandes {
    float x;
    float y;
};

/* 2. Déclaration du programme RPC */
program MATHS_PROG {
    version MATHS_VERSION_1 {
        int PUISS2(int) = 1;         /* Procédure numéro 1 */
        float MULTXY(operandes) = 2; /* Procédure numéro 2 */
    } = 1;                           /* Version numéro 1 */
} = 0x20000002;                      /* Numéro unique du programme RPC */
```

#### Explications clés :
- **Numéro de programme (`0x20000002`)** :
  Les numéros RPC sont normalisés par tranches :
  - `0x00000000` à `0x1fffffff` : Définis par Sun Microsystems (ex: NFS, NIS).
  - `0x20000000` à `0x3fffffff` : Réservés aux développeurs utilisateurs (comme dans ce TP).
  - `0x40000000` à `0x5fffffff` : Temporaires / dynamiques.
- **Numéro de version (`1`)** : Permet de faire évoluer une API. Le même serveur pourrait supporter la version 1 et la version 2 en parallèle.
- **Règle Sun RPC sur les paramètres** :
  Une procédure RPC ne peut prendre **qu'un seul paramètre d'entrée** et renvoyer **qu'un seul résultat**. Si l'on souhaite passer plusieurs valeurs (comme deux flottants pour la multiplication), on doit obligatoirement les regrouper dans une `struct` (ici `struct operandes`).

---

### 5.2 Le Serveur Réparti : `calcule_server.c`

Ce fichier [calcule_server.c](file:///c:/Users/DELL/Documents/Master/Master_M2I/SEMESTRE%203/Module%201%20-%20Applications%20R%C3%A9parties/TPs/TP%203%20-%20RPC/calcule_server.c) contient la logique métier implémentée par le développeur.

```c
#include "calcule.h"

/* Implémentation de la procédure 1 : 2^n */
int *puiss2_1_svc(int *argp, struct svc_req *rqstp) {
    static int result;
    int i;
    int n = *argp;

    /* Cas 2^0 = 1 */
    if (n <= 0) {
        result = 1;
        return &result;
    }

    result = 2;
    for (i = 1; i < n; i++) {
        result = result * 2;
    }
    return &result;
}

/* Implémentation de la procédure 2 : x * y */
float *multxy_1_svc(operandes *argp, struct svc_req *rqstp) {
    static float result;
    result = argp->x * argp->y;
    return &result;
}
```

#### Points techniques cruciaux :
1. **Convention de nommage automatique** :
   La fonction `PUISS2` en version `1` est traduite par `rpcgen` en `puiss2_1_svc`.
   - `_1` : numéro de version.
   - `_svc` : suffixe désignant l'implémentation côté service (serveur).
2. **Pourquoi les arguments sont des pointeurs ?**
   En RPC, tous les paramètres sont passés par adresse (pointeurs), même pour un simple `int`.
3. **Pourquoi le mot-clé `static` sur le résultat ? (IMPORTANT)**
   ```c
   static int result;
   return &result;
   ```
   En C standard, une variable locale non-statique allouée sur la pile (*stack*) est détruite dès la sortie de la fonction. Renvoyer son adresse provoquerait un comportement indéfini (déréférencement d'un pointeur vers de la mémoire libérée).
   Le squelette RPC (`calcule_svc.c`) a besoin d'accéder à cette mémoire juste après pour la sérialiser et l'envoyer au client. Le mot-clé `static` alloue la variable dans le segment de données permanent du processus, garantissant que la valeur reste intacte.

---

### 5.3 Le Client : `client_calcule.c`

Le fichier [client_calcule.c](file:///c:/Users/DELL/Documents/Master/Master_M2I/SEMESTRE%203/Module%201%20-%20Applications%20R%C3%A9parties/TPs/TP%203%20-%20RPC/client_calcule.c) est l'application cliente consommant le service distant.

```c
#include <stdio.h>
#include <stdlib.h>
#include "calcule.h"

int main(int argc, char *argv[]) {
    CLIENT *client;
    operandes param;
    float *resultat;
    int n, *p;
    char *server_host = "localhost";

    if (argc >= 2) {
        server_host = argv[1];
    }

    /* 1. Création du handle client (Connexion logique au service) */
    client = clnt_create(server_host, MATHS_PROG, MATHS_VERSION_1, "udp");
    if (client == NULL) {
        clnt_pcreateerror(server_host);
        fprintf(stderr, "Erreur lors de la creation du client RPC vers %s\n", server_host);
        exit(EXIT_FAILURE);
    }

    /* 2. Appel transparent de multxy_1 */
    param.x = 12.0f;
    param.y = 20.0f;
    resultat = multxy_1(&param, client);
    if (resultat == (float *) NULL) {
        clnt_perror(client, "Echec de l'appel RPC multxy_1");
    } else {
        printf("Calcul distant : multxy(%.2f, %.2f) = %f\n", param.x, param.y, *resultat);
    }

    /* 3. Appel transparent de puiss2_1 */
    n = 16;
    p = puiss2_1(&n, client);
    if (p == (int *) NULL) {
        clnt_perror(client, "Echec de l'appel RPC puiss2_1");
    } else {
        printf("Calcul distant : puiss2(%d) = %d\n", n, *p);
    }

    /* 4. Fermeture et libération des ressources */
    clnt_destroy(client);
    return 0;
}
```

#### Éléments majeurs à comprendre :
- **`CLIENT *client`** : C'est le descripteur de contexte RPC (semblable à un handle de session ou descripteur de fichier).
- **`clnt_create(hôte, programme, version, protocole)`** :
  - Interroge le `rpcbind` de l'hôte désigné sur le protocole demandé (`"udp"` ou `"tcp"`).
  - Récupère le port du serveur et configure le socket.
- **`multxy_1(&param, client)`** :
  Pour le code client, c'est un appel de fonction tout à fait ordinaire en C ! Pourtant, le calcul réel se déroule sur le serveur.
- **Gestion des erreurs avec `clnt_perror`** :
  Si le serveur plante, si le réseau coupe ou si le timeout expire, la fonction renvoie `NULL`, et `clnt_perror` affiche la cause exacte de l'erreur réseau RPC.

---

### 5.4 Les Fichiers Générés par `rpcgen`

Lors de l'exécution de `rpcgen calcule.x`, 4 fichiers sont produits :

1. **`calcule.h`** :
   - Contient la définition des structures C (ex: `typedef struct operandes operandes;`).
   - Contient les constantes générées à partir du fichier `.x` :
     ```c
     #define MATHS_PROG 0x20000002
     #define MATHS_VERSION_1 1
     #define PUISS2 1
     #define MULTXY 2
     ```
   - Contient les prototypes des fonctions stubs client (`multxy_1`) et serveur (`multxy_1_svc`).

2. **`calcule_clnt.c` (Client Stub)** :
   Implémente la fonction `multxy_1(operandes *argp, CLIENT *clnt)`.
   Son rôle interne :
   - Fait appel à `clnt_call()` du système RPC.
   - Indique quel filtre XDR utiliser pour sérialiser l'entrée (`xdr_operandes`).
   - Indique quel filtre XDR utiliser pour désérialiser la réponse (`xdr_float`).
   - Attend la réponse du serveur (appel bloquant / synchrone) et renvoie le pointeur vers le résultat.

3. **`calcule_svc.c` (Server Skeleton)** :
   - Contient la fonction `main()` du serveur.
   - Crée le socket de transport (UDP et TCP).
   - S'enregistre auprès de `rpcbind` (`svc_register(...)`).
   - Contient la boucle d'écoute événementielle (`svc_run()`).
   - À chaque message reçu, il lit l'en-tête RPC, identifie le numéro de procédure demandée (via un `switch(rqstp->rq_proc)`), fait l'unmarshalling des paramètres, appelle la fonction métier correspondante (`puiss2_1_svc` ou `multxy_1_svc`), sérialise le résultat et le renvoie sur la socket.

4. **`calcule_xdr.c` (Filtres de Sérialisation)** :
   - Contient les fonctions de conversion XDR (ex: `xdr_operandes(XDR *xdrs, operandes *objp)`).
   - Ces filtres savent comment traduire un `float`, un `int` ou une `structure` en format standardisé 32-bit big-endian réseau.

---

### 5.5 L'automatisation : `Makefile`

Le [Makefile](file:///c:/Users/DELL/Documents/Master/Master_M2I/SEMESTRE%203/Module%201%20-%20Applications%20R%C3%A9parties/TPs/TP%203%20-%20RPC/Makefile) orchestre l'ensemble du processus de compilation :

```makefile
CC = gcc
CFLAGS = -Wall -Wextra -g -I/usr/include/tirpc
LDLIBS = -ltirpc

RPC_SRC = calcule.x
CLIENT = client_calcule
SERVER = serveur

all: $(SERVER) $(CLIENT)

# Étape 1 : Si calcule.x change, régénérer les 4 fichiers sources
calcule.h calcule_xdr.c calcule_clnt.c calcule_svc.c: $(RPC_SRC)
	rpcgen $(RPC_SRC)

# Étape 2 : Compilation des stubs
calcule_xdr.o: calcule_xdr.c calcule.h
calcule_clnt.o: calcule_clnt.c calcule.h
calcule_svc.o: calcule_svc.c calcule.h

# Étape 3 : Édition des liens du Client
$(CLIENT): client_calcule.o calcule_clnt.o calcule_xdr.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

# Étape 4 : Édition des liens du Serveur
$(SERVER): calcule_svc.o calcule_server.o calcule_xdr.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)
```

> **À noter : Pourquoi `-I/usr/include/tirpc` et `-ltirpc` ?**
> Historiquement, le support RPC était intégré dans la bibliothèque standard C (`glibc`). Dans les distributions Linux récentes (Ubuntu 20+, Debian 11+, Fedora, etc.), le code RPC a été retiré de la glibc et transféré vers une bibliothèque dédiée et plus moderne appelée **TI-RPC** (*Transport-Independent RPC*), d'où la nécessité de lier avec `-ltirpc`.

---

## 6. Cycle de Vie d'un Appel RPC (Cinématique Pas à Pas)

Voici chronologiquement tout ce qui se produit lors de l'appel `resultat = multxy_1(&param, client);` :

```
Étape 1 [Client]  : Le programme client appelle multxy_1(&param, client) dans le stub client.
Étape 2 [Client]  : Le stub client convertit (marshalling XDR) les deux floats dans un buffer binaire.
Étape 3 [Client]  : Le stub injecte l'entête RPC (N° Programme=0x20000002, Version=1, Procédure=2, ID Transaction).
Étape 4 [Réseau]  : Un datagramme UDP est envoyé à l'adresse IP et au port du serveur.
Étape 5 [Serveur] : Le skeleton (calcule_svc.c) intercepte le paquet UDP dans sa boucle svc_run().
Étape 6 [Serveur] : Le squelette désérialise (unmarshalling) les deux floats à l'aide de xdr_operandes.
Étape 7 [Serveur] : Le squelette appelle la fonction métier multxy_1_svc(argp, rqstp).
Étape 8 [Serveur] : La fonction métier calcule 12.0 * 20.0 = 240.0 et stocke le résultat dans 'static float result'.
Étape 9 [Serveur] : Le squelette sérialise le résultat float via xdr_float et renvoie le paquet réseau au client.
Étape 10 [Client] : Le stub client reçoit la réponse, déballe le float et renvoie son pointeur au main().
Étape 11 [Client] : Le client affiche "Calcul distant : multxy(12.00, 20.00) = 240.000000".
```

---

## 7. Guide Pratique d'Exécution et Commandes Utiles

Pour exécuter et tester ce TP (sous Linux ou WSL) :

### 1. Démarrer le portmapper
```bash
sudo service rpcbind start
```

### 2. Compiler le projet
```bash
make
```

### 3. Démarrer le serveur
```bash
./serveur &
```

### 4. Vérifier que le service est bien enregistré
```bash
rpcinfo -p
```
*Sortie attendue : une ligne affichant le programme `536870914` (`0x20000002`), la version `1`, le protocole `udp`/`tcp` et le port alloué.*

### 5. Exécuter le client
```bash
./client_calcule localhost
```
*Sortie affichée dans la console :*
```text
Calcul distant : multxy(12.00, 20.00) = 240.000000
Calcul distant : puiss2(16) = 65536
```

---

## 8. Résumé & Schéma Récapitulatif

| Élément | Qui l'écrit ? | Quel est son rôle ? |
| :--- | :--- | :--- |
| `calcule.x` | Le Développeur | Contrat d'interface (signatures des fonctions et constantes IDL). |
| `calcule_server.c` | Le Développeur | Logique mathématique (`puiss2_1_svc`, `multxy_1_svc`). |
| `client_calcule.c` | Le Développeur | Application cliente qui consomme les services distants. |
| `calcule.h` | `rpcgen` | Définitions de types en C et prototypes de fonctions. |
| `calcule_clnt.c` | `rpcgen` | Stub client qui réalise le packaging et l'envoi réseau. |
| `calcule_svc.c` | `rpcgen` | Serveur RPC d'écoute réseau et répartiteur d'appels. |
| `calcule_xdr.c` | `rpcgen` | Fonctions de conversion universelle de données binaires. |
| `rpcbind` | Système Linux | Annuaire dynamique de mise en relation des ports. |

Grâce au mécanisme RPC, la complexité des sockets, de l'endianness et des protocoles réseau est entièrement automatisée par `rpcgen` et `XDR`, offrant au développeur l'illusion parfaite d'exécuter des fonctions locales alors qu'elles sont distribuées.

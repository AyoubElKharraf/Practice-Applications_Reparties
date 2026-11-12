# Guide détaillé des commandes

Ce document explique comment compiler et lancer le mini-chat UDP de ce projet. Les commandes supposent un terminal Linux ou macOS et doivent être exécutées depuis le dossier qui contient le `Makefile`.

## 1. Prérequis

- Un compilateur C, ici `gcc`.
- `make`, qui lit les règles du `Makefile`.
- Des terminaux séparés pour le serveur et les clients.

Sous Windows, utilisez un environnement Linux tel que WSL si `gcc`, `make` et les fonctions POSIX (`fork`, `unistd.h`) ne sont pas disponibles dans votre terminal.

## 2. Compiler le projet

```sh
make
```

La cible par défaut `all` compile les deux programmes :

- `serveur_chat.c` devient l’exécutable `serveur_chat` ;
- `client_chat.c` devient l’exécutable `client_chat`.

Le Makefile utilise `gcc -Wall -Wextra`. Ces options demandent au compilateur d’afficher les avertissements courants. La compilation ne démarre pas les programmes.

Pour ne compiler qu’un programme :

```sh
make serveur_chat
make client_chat
```

Les règles équivalentes, sans Makefile, sont :

```sh
gcc -Wall -Wextra -o serveur_chat serveur_chat.c
gcc -Wall -Wextra -o client_chat client_chat.c
```

## 3. Démarrer le serveur

Dans un terminal :

```sh
./serveur_chat 5000
```

`5000` est le port UDP choisi pour cet exemple. Le port doit être compris entre 1 et 65535 et ne doit pas déjà être utilisé. Le serveur s’attache à ce port et attend les datagrammes. Laissez ce terminal ouvert.

Le serveur affiche les erreurs système éventuelles, comme un port déjà occupé, et les informations sur les clients et les messages.

## 4. Démarrer les clients

Ouvrez un autre terminal pour chaque participant et exécutez :

```sh
./client_chat localhost 5000
```

Remplacez `localhost` par le nom ou l’adresse IP de la machine qui héberge le serveur. Le port doit être le même que celui fourni au serveur. Pour plusieurs clients sur la même machine, ouvrez plusieurs terminaux et lancez la commande dans chacun.

Chaque client est enregistré par le serveur dès son premier message non vide. L’énoncé décrit une diffusion à l’ensemble des clients en ligne. Dans le code actuel, le serveur envoie cependant chaque message aux autres clients enregistrés et exclut l’expéditeur. Le client ne l’affiche pas non plus localement comme message envoyé.

## 5. Quitter un client

Dans le terminal du client, tapez :

```text
/quit
```

Le client envoie cette commande au serveur, qui le retire de la liste des clients en ligne. Les raccourcis `Ctrl+D` (fin d’entrée) et `Ctrl+C` (interruption) ferment également le client ; celui-ci envoie alors `/quit` avant de se terminer.

## 6. Limite de clients et messages

- Le serveur accepte au plus 10 clients. Un client supplémentaire reçoit le message `Serveur plein : 10 clients en ligne maximum`.
- La longueur maximale d’un message est de 225 octets côté protocole. Le client tronque les messages trop longs avant l’envoi et essaie de ne pas couper un caractère UTF-8 en plein milieu.
- Les lignes vides ne sont pas envoyées.
- UDP ne garantit ni la livraison, ni l’ordre, ni l’absence de doublons des datagrammes.

## 7. Nettoyer les exécutables

```sh
make clean
```

La cible `clean` supprime les exécutables `serveur_chat` et `client_chat`. Les fichiers sources et les documents restent présents. Pour reconstruire après le nettoyage, relancez `make`.

## Résumé d’une session locale

Terminal 1 :

```sh
make
./serveur_chat 5000
```

Terminal 2 et, si besoin, terminal 3 :

```sh
./client_chat localhost 5000
```

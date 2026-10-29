# TP 1 — Sockets TCP en C

Ce dépôt regroupe des exemples progressifs de communication client-serveur en IPv4/TCP : échange simple, écho, accès distant et mini-protocole de transfert de fichiers.

## Prérequis et compilation

Les sources utilisent les interfaces POSIX (`fork`, `getifaddrs`, sockets BSD). Compilez-les sous Linux, macOS ou WSL avec GCC et `make` ; elles ne sont pas directement compilables avec le compilateur Windows natif.

Depuis ce dossier :

```sh
make             # compile ex1, ex2 et ex2_versions
make ex1         # seulement les clients/serveurs de l'exercice 1
make ex2         # seulement l'exercice 2
make versions    # seulement les variantes de l'exercice 2
make clean       # supprime les exécutables générés
```

Le Makefile utilise `gcc -Wall -Wextra`. Les exécutables sont placés à côté de leurs sources. Lancez toujours le serveur dans un terminal et le client dans un autre. Sur la même machine, utilisez `127.0.0.1` ou `localhost`. Pour deux machines, utilisez l'adresse IP du serveur et autorisez le port TCP dans le pare-feu.

## Dossiers

- [`ex1/README.md`](ex1/README.md) — sockets de base, écho et clients distants ; port habituel 6000.
- [`ex2/README.md`](ex2/README.md) — client/serveur FTP pédagogique, navigation et transferts ; port fourni par l'utilisateur.
- [`ex2_versions/README.md`](ex2_versions/README.md) — versions V1 (messages/commandes) et V2 (fichiers), port par défaut 5000.

Pour le tutoriel complet de connexion sur deux machines (réseau, compilation, pare-feu, lancement et dépannage), consultez [`GUIDE_COMMUNICATION_2_MACHINES.md`](GUIDE_COMMUNICATION_2_MACHINES.md).

## Principes réseau illustrés

Le serveur crée une socket (`socket`), l'associe à une adresse et un port (`bind`), attend (`listen`) puis accepte une connexion (`accept`). Le client résout l'hôte et se connecte (`connect`). Les deux côtés échangent des octets (`send`/`recv`) puis ferment leur socket (`close`). `AF_INET` désigne IPv4, `SOCK_STREAM` TCP, `htons` convertit un port en ordre réseau. TCP est un flux : un `recv` ne correspond pas forcément à un `send` entier. Les exercices de transfert ajoutent donc des lignes de protocole et des fonctions de lecture/envoi complets.

## Dépannage rapide

- `Connection refused` : vérifiez que le serveur est lancé et que client et serveur utilisent le même port.
- Délai d'attente depuis une autre machine : vérifiez l'adresse IP, le réseau et le pare-feu.
- `Address already in use` : le port est déjà utilisé ; arrêtez l'autre serveur ou choisissez un port libre.
- `make` introuvable : installez les outils de compilation C et GNU Make dans votre environnement Linux/WSL.

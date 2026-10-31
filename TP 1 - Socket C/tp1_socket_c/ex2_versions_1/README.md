# Exercice 2 — Versions pédagogiques V1 et V2

Ce dossier contient deux implémentations indépendantes de client/serveur TCP. Elles compilent à part de `../ex2` et utilisent des noms d'exécutables différents. Les sources s'appuient sur des fonctions POSIX (`fork`, `getifaddrs`) : compilez sous Linux, macOS ou WSL.

## Compilation

Depuis ce dossier :

```sh
make                 # compile serveur_v1, client_v1, serveur_v2, client_v2
make clean           # retire ces exécutables
```

Depuis la racine du projet, `make versions` appelle ce Makefile.

## Version 1 — messages et commandes

Le serveur écoute par défaut sur le port 5000 ; il accepte plusieurs clients à l'aide de `fork()`. Le client propose une invite interactive et envoie des commandes textuelles.

```sh
./serveur_v1 [port]
./client_v1 <hote> [port]
```

Exemple local, dans deux terminaux :

```sh
./serveur_v1 5000
./client_v1 127.0.0.1 5000
```

À l'invite client, essayez `ping`, `date`, `quit` ou un texte libre. Le code source serveur répond au texte ordinaire par un écho. `serveur_v1.c` traite explicitement `ping`, `date` et `quit` ; consultez `client_v1.c` pour confirmer les commandes disponibles dans cette version exacte.

## Version 2 — navigation et transfert de fichiers

Le serveur prend un port facultatif (5000 par défaut) et un dossier racine facultatif (`./racine_ftp` par défaut). Il crée le dossier racine si besoin, puis y confine les sessions clientes. Le client exige l'hôte et accepte un port facultatif.

Terminal serveur :

```sh
mkdir -p partage
./serveur_v2 5001 "$PWD/partage"
```

Terminal client :

```sh
./client_v2 127.0.0.1 5001
```

Commandes de l'invite `v2>` :

```text
ls                 lister le répertoire distant courant
pwd                afficher le chemin distant
cd <repertoire>    changer de répertoire distant
get <fichier>      télécharger dans le répertoire courant du client
put <fichier>      envoyer un fichier local vers le serveur
quit               fermer la session
```

Pour vérifier un transfert, créez un fichier dans `partage`, téléchargez-le avec `get`, ou envoyez un fichier existant avec `put`, puis comparez les copies avec `sha256sum` sur Linux/macOS. Depuis une autre machine, remplacez `127.0.0.1` par l'IP du serveur et ouvrez le port TCP choisi dans son pare-feu.

## Rôle des sources et fonctions

- `serveur_v1.c` : `main()` prépare l'écoute TCP et crée un enfant par connexion ; `afficher_ip()` indique les interfaces ; `traiter_client()` reçoit les messages et répond à `ping`, `date`, `quit` ou par écho.
- `client_v1.c` : se connecte au serveur et fournit le dialogue interactif pour envoyer les commandes/messages.
- `serveur_v2.c` : crée la racine de partage, accepte les connexions et gère `GET_REQUEST`, `PUT_REQUEST`, `LS_REQUEST`, `CD_REQUEST` et `PWD_REQUEST`. `in_root()` et `handle_cd()` contrôlent le confinement ; `handle_get()` et `handle_put()` transfèrent les fichiers ; `handle_ls()` et la requête PWD gèrent la navigation.
- `client_v2.c` : `cmd_get()` télécharge, `cmd_put()` téléverse, `cmd_ls()` liste, `cmd_texte()` envoie les requêtes textuelles ; `main()` connecte et interprète la ligne de commande utilisateur.
- `ftp_common.h` : `send_all()`/`recv_all()` assurent les transferts complets, `recv_line()` lit les en-têtes terminés par une nouvelle ligne, et les fonctions `stream_*_file()` copient une taille exacte par blocs de 4096 octets.

Le protocole V2 annonce nom et taille avant le contenu binaire. TCP ne préserve pas les frontières d'appel : les boucles `send_all`, `recv_all` et le transfert selon la taille évitent de traiter un fragment comme un fichier entier.

> Les fichiers `commandes.txt` et `étapes.txt` sont des notes historiques ; certaines commandes/exécutables qu'ils citent ne correspondent pas aux sources présentes. Suivez les noms et exemples de ce README.

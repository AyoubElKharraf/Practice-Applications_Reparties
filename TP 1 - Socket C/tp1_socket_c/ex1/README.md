# Exercice 1 — Connexions TCP, écho et accès distant

Les programmes écoutent par défaut sur le port TCP **6000**. Compilez depuis la racine du projet avec `make ex1`, ou dans ce dossier avec les commandes `gcc` ci-dessous. Les sources sont POSIX : utilisez Linux, macOS ou WSL.

```sh
cd ..
make ex1
```

## Programmes

| Client | Serveur | Fonctionnement |
|---|---|---|
| `client_simple` | `serveur_simple` | Envoie le message fixe « Bonjour serveur » et affiche la réponse. |
| `client_echo` | `serveur_echo` | Échange interactif ; le serveur renvoie la phrase en majuscules. `quit` termine. |
| `client_distant` | `serveur_distant` | Envoie un message unique à un hôte/port donné ; le serveur accuse réception et affiche l'adresse du client. |
| `client_echo_distant` | `serveur_echo_distant` | Écho interactif avec adresse et port configurables côté client. |

`serveur_echo_multi` est une compilation de `serveur_echo.c` avec `-DMULTI` : le serveur crée un processus fils par client pour traiter plusieurs connexions.

## Démarrer les exemples en local

Dans un premier terminal, lancez un serveur ; dans un second, son client correspondant :

```sh
./serveur_simple
./client_simple                 # dans l'autre terminal

./serveur_echo
./client_echo                   # tapez des phrases, puis quit

./serveur_distant               # port par défaut 6000
./client_distant 127.0.0.1 6000 "Bonjour"

./serveur_echo_distant          # port par défaut 6000
./client_echo_distant 127.0.0.1 6000
```

Un seul serveur peut utiliser un port à la fois : arrêtez le serveur précédent avant d'en lancer un autre. Pour arrêter un serveur en attente, faites `Ctrl+C`.

## Compilation manuelle

```sh
gcc -Wall -Wextra -o serveur_simple serveur_simple.c
gcc -Wall -Wextra -o client_simple client_simple.c
gcc -Wall -Wextra -o serveur_echo serveur_echo.c
gcc -Wall -Wextra -DMULTI -o serveur_echo_multi serveur_echo.c
gcc -Wall -Wextra -o client_echo client_echo.c
gcc -Wall -Wextra -o serveur_distant serveur_distant.c
gcc -Wall -Wextra -o client_distant client_distant.c
gcc -Wall -Wextra -o serveur_echo_distant serveur_echo_distant.c
gcc -Wall -Wextra -o client_echo_distant client_echo_distant.c
```

## Rôle des fonctions et appels importants

- Dans les serveurs, `socket()` crée le point de communication ; `bind()` réserve le port ; `listen()` prépare l'attente ; `accept()` obtient une socket dédiée à un client.
- Dans les clients, `gethostbyname()` résout le nom de machine, puis `connect()` ouvre la connexion TCP.
- `send()` transmet les messages ; `recv()` les lit. Les clients interactifs lisent le clavier avec `fgets()` et détectent `quit`.
- `serveur_echo.c` convertit le texte reçu en majuscules ; la variante `MULTI` utilise `fork()` pour isoler le traitement de chaque client.
- `serveur_distant.c` utilise `getifaddrs()` pour afficher les adresses IPv4 de la machine. `htons()` prépare le port en ordre réseau et `close()` libère les sockets.

Les exemples simples illustrent le cycle socket, mais n'implémentent pas tous un protocole robuste pour les envois partiels ou les messages fragmentés : c'est l'un des points étudiés dans l'exercice 2.

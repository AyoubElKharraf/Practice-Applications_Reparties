# Exercice 2 — Mini-protocole de transfert de fichiers

`client_ftp.c` et `serveur_ftp.c` implémentent un protocole applicatif au-dessus de TCP : commandes texte terminées par une nouvelle ligne, réponses avec état et longueur, et contenu binaire transféré par blocs. Le serveur crée un processus fils par client. La racine de partage doit être un dossier dédié.

## Compilation

Depuis la racine `tp1_socket_c` :

```sh
make ex2
```

Ou depuis `ex2` :

```sh
gcc -Wall -Wextra -o serveur_ftp serveur_ftp.c
gcc -Wall -Wextra -o client_ftp client_ftp.c
```

## Démarrage local

Dans le terminal serveur, créez un dossier de partage et démarrez le serveur (le programme prend exactement un port et un chemin racine) :

```sh
mkdir -p partage
./serveur_ftp 5001 "$PWD/partage"
```

Dans un autre terminal, depuis `ex2`, connectez-vous et utilisez les commandes interactives :

```sh
./client_ftp 127.0.0.1 5001
```

À l'invite `ftp>` :

```text
ls [repertoire]       lister le répertoire distant courant ou indiqué
pwd                   afficher le chemin distant courant (relatif à la racine)
cd <repertoire>       changer de répertoire distant
get <fichier>         télécharger dans le répertoire courant du client
put <fichier-local>   téléverser depuis le répertoire courant du client
quit                  fermer la session
```

Les commandes s'enchaînent sur la même connexion. `get` écrit le fichier reçu dans le dossier où le client a été lancé. `put` prend un fichier local régulier et l'enregistre dans le dossier distant courant. Le serveur refuse les chemins qui sortent de la racine configurée. Choisissez un port libre si 5001 est déjà occupé.

## Une commande par lancement

Le client accepte aussi une commande protocolaire unique après l'hôte et le port :

```sh
./client_ftp 127.0.0.1 5001 PWD_REQUEST
./client_ftp 127.0.0.1 5001 LS_REQUEST
./client_ftp 127.0.0.1 5001 CD_REQUEST sous-dossier
./client_ftp 127.0.0.1 5001 GET_REQUEST rapport.txt
./client_ftp 127.0.0.1 5001 PUT_REQUEST document.txt
```

Pour `PUT_REQUEST`, le fichier indiqué doit exister dans le répertoire courant du client. Pour `GET_REQUEST`, le fichier doit être accessible depuis le répertoire distant courant.

## Fichiers et fonctions

- `serveur_ftp.c` : `main()` valide le port et la racine, crée la socket d'écoute, puis fait `bind()`, `listen()` et `accept()`. `session()` lit les requêtes et les distribue à `handle_get`, `handle_put`, `handle_cd`, `handle_ls` ou `handle_pwd`.
- `handle_get()` vérifie le fichier, envoie une réponse avec sa taille puis son contenu binaire. `handle_put()` lit le nom et la taille annoncée, reçoit les octets et écrit le fichier.
- `handle_cd()` et `handle_ls()` vérifient que le chemin canonique demeure dans la racine serveur. `pwd` expose un chemin relatif à cette racine.
- `client_ftp.c` : `connecter()` résout l'hôte et établit TCP. `cmd_text()` gère `ls`, `pwd`, `cd`; `cmd_get()` télécharge; `cmd_put()` envoie un fichier local. `read_reply()` et `print_payload()` décodent les réponses et affichent les erreurs.
- `ftp_common.h` : `send_all()` et `recv_all()` bouclent jusqu'à transmettre/recevoir le nombre d'octets demandé ; `recv_line()` lit une ligne ; `stream_to_file()` et `stream_from_file()` transfèrent des fichiers en blocs de 4096 octets.

Le serveur est multi-processus (`fork`). Les réponses de texte incluent leur longueur, tandis que les en-têtes de requête sont séparés du flux binaire par `\n`. Cette distinction permet de retrouver les frontières des données malgré le flux TCP.

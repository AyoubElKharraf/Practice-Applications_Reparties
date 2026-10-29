# Guide de A à Z — communiquer entre deux machines

Ce guide explique comment faire communiquer deux machines à distance pour échanger du texte et transférer des fichiers (texte, image, archive, PDF, vidéo ou autre contenu binaire). Il utilise les programmes de `ex1` pour les messages et ceux de `ex2` pour les fichiers.

## Ce qu'il faut

- Deux ordinateurs connectés au même réseau local (même Wi-Fi ou réseau Ethernet), ou deux machines dont les réseaux peuvent se joindre.
- Un environnement Linux, macOS ou WSL avec GCC et GNU Make sur chaque machine. Les sources utilisent des sockets POSIX et `fork()` : elles ne se compilent pas directement avec le compilateur Windows natif.
- Le dossier du projet sur les deux machines, ou au minimum les sources `ex1/serveur_echo_distant.c` sur le serveur et `ex1/client_echo_distant.c` sur le client.
- Le port TCP `6000` libre sur le serveur et autorisé par son pare-feu.
- Pour les transferts de fichiers, le port TCP `5001` (ou un autre port libre) autorisé aussi.

> Si vous utilisez WSL, le réseau NAT par défaut peut empêcher une autre machine du LAN de joindre directement le serveur. Pour le premier essai, utilisez deux machines Linux/macOS, une VM configurée en réseau ponté, ou configurez l'accès entrant WSL depuis Windows.

## 1. Vérifier le réseau

Sur les deux machines, confirmez qu'elles sont connectées au même réseau. Sur la machine qui hébergera le serveur, trouvez son adresse IPv4 locale :

```sh
hostname -I
# autre méthode Linux :
ip -4 addr
```

Choisissez l'adresse associée au Wi-Fi/Ethernet, par exemple `192.168.1.50`. N'utilisez pas `127.0.0.1` depuis l'autre machine : cette adresse désigne toujours l'ordinateur local.

Depuis le client, testez l'accès IP :

```sh
ping 192.168.1.50
```

Remplacez l'exemple par l'adresse réelle. Certaines machines bloquent les requêtes ping ; dans ce cas, l'absence de réponse au ping ne prouve pas à elle seule que TCP ne fonctionnera pas.

## 2. Compiler

Sur les deux machines, ouvrez un terminal à la racine `tp1_socket_c` du projet :

```sh
make ex1
```

La compilation crée `ex1/serveur_echo_distant` et `ex1/client_echo_distant`, parmi les autres exécutables de l'exercice 1. Si `make` ou `gcc` manque, installez les outils de compilation (sur Debian/Ubuntu : `sudo apt update` puis `sudo apt install build-essential`).

Compilation directe si vous ne voulez construire que le serveur/client :

```sh
gcc -Wall -Wextra -o ex1/serveur_echo_distant ex1/serveur_echo_distant.c
gcc -Wall -Wextra -o ex1/client_echo_distant ex1/client_echo_distant.c
```

## 3. Autoriser le port sur le serveur

Sur Ubuntu avec UFW :

```sh
sudo ufw allow 6000/tcp
sudo ufw status
```

Sur une autre distribution, ajoutez une règle entrante TCP pour le port `6000` dans son pare-feu. Sur un réseau d'établissement ou d'entreprise, les règles peuvent être administrées par le réseau. N'ouvrez le port que sur le réseau nécessaire.

## 4. Démarrer le serveur

Sur la machine serveur, depuis la racine du projet :

```sh
./ex1/serveur_echo_distant 6000
```

Gardez ce terminal ouvert. Le programme affiche les adresses IPv4 détectées et attend les connexions. Notez l'adresse LAN du serveur, par exemple `192.168.1.50`.

## 5. Connecter le client

Sur l'autre machine, depuis la racine de son dossier projet :

```sh
./ex1/client_echo_distant 192.168.1.50 6000
```

Remplacez `192.168.1.50` par l'adresse du serveur. Le client affiche « Connecté au serveur ! ». Saisissez ensuite quelques phrases :

```text
> Bonjour serveur
Serveur distant : BONJOUR SERVEUR
> Comment ça va ?
Serveur distant : COMMENT ÇA VA ?
> quit
```

Le serveur affiche les messages reçus et l'adresse du client. `quit` ferme la session cliente ; il ne coupe pas le serveur, qui reste disponible pour d'autres connexions. Pour arrêter le serveur, utilisez `Ctrl+C` dans son terminal.

## 6. Comprendre le trajet d'un message

1. Le serveur appelle `socket()` pour créer une socket TCP IPv4, puis `bind()` sur le port 6000 et toutes ses interfaces (`INADDR_ANY`).
2. `listen()` met la socket en attente et `accept()` crée une connexion dédiée pour chaque client.
3. Le client résout l'adresse, crée sa socket puis appelle `connect()` vers l'IP du serveur et le port 6000.
4. Le client envoie le texte avec `send()`. Le serveur le lit avec `recv()`, le met en majuscules puis le renvoie avec `send()`.
5. Le client affiche la réponse reçue avec `recv()` et ferme sa connexion avec `close()`. Le serveur utilise `fork()` pour traiter chaque client dans un processus fils.

## 7. Problèmes fréquents

| Symptôme | Vérifications |
|---|---|
| `connect: Connection refused` | Le serveur est-il lancé ? L'IP et le port sont-ils corrects ? Le serveur écoute-t-il bien sur le port 6000 ? |
| La connexion expire | Les machines sont-elles sur un réseau qui permet leur communication ? Le pare-feu serveur autorise-t-il TCP/6000 ? Le Wi-Fi utilise-t-il l'isolation des clients ? |
| `bind: Address already in use` | Un autre programme utilise déjà ce port. Arrêtez-le ou choisissez un autre port, par exemple 6001, sur le serveur **et** le client. |
| L'IP affichée ne fonctionne pas | Relevez à nouveau l'adresse avec `hostname -I` ; elle peut changer après reconnexion au Wi-Fi. Utilisez l'adresse de l'interface réseau active, pas `127.0.0.1`. |
| `ping` échoue | Le ping peut être filtré. Essayez quand même le client ; contrôlez surtout le routage et le port TCP. |
| `./...: No such file or directory` | Compilez avec `make ex1` depuis la racine du projet et vérifiez le chemin/nom de l'exécutable. |

Pour changer de port, utilisez le même port des deux côtés, par exemple `6001` : serveur `./ex1/serveur_echo_distant 6001`, règle pare-feu TCP/6001, puis client `./ex1/client_echo_distant <IP_SERVEUR> 6001`.

## 8. Échanger du texte à distance

Cette partie correspond à `ex1/serveur_echo_distant.c` et `ex1/client_echo_distant.c`. Le serveur renvoie chaque texte en majuscules. Les étapes 1 à 4 précédentes restent nécessaires ; sur le serveur, ouvrez TCP/6000 puis lancez :

```sh
./ex1/serveur_echo_distant 6000
```

Sur le client distant, en remplaçant l'adresse par l'IP LAN réelle du serveur :

```sh
./ex1/client_echo_distant 192.168.1.50 6000
```

Tapez des phrases puis `quit`. Le client et le serveur doivent utiliser le même numéro de port. Le port par défaut est `6000` si le second argument n'est pas donné.

Pour envoyer une seule demande avec réponse d'accusé de réception plutôt qu'une session d'écho, utilisez les programmes `client_distant` et `serveur_distant` : démarrez `./ex1/serveur_distant 6000` sur la machine serveur puis lancez côté client `./ex1/client_distant 192.168.1.50 6000 "Bonjour depuis le client"`. Ce couple illustre un message court et une réponse unique.

## 9. Transférer des fichiers de tout type à distance

L'exercice 2 transfère les fichiers comme des octets, en mode binaire. Les données ne sont donc pas limitées au texte. Le serveur partage un dossier racine ; le client peut y lister les fichiers, naviguer, envoyer un fichier (`put`) et en recevoir un (`get`). Les noms/tailles sont échangés dans le protocole, puis le contenu est transféré par blocs.

### Préparer et lancer le serveur de fichiers

Sur la machine serveur, depuis la racine `tp1_socket_c` :

```sh
make ex2
mkdir -p partage/multimedia
printf 'Bonjour depuis le serveur\n' > partage/message.txt
./ex2/serveur_ftp 5001 "$PWD/partage"
```

Laissez ce terminal ouvert. Ouvrez le port TCP/5001 sur le pare-feu serveur, comme indiqué plus haut (remplacez 6000 par 5001 dans la commande `ufw`). Le dossier `partage` est la racine visible par le client. Les opérations `put` et `get` agissent dans le dossier distant courant ; le client ne peut pas sortir de la racine serveur.

### Se connecter et échanger du texte

Sur la machine cliente, compilez avec `make ex2`, puis connectez-vous :

```sh
./ex2/client_ftp 192.168.1.50 5001
```

À l'invite `ftp>` :

```text
pwd                   afficher le répertoire distant courant
ls                    voir message.txt et multimedia/
get message.txt       télécharger le texte depuis le serveur
```

Après `get`, `message.txt` est enregistré dans le répertoire local depuis lequel vous avez lancé le client.

### Téléverser un fichier texte, une image ou un fichier binaire

Dans un autre terminal client, créez des fichiers d'exemple :

```sh
printf 'Fichier envoyé au serveur\n' > note.txt
head -c 1048576 /dev/urandom > donnees.bin
```

Le fichier `.bin` contient un mégaoctet d'octets aléatoires ; remplacez-le par un fichier réel (image, archive, PDF ou vidéo) si nécessaire. Dans la session `ftp>` :

```text
put note.txt
put donnees.bin
ls
cd multimedia
pwd
cd ..
quit
```

Le serveur sauvegarde ces fichiers dans le dossier partagé distant. `put` prend le chemin local indiqué, mais transmet le nom de base du fichier. Pour retrouver la sortie, `ls` affiche la liste du répertoire serveur courant.

### Télécharger un fichier binaire et vérifier son intégrité

Dans la session FTP, téléchargez le fichier envoyé précédemment :

```text
get donnees.bin
quit
```

Comparez l'empreinte du fichier sur le serveur et sur le client. Depuis la racine du projet, côté serveur :

```sh
sha256sum partage/donnees.bin
```

Côté client, dans le dossier où `get` a écrit le fichier :

```sh
sha256sum donnees.bin
```

Les deux empreintes SHA-256 doivent être identiques. Pour macOS, utilisez `shasum -a 256` à la place de `sha256sum`.

### Commandes du client de fichiers

| Commande `ftp>` | Effet |
|---|---|
| `ls [dossier]` | Liste le dossier distant courant ou le dossier donné. |
| `pwd` | Affiche le chemin distant courant. |
| `cd <dossier>` | Change de dossier distant, sans sortir de la racine partagée. |
| `get <fichier>` | Télécharge un fichier distant vers le dossier local courant. |
| `put <fichier-local>` | Envoie un fichier local vers le dossier distant courant. |
| `quit` ou `exit` | Ferme la session. |

Le client `ex2/client_ftp` exige les deux arguments hôte et port. Le serveur `ex2/serveur_ftp` exige lui aussi le port et le chemin de racine : `./ex2/serveur_ftp <port> <dossier_partage>`.

## 10. Utiliser les versions pédagogiques V1/V2 (optionnel)

Le sous-dossier `ex2_versions` propose une autre interface. Compilez sur les deux machines avec `make versions` depuis la racine du projet, ou avec `make` depuis `ex2_versions`.

- Messages/commandes : serveur `./ex2_versions/serveur_v1 5000`, puis client distant `./ex2_versions/client_v1 192.168.1.50 5000`. Cette V1 répond à `ping`, `date`, `quit` et renvoie un écho pour les autres textes.
- Transfert de fichiers : serveur `./ex2_versions/serveur_v2 5001 "$PWD/partage_v2"`, puis client `./ex2_versions/client_v2 192.168.1.50 5001`. Les commandes disponibles sont `ls`, `pwd`, `cd`, `get`, `put`, `quit`. Ouvrez TCP/5001 sur le pare-feu.

Choisissez un seul serveur pour chaque port. Si la V1 et le FTP `ex2` sont utilisés en même temps, ils peuvent écouter sur des ports différents, par exemple V1 sur 5000 et FTP sur 5001.

## Communication entre réseaux différents

Ce guide vise d'abord deux machines joignables sur le même réseau local. Pour joindre un serveur derrière un routeur depuis Internet, il faut une configuration réseau supplémentaire (adressage public, redirection de port, règles de pare-feu) et une protection adaptée. Ne publiez pas ce serveur de TP directement sur Internet.

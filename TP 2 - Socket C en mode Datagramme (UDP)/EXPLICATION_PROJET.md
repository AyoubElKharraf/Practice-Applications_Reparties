# Comprendre le mini-chat en C avec UDP

## 1. But du projet

Le projet met en place un chat simple en réseau. Un serveur reçoit les messages des clients et les redistribue aux autres clients connus. La communication utilise UDP, un protocole sans connexion : les données sont envoyées sous forme de datagrammes indépendants, sans établir une session fiable comme avec TCP.

Le titre du sujet mentionne le mode datagramme (UDP) ; le mot TCP dans le nom du dossier ne décrit pas le protocole utilisé par ces deux programmes. Le code appelle `SOCK_DGRAM`, ce qui correspond à UDP.

## 2. Rôle des fichiers

- `serveur_chat.c` : crée une socket UDP, attend les messages, maintient une liste de dix adresses de clients au maximum et redistribue les messages.
- `client_chat.c` : résout l’adresse du serveur, crée sa socket et permet d’émettre et recevoir en même temps.
- `Makefile` : fournit les règles de compilation et de nettoyage.

## 3. Déroulement général

1. Le serveur crée une socket IPv4 UDP avec `socket(AF_INET, SOCK_DGRAM, 0)` puis réserve le port demandé avec `bind`.
2. Un client résout le nom du serveur, crée sa socket et choisit un port local libre avec `bind` sur le port zéro.
3. Le client crée un processus fils avec `fork`. Le parent lit le clavier et envoie les datagrammes ; le fils attend les datagrammes et affiche les messages reçus. Les deux partagent la socket héritée.
4. À la réception d’un message, le serveur récupère l’adresse IP et le port de l’expéditeur avec `recvfrom`. Il ajoute cette adresse à sa table si elle n’y figure pas.
5. Le serveur construit un texte préfixé par `[ip:port]>`, puis le transmet avec `sendto`. L’énoncé demande de le diffuser à l’ensemble des clients en ligne. Le code actuel saute l’indice de l’expéditeur (`if (i == idx) continue`) : seuls les autres clients le reçoivent. Le client ne réaffiche pas non plus localement le texte qu’il vient d’envoyer.
6. Si le message est `/quit`, le serveur retire l’adresse correspondante de la table.

## 4. Fonctions importantes du serveur

### `addr_to_str`

Convertit l’adresse IPv4 et le port d’une structure `sockaddr_in` en texte (`adresse:port`). `inet_ntop` convertit l’adresse IP ; `ntohs` remet le port en ordre d’octets compréhensible par la machine.

### `find_client`

Parcourt la table `clients` et compare l’adresse IP et le port. Elle retourne l’indice du client si trouvé, sinon `-1`. Cette paire identifie le point de terminaison UDP du client.

### `remove_client`

Retire un élément en décalant les entrées suivantes vers la gauche, puis décrémente `nb_clients`.

### `print_clients`

Affiche les adresses actuellement enregistrées, en utilisant `addr_to_str`.

### `main` du serveur

Vérifie le nombre d’arguments et la plage du port, crée et attache la socket, puis boucle sur `recvfrom`. Il traite `/quit`, refuse les nouveaux clients si la table est pleine, ajoute les autres nouveaux clients, formate le message et le diffuse avec `sendto`.

## 5. Fonctions importantes du client

### `main` du client

Vérifie les arguments, convertit le port avec `atoi`, recherche l’hôte avec `gethostbyname`, puis prépare les structures d’adresse IPv4. Il crée la socket UDP et fait `bind` sur le port zéro : le système attribue un port local libre. Ce `bind` se fait avant `fork`, afin que les deux processus partagent le même point de réception.

### `fork`

Duplique le processus. Le fils (`pid == 0`) s’occupe de la réception ; le parent s’occupe de la saisie et de l’envoi. Cette séparation évite qu’une attente bloquante de réception empêche la saisie d’un message, ou inversement.

### `recvfrom` et `sendto`

`recvfrom` attend et lit un datagramme dans le fils du client et dans la boucle du serveur. `sendto` envoie un datagramme à une adresse cible sans connexion préalable : le client l’utilise pour contacter le serveur, et le serveur pour répondre aux clients.

### `fgets`, `strcspn` et la gestion de `/quit`

Le parent lit une ligne avec `fgets`, enlève les fins de ligne avec `strcspn`, ignore les lignes vides et limite la taille à `MSG_MAX`. Si la ligne dépasse le tampon de saisie, le reste est consommé afin qu’il ne devienne pas le prochain message. `/quit` déclenche l’envoi de la commande puis l’arrêt du fils avec `kill`, l’attente de sa terminaison avec `waitpid` et la fermeture de la socket avec `close`.

### Signal `SIGINT`

`sigaction` installe un gestionnaire pour `Ctrl+C`. Celui-ci positionne le drapeau `interrompu`; la boucle d’entrée peut alors se terminer proprement et prévenir le serveur. `sig_atomic_t` est employé pour le drapeau modifié par le gestionnaire de signal.

## 6. Structures et constantes

- `struct sockaddr_in` décrit une adresse IPv4 et son port.
- `AF_INET` indique IPv4 ; `SOCK_DGRAM` indique des datagrammes UDP.
- `htons` convertit un port vers l’ordre réseau ; `htonl(INADDR_ANY)` permet au serveur d’écouter sur toutes ses interfaces réseau.
- `MAX_CLIENTS` vaut 10, `MSG_MAX` vaut 225 et `BUF_SIZE` réserve de la place au préfixe d’adresse en plus du message.
- `memset` initialise les structures à zéro ; `memcpy` copie l’adresse IP résolue ; `snprintf` construit les chaînes sans dépasser le tampon fourni.

## 7. Limites et points à comprendre

- UDP n’offre pas de garantie de livraison, de remise dans l’ordre ou de retransmission. Le programme ne met pas en place ces mécanismes.
- L’énoncé demande une diffusion à tous les clients en ligne, mais l’implémentation actuelle ne retransmet pas le message à son expéditeur. Pour suivre littéralement l’énoncé, il faudrait retirer le test qui saute `idx` dans la boucle de diffusion du serveur.
- La liste du serveur est temporaire : elle est perdue quand le serveur s’arrête. Un client devient connu lorsqu’il envoie son premier message ; il n’y a pas de procédure de connexion séparée.
- Les clients qui ferment brutalement leur machine sans envoyer `/quit` peuvent rester dans la table jusqu’à l’arrêt du serveur.
- La taille des messages est limitée. Le client tronque son entrée ; le serveur ne traite que jusqu’à `MSG_MAX` octets reçus.
- Le client utilise des appels POSIX (`fork`, `sigaction`, `unistd.h`) et le Makefile cible `gcc`. Il est donc prévu pour un environnement de type Unix, ou WSL sous Windows.

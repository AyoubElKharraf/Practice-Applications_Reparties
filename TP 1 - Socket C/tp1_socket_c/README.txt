TP 1 - SOCKETS EN C
===================

Ce dossier contient plusieurs petits programmes en C pour apprendre la communication réseau avec les sockets TCP. Les noms « client » et « serveur » désignent les deux côtés d'une connexion : le serveur attend les connexions, et le client les ouvre.

STRUCTURE DU DOSSIER
--------------------

.vscode/
    Paramètres de Visual Studio Code pour l'éditeur IntelliSense : chemins
    d'inclusion, compilateur et standard C. Ce dossier ne contient pas les
    programmes du TP.

ex1/ - Connexions TCP simples, écho et accès distant
    client_simple.c / serveur_simple.c
        Exemple de base : le client envoie un message fixe et le serveur
        répond. À lancer dans deux terminaux sur la même machine.

    client_echo.c / serveur_echo.c
        Le client lit des lignes au clavier. Le serveur renvoie chaque ligne
        en majuscules. Le mot « quit » termine la session.

    serveur_echo.c compilé avec MULTI
        Variante du serveur d'écho qui traite les connexions dans des
        processus fils pour servir plusieurs clients. Le Makefile crée
        l'exécutable serveur_echo_multi avec cette option.

    client_distant.c / serveur_distant.c
        Exemple de message envoyé à un serveur accessible par nom ou adresse
        IP. Le serveur affiche les adresses réseau de la machine et accuse
        réception du message.

    client_echo_distant.c / serveur_echo_distant.c
        Variante interactive de l'écho utilisable avec une machine distante.
        Le client précise l'adresse du serveur et peut éventuellement son port.

ex2/ - Mini-protocole de transfert de fichiers
    client_ftp.c / serveur_ftp.c
        Client et serveur qui échangent des commandes et des données de
        fichiers via TCP. Le serveur prend en charge les opérations prévues
        par le protocole du TP, notamment lister, naviguer et transférer des
        fichiers. Il limite les chemins au dossier racine de partage.

    ftp_common.h
        Fonctions partagées pour envoyer ou recevoir tous les octets demandés
        et lire des lignes de protocole. TCP est un flux d'octets : un appel
        send() ou recv() ne correspond pas nécessairement à un message complet.

    video.bin
        Fichier binaire fourni comme exemple de donnée à transférer.

ex2_versions/ - Deux versions pédagogiques distinctes de l'exercice 2
    serveur_v1.c / client_v1.c
        Version 1 : échange de messages et commandes simples entre client et
        serveur distant.

    serveur_v2.c / client_v2.c
        Version 2 : commandes de navigation et transfert de fichiers en blocs,
        avec un espace de fichiers partagé limité à une racine.

    ftp_common.h
        Primitives communes à la version 2 pour assurer les transferts complets
        et lire les lignes du protocole.

    commandes.txt / étapes.txt
        Aide et étapes d'utilisation préparées pour ces versions.

Makefile (à la racine)
    Automatise la compilation avec gcc. « make » compile les exercices 1 et 2
    ainsi que le sous-dossier ex2_versions. « make ex1 », « make ex2 » et
    « make versions » compilent seulement la partie correspondante.
    « make clean » supprime les exécutables générés.

COMMENT L'ENSEMBLE S'ARTICULE
-----------------------------

Le dossier ex1 sert à comprendre progressivement socket(), bind(), listen(),
accept(), connect(), send() et recv() avec de petits échanges de texte.

Le dossier ex2 réutilise TCP pour un protocole plus riche. Les commandes et
les données sont échangées selon un format défini par le projet; les fichiers
sont envoyés par morceaux, et leur taille est communiquée afin de savoir quand
le transfert est terminé.

Le dossier ex2_versions conserve une autre présentation de l'exercice 2,
avec une version de démonstration de commandes et une version orientée transfert
de fichiers. Ce sont des sources séparées de ex2, avec leur propre Makefile.

NOTIONS IMPORTANTES POUR L'EXPLICATION ORALE
--------------------------------------------

- Une socket est un point de communication entre deux programmes.
- AF_INET indique l'utilisation d'adresses IPv4; SOCK_STREAM indique TCP.
- Le serveur prépare une adresse avec bind(), se met en attente avec listen(),
  puis accepte une connexion avec accept().
- Le client ouvre sa connexion avec connect().
- htons() et ntohs() convertissent les numéros de port vers ou depuis l'ordre
  des octets du réseau.
- TCP garantit un flux fiable et ordonné, mais ne conserve pas les limites des
  messages. Il faut donc définir comment reconnaître une commande complète
  (par exemple une ligne terminée par un saut de ligne) et combien d'octets
  constituent un fichier transféré.
- Pour tester une communication entre deux machines, le serveur doit écouter
  sur une interface accessible, le client doit utiliser l'adresse IP du
  serveur, et le port doit être autorisé par le pare-feu.

Les commentaires explicatifs des fonctions sont présents dans les fichiers
C et les en-têtes pour aider à suivre le rôle de chaque étape.
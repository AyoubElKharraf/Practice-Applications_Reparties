# TP 3 - Remote Procedure Call (ONC RPC / Sun RPC)

Ce dossier contient l'implémentation complète de l'exemple RPC avec les fonctions `puiss2` et `multxy`.

---

## 1. Structure des fichiers

- [calcule.x](file:///c:/Users/DELL/Documents/Master/Master_M2I/SEMESTRE%203/Module%201%20-%20Applications%20R%C3%A9parties/TPs/TP%203%20-%20RPC/calcule.x) : Spécification IDL de l'interface RPC (structures, identifiant du programme `0x20000002`, version 1, numéros de procédures).
- [calcule_server.c](file:///c:/Users/DELL/Documents/Master/Master_M2I/SEMESTRE%203/Module%201%20-%20Applications%20R%C3%A9parties/TPs/TP%203%20-%20RPC/calcule_server.c) : Implémentation côté serveur des deux fonctions (`puiss2_1_svc` et `multxy_1_svc`).
- [client_calcule.c](file:///c:/Users/DELL/Documents/Master/Master_M2I/SEMESTRE%203/Module%201%20-%20Applications%20R%C3%A9parties/TPs/TP%203%20-%20RPC/client_calcule.c) : Programme client appelant les fonctions distantes via les stubs RPC.
- [Makefile](file:///c:/Users/DELL/Documents/Master/Master_M2I/SEMESTRE%203/Module%201%20-%20Applications%20R%C3%A9parties/TPs/TP%203%20-%20RPC/Makefile) : Automatisation de la génération avec `rpcgen` et de la compilation.

---

## 2. Prérequis pour l'exécution (Environnement Linux / WSL / VM)

> **Note importante** : Le protocole Sun/ONC RPC repose sur le service d'enregistrement **`rpcbind`** (ou `portmap`) et la bibliothèque système RPC (incluse dans glibc ou via `libtirpc`). Sur Windows, il s'exécute typiquement dans un environnement Linux (WSL, machine virtuelle Ubuntu/Debian, ou conteneur).

### Paquets requis (Debian / Ubuntu / Kali / WSL) :
```bash
sudo apt update
sudo apt install build-essential rpcbind libtirpc-dev
```

### Activer le service `rpcbind` :
Avant de lancer le serveur RPC, `rpcbind` doit être actif pour que le serveur puisse enregistrer son programme et son port :
```bash
sudo service rpcbind start
# ou
sudo systemctl start rpcbind
```

---

## 3. Compilation et Génération

Dans le terminal Linux (dans le dossier `TP 3 - RPC`) :

### Méthode 1 : Utilisation du Makefile
```bash
make
```

### Méthode 2 : Étape par étape à la main

1. **Générer les stubs et les en-têtes avec `rpcgen`** :
   ```bash
   rpcgen calcule.x
   ```
   Cela crée :
   - `calcule.h`
   - `calcule_xdr.c`
   - `calcule_clnt.c`
   - `calcule_svc.c`

2. **Compiler le serveur** :
   ```bash
   gcc -Wall -I/usr/include/tirpc -c calcule_xdr.c
   gcc -Wall -I/usr/include/tirpc -c calcule_svc.c
   gcc -Wall -I/usr/include/tirpc -c calcule_server.c
   gcc -o serveur calcule_svc.o calcule_server.o calcule_xdr.o -ltirpc
   ```

3. **Compiler le client** :
   ```bash
   gcc -Wall -I/usr/include/tirpc -c client_calcule.c
   gcc -Wall -I/usr/include/tirpc -c calcule_clnt.c
   gcc -o client_calcule client_calcule.o calcule_clnt.o calcule_xdr.o -ltirpc
   ```

---

## 4. Exécution et Test

1. **Lancer le serveur** :
   ```bash
   ./serveur &
   ```

2. **Vérifier l'enregistrement auprès du service RPC** :
   ```bash
   rpcinfo -p
   ```
   Vous devriez voir apparaître le programme `536870914` (`0x20000002`), version 1, sur UDP et TCP.

3. **Lancer le client** :
   ```bash
   # En local :
   ./client_calcule localhost

   # Ou vers une machine distante :
   ./client_calcule <IP_OU_NOM_SERVEUR>
   ```

### Résultat attendu :
```text
Calcul distant : multxy(12.00, 20.00) = 240.000000
Calcul distant : puiss2(16) = 65536
```

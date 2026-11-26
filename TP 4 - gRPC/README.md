# Rapport de TP : Système Distribué avec gRPC et Protocol Buffers en Java

---

## 1. Introduction et Objectif du TP

Ce TP a pour objectif de concevoir et d'implémenter un système d'appels de procédures distantes moderne à l'aide de **gRPC** et **Protocol Buffers (Protobuf)** en **Java**, calqué sur les besoins du cours d'Applications Réparties (implémentation des opérations `puiss2` et `multxy`).

En plus des appels unaires classiques (comme en Sun/ONC RPC et Java RMI), nous exploitons la puissance de **HTTP/2** pour implémenter des mécanismes de **Streaming** (Server Streaming et Bidirectional Streaming).

---

## 2. Comparatif : ONC RPC vs Java RMI vs gRPC

| Fonctionnalité | ONC / Sun RPC (C) | Java RMI (Java) | gRPC (Google) |
| :--- | :--- | :--- | :--- |
| **Contrat d'interface (IDL)** | Fichier `.x` (ex: `calcule.x`) | Interface Java étendant `java.rmi.Remote` | Fichier `.proto` (*Protocol Buffers v3*) |
| **Compilateur de stubs** | `rpcgen` | `rmic` (historique) / proxies dynamiques | `protoc` + plugin `protoc-gen-grpc-java` |
| **Protocole de Transport** | RPC sur TCP ou UDP (ports dynamiques via `portmap`/`rpcbind`) | Sockets TCP brutes (JRMP) | **HTTP/2** (multiplexé sur un seul port TCP standard) |
| **Format de Sérialisation** | **XDR** (*eXternal Data Representation*) | Sérialisation native Java (`java.io.Serializable`) | **Protobuf** (binaire compressé, typé, performant et neutre vis-à-vis du langage) |
| **Interopérabilité** | C / C++ principalement | Java uniquement | Polyglotte (Java, Python, C++, Go, C#, Rust, Node.js...) |
| **Types d'appels** | Requête / Réponse unitaire bloquante | Requête / Réponse unitaire bloquante | **Unaire**, **Server Streaming**, **Client Streaming**, **Streaming Bidirectionnel** |

---

## 3. Architecture du Projet

```text
TP 4 - MiniProjet RPC/
├── pom.xml                                    # Configuration Maven des dépendances et plugins
├── generate-proto.bat                         # Script Windows pour compiler manuellement le .proto via protoc
├── run-server.bat                             # Script pour lancer le Serveur gRPC en un clic
├── run-client.bat                             # Script pour lancer le Client gRPC en un clic
├── README.md                                  # Ce document explicatif complet
└── src/
    └── main/
        ├── proto/
        │   └── calcule.proto                  # Définition de l'IDL gRPC (services et messages)
        └── java/
            └── ma/ump/grpc/
                ├── CalculServiceImpl.java     # Implémentation du service gRPC (Unaire + Streaming)
                ├── CalculServer.java          # Serveur Netty exposant le service sur le port 63676
                └── CalculClient.java          # Client démontrant tous les modes d'appel
```

---

## 4. Description détaillée des Fichiers

### 4.1. L'Interface IDL : `src/main/proto/calcule.proto`

Le contrat définit à la fois les structures de données (`message`) et les opérations distantes (`service`) :

```protobuf
syntax = "proto3";

option java_multiple_files = true;
option java_package = "ma.ump.grpc";
option java_outer_classname = "CalculeProto";

package calcule;

service CalculService {
  // 1. Unaire : Calcule 2^n
  rpc Puiss2 (Puiss2Request) returns (Puiss2Reply);

  // 2. Unaire : Calcule x * y
  rpc MultXY (MultXYRequest) returns (MultXYReply);

  // 3. Server Streaming : Calcule et diffuse 2^0 jusqu'à 2^n en flux continu
  rpc GenererPuissances (Puiss2Request) returns (stream Puiss2Reply);

  // 4. Streaming Bidirectionnel : Flux continu de couples (x, y) et réponses au fur et à mesure
  rpc MultStream (stream MultXYRequest) returns (stream MultXYReply);
}

message Puiss2Request {
  int32 n = 1;
}

message Puiss2Reply {
  int32 exposant = 1;
  int64 resultat = 2;
}

message MultXYRequest {
  float x = 1;
  float y = 2;
}

message MultXYReply {
  float resultat = 1;
}
```

* **`syntax = "proto3"`** : Version moderne de Protobuf.
* **`java_multiple_files = true`** : Génère un fichier `.java` distinct pour chaque classe de message et stub.
* **`stream`** : Mot-clé indiquant un flux asynchrone HTTP/2.

---

### 4.2. Implémentation du Service : `CalculServiceImpl.java`

Cette classe hérite de la classe abstraite `CalculServiceGrpc.CalculServiceImplBase` générée par gRPC.

* **Gestion du modèle Unaire** :
  * On reçoit la requête directement (`request`).
  * On envoie la réponse via `responseObserver.onNext(reply)`.
  * On termine l'appel avec `responseObserver.onCompleted()`.
  * En cas d'erreur de validation (ex. $n < 0$ ou $n \ge 63$), on renvoie `responseObserver.onError(Status.INVALID_ARGUMENT...)`.

* **Gestion du Server Streaming (`genererPuissances`)** :
  * Le serveur fait une boucle pour calculer $2^0, 2^1, \dots, 2^n$.
  * À chaque itération, il appelle `responseObserver.onNext(...)`. Les messages sont poussés immédiatement sur la connexion HTTP/2.
  * Il clôture avec `responseObserver.onCompleted()`.

* **Gestion du Streaming Bidirectionnel (`multStream`)** :
  * Le serveur retourne un `StreamObserver<MultXYRequest>` qui gère les événements du client :
    * `onNext(req)` : invoqué à chaque fois que le client envoie une paire $(x, y)$. Le serveur calcule le résultat et appelle immédiatement `responseObserver.onNext(...)`.
    * `onCompleted()` : invoqué quand le client indique qu'il a fini d'envoyer ses données.

---

### 4.3. Le Serveur : `CalculServer.java`

* Utilise `io.grpc.ServerBuilder` adossé à **Netty Shaded** pour l'implémentation du protocole HTTP/2.
* Écoute sur le port **63676** (sélectionné pour éviter les collisions avec d'autres services locaux).
* Intègre un `ShutdownHook` pour libérer proprement les sockets lors de l'arrêt du serveur.

---

### 4.4. Le Client : `CalculClient.java`

* Crée un `ManagedChannel` pointant sur `localhost:63676` avec `usePlaintext()` (chiffrement TLS désactivé pour simplifier l'environnement de TP local).
* Utilise deux types de stubs générés :
  1. **`CalculServiceBlockingStub`** : Synchrone / bloquant, idéal pour les appels unaires et le Server Streaming (itérable simplement via `Iterator<T>`).
  2. **`CalculServiceStub`** : Asynchrone non bloquant, nécessaire pour le Streaming Bidirectionnel basé sur des observateurs d'événements (`StreamObserver`).
* Emploie un `CountDownLatch` pour synchroniser le thread principal et attendre la fin du flux bidirectionnel avant de fermer le canal.

---

## 5. Démonstration et Traces d'Exécution Réelles

### Console Serveur (`CalculServer`)
```text
[Serveur] gRPC démarré sur le port 63676
[Serveur] puiss2(n=16) -> 65536
[Serveur] multXY(x=12.5, y=4.0) -> 50.0
[Serveur - Server Streaming] Génération puissances de 0 à 6
[Serveur - BiDi Stream] Reçu (2.0 * 3.0) -> Réponse: 6.0
[Serveur - BiDi Stream] Reçu (15.0 * 4.0) -> Réponse: 60.0
[Serveur - BiDi Stream] Reçu (1.5 * 6.0) -> Réponse: 9.0
[Serveur - BiDi Stream] Flux client terminé.
```

### Console Client (`CalculClient`)
```text
==================================================
1. TESTS RPC UNAIRES (Standard)
==================================================
[Client] 2^16 = 65536
[Client] 12.5 * 4.0 = 50.0

==================================================
2. TEST SERVER STREAMING (GenererPuissances)
==================================================
[Client - Stream Serveur] 2^0 = 1
[Client - Stream Serveur] 2^1 = 2
[Client - Stream Serveur] 2^2 = 4
[Client - Stream Serveur] 2^3 = 8
[Client - Stream Serveur] 2^4 = 16
[Client - Stream Serveur] 2^5 = 32
[Client - Stream Serveur] 2^6 = 64

==================================================
3. TEST BIDIRECTIONAL STREAMING (MultStream)
==================================================
[Client - BiDi Envoi] 2.0 * 3.0
[Client - BiDi Reçu] Résultat multiplication = 6.0
[Client - BiDi Envoi] 15.0 * 4.0
[Client - BiDi Reçu] Résultat multiplication = 60.0
[Client - BiDi Envoi] 1.5 * 6.0
[Client - BiDi Reçu] Résultat multiplication = 9.0
[Client - BiDi] Fin du flux confirmé par le serveur.

[Client] Tous les tests ont été exécutés avec succès !
```

---

## 6. Guide d'Exécution

### Méthode 1 : En ligne de commande (Scripts Batch fournis)

1. **Lancer le serveur :**
   Double-cliquer sur `run-server.bat` ou exécuter :
   ```cmd
   .\run-server.bat
   ```
2. **Lancer le client (dans un autre terminal) :**
   Double-cliquer sur `run-client.bat` ou exécuter :
   ```cmd
   .\run-client.bat
   ```

### Méthode 2 : Dans votre IDE (IntelliJ IDEA / Eclipse / VS Code)

1. Ouvrir le dossier en tant que projet Maven.
2. L'IDE détectera automatiquement le `pom.xml` et le dossier des classes générées `target/generated-sources/protobuf`.
3. Lancer la méthode `main()` de `CalculServer.java`.
4. Lancer la méthode `main()` de `CalculClient.java`.

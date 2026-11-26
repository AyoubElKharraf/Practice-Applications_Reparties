# Guide Exhaustif & Rapport de TP : Architecture Répartie avec gRPC et Protocol Buffers en Java

---

## 1. Contexte, Problématique et Objectif du TP

### 1.1. Contexte académique
Dans le cadre du cours d'**Applications Réparties (Master M2I)**, nous étudions l'évolution des paradigmes de communication inter-processus et des appels de procédures distantes :
1. **Sockets Berkeley (TCP/UDP en C et Java)** : Communication bas niveau via échange direct d'octets / messages bruts. Nécessite une gestion manuelle de la synchronisation, du formatage et du cycle de vie des connexions.
2. **ONC RPC / Sun RPC (en C)** : Introduction de la transparence de localisation. Un compilateur d'interface (`rpcgen`) génère le code de sérialisation XDR et les stubs client/serveur à partir d'un contrat `.x`.
3. **Java RMI (Remote Method Invocation)** : Transposition de l'approche RPC au monde objet en Java. Utilise le protocole JRMP et la sérialisation native Java (`java.io.Serializable`).
4. **gRPC (Google Remote Procedure Call)** : Standard moderne de l'industrie pour les architectures distribuées, les microservices et l'informatique en nuage (*cloud native*).

### 1.2. Objectif du TP
Ce TP transpose le problème classique du cours (les opérations `puiss2` et `multxy`) vers la technologie **gRPC en Java**. 
Les objectifs pédagogiques sont :
* Comprendre le rôle fondamental de l'**IDL (Interface Definition Language)** avec **Protocol Buffers v3**.
* Découvrir le cycle de compilation et de génération de stubs via le compilateur `protoc`.
* Comprendre le transport binaire multiplexé sur **HTTP/2**.
* Implémenter et expérimenter les différents modèles d'interaction :
  * **RPC Unaire** (Requête unique $\rightarrow$ Réponse unique).
  * **Server Streaming** (Requête unique $\rightarrow$ Flux de réponses).
  * **Streaming Bidirectionnel** (Flux de requêtes $\rightleftharpoons$ Flux de réponses asynchrones).

---

## 2. Fondements Théoriques de gRPC & Protocol Buffers

### 2.1. Qu'est-ce que Protocol Buffers (Protobuf) ?
**Protocol Buffers** est un mécanisme développé par Google pour sérialiser des structures de données de manière neutre vis-à-vis du langage et de la plateforme, avec un niveau de compacité et de rapidité nettement supérieur au JSON ou au XML.

#### Fonctionnement
1. **Définition dans un fichier `.proto`** : On décrit la structure des messages et les signatures des services.
2. **Numérotation des champs (`tag`)** : Chaque attribut possède un numéro unique (ex: `int32 n = 1;`). Ce numéro identifie le champ dans le binaire sérialisé. Le nom textuel du champ n'est jamais transmis sur le réseau, ce qui réduit drastiquement la taille des paquets.
3. **Typage strict et rétrocompatibilité** : Il est possible d'ajouter ou de déprécier des champs sans casser la compatibilité avec d'anciennes versions de clients ou serveurs.

### 2.2. Le Protocole de Transport : HTTP/2
Contrairement aux API REST classiques (souvent basées sur HTTP/1.1) ou à Java RMI (sockets TCP dédiées), gRPC repose exclusivement sur **HTTP/2**. Les bénéfices fondamentaux sont :
* **Multiplexage sur une seule connexion TCP** : Plusieurs requêtes et réponses voyagent en parallèle sur la même socket via des flux logiques (*streams*), éliminant le problème de blocage en tête de ligne (*Head-of-Line Blocking* au niveau applicatif).
* **Compression des en-têtes (HPACK)** : Réduit la surcharge métadonnée à chaque appel.
* **Trames binaires (*Binary Framing*)** : Au lieu de texte ASCII lourd à parser, HTTP/2 découpe les échanges en petites trames binaires optimisées pour le matériel réseau.
* **Support natif du Streaming** : Permet au client, au serveur ou aux deux d'émettre des séquences de données au fil de l'eau sans rouvrir de connexion.

### 2.3. Les 4 Modèles de Communication dans gRPC
gRPC prend en charge 4 types d'interactions RPC :
1. **Unaire (Unary RPC)** : Le client envoie une requête unique et attend une seule réponse. C'est l'équivalent direct de l'appel de fonction distant classique (comme en C RPC ou en RMI).
2. **Server Streaming** : Le client envoie une requête et reçoit en retour un flux de messages. Le client lit ce flux jusqu'à ce que le serveur indique qu'il n'y a plus de messages.
3. **Client Streaming** : Le client envoie une série de messages au serveur, attend qu'il les traite tous, puis reçoit une réponse unique.
4. **Streaming Bidirectionnel (BiDi Streaming)** : Les deux côtés envoient un flux de messages de manière indépendante. La lecture et l'écriture sont complètement asynchrones.

---

## 3. Analyse Comparative Approfondie : RPC vs RMI vs gRPC

| Dimension | Sun / ONC RPC (C) | Java RMI | gRPC (Google) |
| :--- | :--- | :--- | :--- |
| **Langage cible** | C / C++ | Strictement Java | Polyglotte (Java, Python, Go, C++, C#, Rust...) |
| **Langage de description d'interface** | Fichier `.x` (RPCL) | Interface Java (`extends Remote`) | Fichier `.proto` (Protobuf v3) |
| **Générateur de code** | Utilitaire `rpcgen` | Compilateur `rmic` (ou proxies dynamiques) | Compilateur `protoc` + plugin gRPC Java |
| **Format de sérialisation** | **XDR** (*External Data Representation*) | Sérialisation Java binaire native | **Protocol Buffers** (binaire compressé à tags) |
| **Couche Transport** | UDP ou TCP brut (Ports assignés dynamiquement) | TCP brut via protocole propriétaire **JRMP** | **HTTP/2** standardisé (port unique, pare-feu friendly) |
| **Résolution de nom & Service** | Service `portmap` / `rpcbind` | Service d'annuaire `rmiregistry` | Intégré ou délégable (DNS, Consul, Kubernetes CoreDNS) |
| **Modes d'appels** | Unaire synchrone | Unaire synchrone | Unaire, Server Stream, Client Stream, Bidirectionnel |
| **Contrôle de flux (Flow Control)** | Aucun au niveau applicatif | Aucun au niveau applicatif | Natif HTTP/2 par fenêtre de flux (*Window-based*) |
| **Gestion des erreurs** | Codes numériques RPC standards | Exceptions Java propagées (`RemoteException`) | Codes d'état formels (`io.grpc.Status` : NOT_FOUND, INVALID_ARGUMENT, etc.) |

---

## 4. Organisation et Structure du Projet

```text
TP 4 - MiniProjet RPC/
├── pom.xml                                    # Configuration Maven du projet
├── DOCUMENTATION_COMPLETE_TP.md              # Le présent guide détaillé et exhaustif
├── README.md                                  # Guide de démarrage rapide
├── generate-proto.bat                         # Script de compilation de l'interface .proto
├── run-server.bat                             # Script de lancement du Serveur
├── run-client.bat                             # Script de lancement du Client
├── target/                                    # Fichiers compilés et bibliothèques
│   ├── classes/                               # Bytecode (.class) de l'application
│   ├── lib/                                   # Dépendances JAR gRPC, Netty, Protobuf
│   └── generated-sources/protobuf/            # Code source Java généré par protoc
│       ├── java/ma/ump/grpc/                  # Classes de messages Protobuf
│       └── grpc-java/ma/ump/grpc/             # Stubs client et squelettes serveurs
└── src/
    └── main/
        ├── proto/
        │   └── calcule.proto                  # Contrat d'interface formel
        └── java/
            └── ma/ump/grpc/
                ├── CalculServiceImpl.java     # Implémentation métier du service
                ├── CalculServer.java          # Programme principal Serveur
                └── CalculClient.java          # Programme principal Client
```

---

## 5. Explication Détaillée du Code Source

### 5.1. Le Contrat d'Interface (`src/main/proto/calcule.proto`)

```protobuf
syntax = "proto3";

// Options de compilation pour le générateur Java
option java_multiple_files = true;
option java_package = "ma.ump.grpc";
option java_outer_classname = "CalculeProto";

package calcule;

// Définition du Service distant
service CalculService {
  // 1. Appel Unaire classique : Calcule 2^n
  rpc Puiss2 (Puiss2Request) returns (Puiss2Reply);

  // 2. Appel Unaire classique : Calcule x * y
  rpc MultXY (MultXYRequest) returns (MultXYReply);

  // 3. Server Streaming : Calcule et émet 2^0 jusqu'à 2^n en flux continu
  rpc GenererPuissances (Puiss2Request) returns (stream Puiss2Reply);

  // 4. Streaming Bidirectionnel : Flux continu de couples (x, y) et résultats en temps réel
  rpc MultStream (stream MultXYRequest) returns (stream MultXYReply);
}

// Structures des messages de requête et de réponse
message Puiss2Request {
  int32 n = 1; // Le tag '= 1' identifie ce champ sur le réseau
}

message Puiss2Reply {
  int32 exposant = 1;
  int64 resultat = 2; // int64 (long en Java) pour éviter les dépassements de capacité
}

message MultXYRequest {
  float x = 1;
  float y = 2;
}

message MultXYReply {
  float resultat = 1;
}
```

#### Explications clés :
* `syntax = "proto3"` : Spécifie la version 3 de Protobuf. En proto3, tous les champs sont optionnels par défaut et les valeurs par défaut sont standardisées (0 pour les nombres, chaîne vide pour les chaînes).
* `java_multiple_files = true` : Indique à `protoc` de créer une classe Java distincte par message au lieu de tout imbriquer dans une seule classe géante.
* `stream` : Préfixé devant le type de retour ou de requête, il active le mode de streaming HTTP/2.

---

### 5.2. L'Implémentation du Service (`CalculServiceImpl.java`)

Le service n'implémente pas une interface classique, mais hérite de `CalculServiceGrpc.CalculServiceImplBase`, classe abstraite générée automatiquement par gRPC.

```java
package ma.ump.grpc;

import io.grpc.Status;
import io.grpc.stub.StreamObserver;

public class CalculServiceImpl extends CalculServiceGrpc.CalculServiceImplBase {

    // ==========================================
    // 1. MÉTHODE UNAIRE : puiss2
    // ==========================================
    @Override
    public void puiss2(Puiss2Request request, StreamObserver<Puiss2Reply> responseObserver) {
        int n = request.getN();

        // Validation métier des paramètres
        if (n < 0 || n >= 63) {
            responseObserver.onError(
                Status.INVALID_ARGUMENT
                    .withDescription("L'exposant n doit être compris entre 0 et 62")
                    .asRuntimeException()
            );
            return;
        }

        // Calcul de 2^n par décalage de bits (bitshift)
        long res = 1L << n;
        System.out.println("[Serveur] puiss2(n=" + n + ") -> " + res);

        // Construction du message de réponse avec le Builder Protobuf
        Puiss2Reply reply = Puiss2Reply.newBuilder()
                .setExposant(n)
                .setResultat(res)
                .build();

        // Envoi de la réponse au client
        responseObserver.onNext(reply);
        // Clôture de l'appel unaire
        responseObserver.onCompleted();
    }

    // ==========================================
    // 2. MÉTHODE UNAIRE : multXY
    // ==========================================
    @Override
    public void multXY(MultXYRequest request, StreamObserver<MultXYReply> responseObserver) {
        float res = request.getX() * request.getY();
        System.out.println("[Serveur] multXY(x=" + request.getX() + ", y=" + request.getY() + ") -> " + res);

        MultXYReply reply = MultXYReply.newBuilder()
                .setResultat(res)
                .build();

        responseObserver.onNext(reply);
        responseObserver.onCompleted();
    }

    // ==========================================
    // 3. SERVER STREAMING : genererPuissances
    // ==========================================
    @Override
    public void genererPuissances(Puiss2Request request, StreamObserver<Puiss2Reply> responseObserver) {
        int max = Math.min(request.getN(), 62);
        System.out.println("[Serveur - Server Streaming] Génération puissances de 0 à " + max);

        for (int i = 0; i <= max; i++) {
            long res = 1L << i;
            Puiss2Reply reply = Puiss2Reply.newBuilder()
                    .setExposant(i)
                    .setResultat(res)
                    .build();

            // Pousse chaque valeur immédiatement dans le flux HTTP/2
            responseObserver.onNext(reply);
        }

        // Indique au client que l'intégralité du flux a été transmise
        responseObserver.onCompleted();
    }

    // ==========================================
    // 4. STREAMING BIDIRECTIONNEL : multStream
    // ==========================================
    @Override
    public StreamObserver<MultXYRequest> multStream(StreamObserver<MultXYReply> responseObserver) {
        // Le serveur retourne un observateur pour traiter les messages arrivant du client
        return new StreamObserver<MultXYRequest>() {
            @Override
            public void onNext(MultXYRequest req) {
                float res = req.getX() * req.getY();
                System.out.println("[Serveur - BiDi Stream] Reçu (" + req.getX() + " * " + req.getY() + ") -> Réponse: " + res);
                
                // Réponse immédiate envoyée au client pour cette opération spécifique
                MultXYReply reply = MultXYReply.newBuilder()
                        .setResultat(res)
                        .build();
                responseObserver.onNext(reply);
            }

            @Override
            public void onError(Throwable t) {
                System.err.println("[Serveur - BiDi Stream] Erreur: " + t.getMessage());
            }

            @Override
            public void onCompleted() {
                System.out.println("[Serveur - BiDi Stream] Flux client terminé.");
                // Fermeture du flux de réponse serveur
                responseObserver.onCompleted();
            }
        };
    }
}
```

#### Notions fondamentales sur `StreamObserver<T>` :
* gRPC adopte le modèle de programmation réactive (*Observer pattern*).
* `onNext(T value)` : Délivre un message sur le canal de transmission.
* `onError(Throwable t)` : Termine la communication de façon anormale en propageant un statut d'erreur gRPC.
* `onCompleted()` : Signale la fin propre et normale de l'émission des données.

---

### 5.3. Le Serveur gRPC (`CalculServer.java`)

```java
package ma.ump.grpc;

import io.grpc.Server;
import io.grpc.ServerBuilder;
import java.io.IOException;

public class CalculServer {
    // Port d'écoute choisi pour éviter les conflits locaux
    public static final int PORT = 63676;

    public static void main(String[] args) throws IOException, InterruptedException {
        // Création et démarrage de l'instance de serveur Netty
        Server server = ServerBuilder.forPort(PORT)
                .addService(new CalculServiceImpl()) // Enregistrement du service
                .build()
                .start();

        System.out.println("[Serveur] gRPC démarré sur le port " + PORT);

        // Mécanisme d'arrêt propre (libération des ressources à l'arrêt du programme)
        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            System.out.println("[Serveur] Arrêt en cours...");
            server.shutdown();
            System.out.println("[Serveur] Arrêté avec succès.");
        }));

        // Bloque le thread principal afin de maintenir le serveur actif
        server.awaitTermination();
    }
}
```

---

### 5.4. Le Client gRPC (`CalculClient.java`)

```java
package ma.ump.grpc;

import io.grpc.ManagedChannel;
import io.grpc.ManagedChannelBuilder;
import io.grpc.stub.StreamObserver;

import java.util.Iterator;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

public class CalculClient {
    public static void main(String[] args) throws InterruptedException {
        // 1. Établissement du canal HTTP/2 vers le serveur distant
        ManagedChannel channel = ManagedChannelBuilder
                .forAddress("localhost", 63676)
                .usePlaintext() // Pas de chiffrement SSL/TLS (environnement de test local)
                .build();

        try {
            // Création des deux types de stubs :
            // - BlockingStub : pour appels synchrones (bloquants)
            CalculServiceGrpc.CalculServiceBlockingStub blockingStub =
                    CalculServiceGrpc.newBlockingStub(channel);
            // - AsyncStub : pour appels asynchrones / streaming bidirectionnel
            CalculServiceGrpc.CalculServiceStub asyncStub =
                    CalculServiceGrpc.newStub(channel);

            // ==========================================
            // TEST 1 : RPC Unaires
            // ==========================================
            System.out.println("==================================================");
            System.out.println("1. TESTS RPC UNAIRES (Standard)");
            System.out.println("==================================================");

            Puiss2Reply repPuiss = blockingStub.puiss2(
                    Puiss2Request.newBuilder().setN(16).build()
            );
            System.out.println("[Client] 2^16 = " + repPuiss.getResultat());

            MultXYReply repMult = blockingStub.multXY(
                    MultXYRequest.newBuilder().setX(12.5f).setY(4.0f).build()
            );
            System.out.println("[Client] 12.5 * 4.0 = " + repMult.getResultat());

            // ==========================================
            // TEST 2 : Server Streaming
            // ==========================================
            System.out.println("\n==================================================");
            System.out.println("2. TEST SERVER STREAMING (GenererPuissances)");
            System.out.println("==================================================");
            
            // Avec un blockingStub, le flux serveur est manipulable comme un simple Iterator Java
            Iterator<Puiss2Reply> streamPuissances = blockingStub.genererPuissances(
                    Puiss2Request.newBuilder().setN(6).build()
            );
            while (streamPuissances.hasNext()) {
                Puiss2Reply reply = streamPuissances.next();
                System.out.println("[Client - Stream Serveur] 2^" + reply.getExposant() + " = " + reply.getResultat());
            }

            // ==========================================
            // TEST 3 : Streaming Bidirectionnel
            // ==========================================
            System.out.println("\n==================================================");
            System.out.println("3. TEST BIDIRECTIONAL STREAMING (MultStream)");
            System.out.println("==================================================");

            // Verrou de synchronisation pour attendre la réponse du serveur asynchrone
            CountDownLatch latch = new CountDownLatch(1);

            // Observateur pour écouter les réponses du serveur
            StreamObserver<MultXYReply> responseObserver = new StreamObserver<MultXYReply>() {
                @Override
                public void onNext(MultXYReply reply) {
                    System.out.println("[Client - BiDi Reçu] Résultat multiplication = " + reply.getResultat());
                }

                @Override
                public void onError(Throwable t) {
                    System.err.println("[Client - BiDi] Erreur reçue: " + t.getMessage());
                    latch.countDown();
                }

                @Override
                public void onCompleted() {
                    System.out.println("[Client - BiDi] Fin du flux confirmé par le serveur.");
                    latch.countDown();
                }
            };

            // Obtention de l'observateur d'émission client
            StreamObserver<MultXYRequest> requestObserver = asyncStub.multStream(responseObserver);

            float[][] couples = {
                    {2.0f, 3.0f},
                    {15.0f, 4.0f},
                    {1.5f, 6.0f}
            };

            // Émission des éléments l'un après l'autre
            for (float[] pair : couples) {
                System.out.println("[Client - BiDi Envoi] " + pair[0] + " * " + pair[1]);
                requestObserver.onNext(MultXYRequest.newBuilder().setX(pair[0]).setY(pair[1]).build());
                Thread.sleep(200);
            }

            // Signalement au serveur que le client a fini d'émettre
            requestObserver.onCompleted();

            // Attente de la réception complète des réponses du serveur
            latch.await(5, TimeUnit.SECONDS);

            System.out.println("\n[Client] Tous les tests ont été exécutés avec succès !");

        } finally {
            // Fermeture propre du canal de communication
            channel.shutdown().awaitTermination(3, TimeUnit.SECONDS);
        }
    }
}
```

---

## 6. Guide Pas à Pas pour Exécuter le Projet

### Méthode Recommandée : Utilisation des Scripts Batch (Simple et Rapide)

1. Ouvrez un terminal dans le répertoire du projet ou utilisez l'Explorateur Windows.
2. **Étape 1 : Démarrer le Serveur**
   Double-cliquez sur `run-server.bat` ou lancez :
   ```cmd
   .\run-server.bat
   ```
   *Le serveur démarre et se place en attente sur le port 63676.*

3. **Étape 2 : Démarrer le Client**
   Dans un **deuxième** terminal, double-cliquez sur `run-client.bat` ou lancez :
   ```cmd
   .\run-client.bat
   ```
   *Le client s'exécute, affiche l'intégralité des résultats et conserve la fenêtre ouverte (`pause`).*

---

### Méthode Manuelle : Commandes Java Directes

Si vous préférez exécuter les commandes à la main :

```powershell
# 1. Se placer dans le répertoire du projet
cd "c:\Users\DELL\Documents\Master\Master_M2I\SEMESTRE 3\Module 1 - Applications Réparties\TPs\TP 4 - MiniProjet RPC"

# 2. Terminal 1 : Lancement du serveur
java -cp "target/classes;target/lib/*" ma.ump.grpc.CalculServer

# 3. Terminal 2 : Lancement du client
java -cp "target/classes;target/lib/*" ma.ump.grpc.CalculClient
```

---

## 7. Traces Réelles d'Exécution Validées

### Sortie Console du Serveur
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

### Sortie Console du Client
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

## 8. Synthèse et Conclusion Pédagogique

Ce TP illustre le saut technologique accompli entre les systèmes RPC historiques et les architectures distribuées modernes :

1. **Compacité et Performance** : Grâce à Protocol Buffers, les messages transitent sous une forme binaire ultra-légère (plus rapide à sérialiser et désérialiser que Java Serializable ou JSON).
2. **Efficacité Réseau avec HTTP/2** : Une seule connexion persistante et multiplexée gère simultanément de multiples flux d'appels, là où RMI ouvrait fréquemment des connexions TCP multiples et où Sun RPC nécessitait des conversions complexes via XDR et le démon `portmap`.
3. **Flexibilité des flux (Streaming)** : gRPC démystifie le temps réel et l'asynchronisme en intégrant le streaming de façon native sans avoir recours à des technologies tierces comme WebSockets ou des files de messages (Message Brokers).
4. **Indépendance vis-à-vis des langages** : Bien que ce TP soit codé en Java, le client aurait pu être écrit en Python ou en C++ sans modifier une seule ligne du serveur Java, illustrant l'interopérabilité totale garantie par le contrat `.proto`.

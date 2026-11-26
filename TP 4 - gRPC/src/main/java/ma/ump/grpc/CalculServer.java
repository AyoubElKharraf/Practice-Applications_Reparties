package ma.ump.grpc;

import io.grpc.Server;
import io.grpc.ServerBuilder;

import java.io.IOException;

public class CalculServer {
    public static final int PORT = 63676;

    public static void main(String[] args) throws IOException, InterruptedException {
        Server server = ServerBuilder.forPort(PORT)
                .addService(new CalculServiceImpl())
                .build()
                .start();

        System.out.println("[Serveur] gRPC démarré sur le port " + PORT);

        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            System.out.println("[Serveur] Arrêt en cours...");
            server.shutdown();
            System.out.println("[Serveur] Arrêté avec succès.");
        }));

        server.awaitTermination();
    }
}

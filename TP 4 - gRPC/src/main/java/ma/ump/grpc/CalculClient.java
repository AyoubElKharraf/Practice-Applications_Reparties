package ma.ump.grpc;

import io.grpc.ManagedChannel;
import io.grpc.ManagedChannelBuilder;
import io.grpc.stub.StreamObserver;

import java.util.Iterator;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

public class CalculClient {
    public static void main(String[] args) throws InterruptedException {
        ManagedChannel channel = ManagedChannelBuilder
                .forAddress("localhost", 63676)
                .usePlaintext()
                .build();

        try {
            CalculServiceGrpc.CalculServiceBlockingStub blockingStub =
                    CalculServiceGrpc.newBlockingStub(channel);
            CalculServiceGrpc.CalculServiceStub asyncStub =
                    CalculServiceGrpc.newStub(channel);

            System.out.println("==================================================");
            System.out.println("1. TESTS RPC UNAIRES (Standard)");
            System.out.println("==================================================");

            // Appel Puiss2
            Puiss2Reply repPuiss = blockingStub.puiss2(
                    Puiss2Request.newBuilder().setN(16).build()
            );
            System.out.println("[Client] 2^16 = " + repPuiss.getResultat());

            // Appel MultXY
            MultXYReply repMult = blockingStub.multXY(
                    MultXYRequest.newBuilder().setX(12.5f).setY(4.0f).build()
            );
            System.out.println("[Client] 12.5 * 4.0 = " + repMult.getResultat());

            System.out.println("\n==================================================");
            System.out.println("2. TEST SERVER STREAMING (GenererPuissances)");
            System.out.println("==================================================");
            Iterator<Puiss2Reply> streamPuissances = blockingStub.genererPuissances(
                    Puiss2Request.newBuilder().setN(6).build()
            );
            while (streamPuissances.hasNext()) {
                Puiss2Reply reply = streamPuissances.next();
                System.out.println("[Client - Stream Serveur] 2^" + reply.getExposant() + " = " + reply.getResultat());
            }

            System.out.println("\n==================================================");
            System.out.println("3. TEST BIDIRECTIONAL STREAMING (MultStream)");
            System.out.println("==================================================");
            CountDownLatch latch = new CountDownLatch(1);

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

            StreamObserver<MultXYRequest> requestObserver = asyncStub.multStream(responseObserver);

            float[][] couples = {
                    {2.0f, 3.0f},
                    {15.0f, 4.0f},
                    {1.5f, 6.0f}
            };

            for (float[] pair : couples) {
                System.out.println("[Client - BiDi Envoi] " + pair[0] + " * " + pair[1]);
                requestObserver.onNext(MultXYRequest.newBuilder().setX(pair[0]).setY(pair[1]).build());
                Thread.sleep(200);
            }

            requestObserver.onCompleted();

            boolean finished = latch.await(5, TimeUnit.SECONDS);
            if (!finished) {
                System.err.println("[Client] Délai d'attente dépassé pour le streaming.");
            }

            System.out.println("\n[Client] Tous les tests ont été exécutés avec succès !");

        } finally {
            channel.shutdown().awaitTermination(3, TimeUnit.SECONDS);
        }
    }
}

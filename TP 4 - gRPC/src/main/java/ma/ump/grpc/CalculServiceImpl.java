package ma.ump.grpc;

import io.grpc.Status;
import io.grpc.stub.StreamObserver;

public class CalculServiceImpl extends CalculServiceGrpc.CalculServiceImplBase {

    @Override
    public void puiss2(Puiss2Request request, StreamObserver<Puiss2Reply> responseObserver) {
        int n = request.getN();
        if (n < 0 || n >= 63) {
            responseObserver.onError(
                Status.INVALID_ARGUMENT
                    .withDescription("L'exposant n doit être compris entre 0 et 62")
                    .asRuntimeException()
            );
            return;
        }

        long res = 1L << n;
        System.out.println("[Serveur] puiss2(n=" + n + ") -> " + res);

        Puiss2Reply reply = Puiss2Reply.newBuilder()
                .setExposant(n)
                .setResultat(res)
                .build();

        responseObserver.onNext(reply);
        responseObserver.onCompleted();
    }

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
            responseObserver.onNext(reply);
        }
        responseObserver.onCompleted();
    }

    @Override
    public StreamObserver<MultXYRequest> multStream(StreamObserver<MultXYReply> responseObserver) {
        return new StreamObserver<MultXYRequest>() {
            @Override
            public void onNext(MultXYRequest req) {
                float res = req.getX() * req.getY();
                System.out.println("[Serveur - BiDi Stream] Reçu (" + req.getX() + " * " + req.getY() + ") -> Réponse: " + res);
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
                responseObserver.onCompleted();
            }
        };
    }
}

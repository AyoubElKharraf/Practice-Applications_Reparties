package ma.ump.grpc;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.68.1)",
    comments = "Source: calcule.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class CalculServiceGrpc {

  private CalculServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "calcule.CalculService";

  // Static method descriptors that strictly reflect the proto.
  private static volatile io.grpc.MethodDescriptor<ma.ump.grpc.Puiss2Request,
      ma.ump.grpc.Puiss2Reply> getPuiss2Method;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "Puiss2",
      requestType = ma.ump.grpc.Puiss2Request.class,
      responseType = ma.ump.grpc.Puiss2Reply.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<ma.ump.grpc.Puiss2Request,
      ma.ump.grpc.Puiss2Reply> getPuiss2Method() {
    io.grpc.MethodDescriptor<ma.ump.grpc.Puiss2Request, ma.ump.grpc.Puiss2Reply> getPuiss2Method;
    if ((getPuiss2Method = CalculServiceGrpc.getPuiss2Method) == null) {
      synchronized (CalculServiceGrpc.class) {
        if ((getPuiss2Method = CalculServiceGrpc.getPuiss2Method) == null) {
          CalculServiceGrpc.getPuiss2Method = getPuiss2Method =
              io.grpc.MethodDescriptor.<ma.ump.grpc.Puiss2Request, ma.ump.grpc.Puiss2Reply>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "Puiss2"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.ProtoUtils.marshaller(
                  ma.ump.grpc.Puiss2Request.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.ProtoUtils.marshaller(
                  ma.ump.grpc.Puiss2Reply.getDefaultInstance()))
              .setSchemaDescriptor(new CalculServiceMethodDescriptorSupplier("Puiss2"))
              .build();
        }
      }
    }
    return getPuiss2Method;
  }

  private static volatile io.grpc.MethodDescriptor<ma.ump.grpc.MultXYRequest,
      ma.ump.grpc.MultXYReply> getMultXYMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "MultXY",
      requestType = ma.ump.grpc.MultXYRequest.class,
      responseType = ma.ump.grpc.MultXYReply.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<ma.ump.grpc.MultXYRequest,
      ma.ump.grpc.MultXYReply> getMultXYMethod() {
    io.grpc.MethodDescriptor<ma.ump.grpc.MultXYRequest, ma.ump.grpc.MultXYReply> getMultXYMethod;
    if ((getMultXYMethod = CalculServiceGrpc.getMultXYMethod) == null) {
      synchronized (CalculServiceGrpc.class) {
        if ((getMultXYMethod = CalculServiceGrpc.getMultXYMethod) == null) {
          CalculServiceGrpc.getMultXYMethod = getMultXYMethod =
              io.grpc.MethodDescriptor.<ma.ump.grpc.MultXYRequest, ma.ump.grpc.MultXYReply>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "MultXY"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.ProtoUtils.marshaller(
                  ma.ump.grpc.MultXYRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.ProtoUtils.marshaller(
                  ma.ump.grpc.MultXYReply.getDefaultInstance()))
              .setSchemaDescriptor(new CalculServiceMethodDescriptorSupplier("MultXY"))
              .build();
        }
      }
    }
    return getMultXYMethod;
  }

  private static volatile io.grpc.MethodDescriptor<ma.ump.grpc.Puiss2Request,
      ma.ump.grpc.Puiss2Reply> getGenererPuissancesMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GenererPuissances",
      requestType = ma.ump.grpc.Puiss2Request.class,
      responseType = ma.ump.grpc.Puiss2Reply.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<ma.ump.grpc.Puiss2Request,
      ma.ump.grpc.Puiss2Reply> getGenererPuissancesMethod() {
    io.grpc.MethodDescriptor<ma.ump.grpc.Puiss2Request, ma.ump.grpc.Puiss2Reply> getGenererPuissancesMethod;
    if ((getGenererPuissancesMethod = CalculServiceGrpc.getGenererPuissancesMethod) == null) {
      synchronized (CalculServiceGrpc.class) {
        if ((getGenererPuissancesMethod = CalculServiceGrpc.getGenererPuissancesMethod) == null) {
          CalculServiceGrpc.getGenererPuissancesMethod = getGenererPuissancesMethod =
              io.grpc.MethodDescriptor.<ma.ump.grpc.Puiss2Request, ma.ump.grpc.Puiss2Reply>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GenererPuissances"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.ProtoUtils.marshaller(
                  ma.ump.grpc.Puiss2Request.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.ProtoUtils.marshaller(
                  ma.ump.grpc.Puiss2Reply.getDefaultInstance()))
              .setSchemaDescriptor(new CalculServiceMethodDescriptorSupplier("GenererPuissances"))
              .build();
        }
      }
    }
    return getGenererPuissancesMethod;
  }

  private static volatile io.grpc.MethodDescriptor<ma.ump.grpc.MultXYRequest,
      ma.ump.grpc.MultXYReply> getMultStreamMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "MultStream",
      requestType = ma.ump.grpc.MultXYRequest.class,
      responseType = ma.ump.grpc.MultXYReply.class,
      methodType = io.grpc.MethodDescriptor.MethodType.BIDI_STREAMING)
  public static io.grpc.MethodDescriptor<ma.ump.grpc.MultXYRequest,
      ma.ump.grpc.MultXYReply> getMultStreamMethod() {
    io.grpc.MethodDescriptor<ma.ump.grpc.MultXYRequest, ma.ump.grpc.MultXYReply> getMultStreamMethod;
    if ((getMultStreamMethod = CalculServiceGrpc.getMultStreamMethod) == null) {
      synchronized (CalculServiceGrpc.class) {
        if ((getMultStreamMethod = CalculServiceGrpc.getMultStreamMethod) == null) {
          CalculServiceGrpc.getMultStreamMethod = getMultStreamMethod =
              io.grpc.MethodDescriptor.<ma.ump.grpc.MultXYRequest, ma.ump.grpc.MultXYReply>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.BIDI_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "MultStream"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.ProtoUtils.marshaller(
                  ma.ump.grpc.MultXYRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.ProtoUtils.marshaller(
                  ma.ump.grpc.MultXYReply.getDefaultInstance()))
              .setSchemaDescriptor(new CalculServiceMethodDescriptorSupplier("MultStream"))
              .build();
        }
      }
    }
    return getMultStreamMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static CalculServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<CalculServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<CalculServiceStub>() {
        @java.lang.Override
        public CalculServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new CalculServiceStub(channel, callOptions);
        }
      };
    return CalculServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static CalculServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<CalculServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<CalculServiceBlockingStub>() {
        @java.lang.Override
        public CalculServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new CalculServiceBlockingStub(channel, callOptions);
        }
      };
    return CalculServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static CalculServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<CalculServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<CalculServiceFutureStub>() {
        @java.lang.Override
        public CalculServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new CalculServiceFutureStub(channel, callOptions);
        }
      };
    return CalculServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     * <pre>
     * 1. Unaire : Calcule 2^n
     * </pre>
     */
    default void puiss2(ma.ump.grpc.Puiss2Request request,
        io.grpc.stub.StreamObserver<ma.ump.grpc.Puiss2Reply> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getPuiss2Method(), responseObserver);
    }

    /**
     * <pre>
     * 2. Unaire : Calcule x * y
     * </pre>
     */
    default void multXY(ma.ump.grpc.MultXYRequest request,
        io.grpc.stub.StreamObserver<ma.ump.grpc.MultXYReply> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getMultXYMethod(), responseObserver);
    }

    /**
     * <pre>
     * 3. Server Streaming : Calcule et diffuse 2^0 jusqu'à 2^n
     * </pre>
     */
    default void genererPuissances(ma.ump.grpc.Puiss2Request request,
        io.grpc.stub.StreamObserver<ma.ump.grpc.Puiss2Reply> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGenererPuissancesMethod(), responseObserver);
    }

    /**
     * <pre>
     * 4. Streaming Bidirectionnel : Flux continu de (x, y) et réponses au fil de l'eau
     * </pre>
     */
    default io.grpc.stub.StreamObserver<ma.ump.grpc.MultXYRequest> multStream(
        io.grpc.stub.StreamObserver<ma.ump.grpc.MultXYReply> responseObserver) {
      return io.grpc.stub.ServerCalls.asyncUnimplementedStreamingCall(getMultStreamMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service CalculService.
   */
  public static abstract class CalculServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return CalculServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service CalculService.
   */
  public static final class CalculServiceStub
      extends io.grpc.stub.AbstractAsyncStub<CalculServiceStub> {
    private CalculServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected CalculServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new CalculServiceStub(channel, callOptions);
    }

    /**
     * <pre>
     * 1. Unaire : Calcule 2^n
     * </pre>
     */
    public void puiss2(ma.ump.grpc.Puiss2Request request,
        io.grpc.stub.StreamObserver<ma.ump.grpc.Puiss2Reply> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getPuiss2Method(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * 2. Unaire : Calcule x * y
     * </pre>
     */
    public void multXY(ma.ump.grpc.MultXYRequest request,
        io.grpc.stub.StreamObserver<ma.ump.grpc.MultXYReply> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getMultXYMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * 3. Server Streaming : Calcule et diffuse 2^0 jusqu'à 2^n
     * </pre>
     */
    public void genererPuissances(ma.ump.grpc.Puiss2Request request,
        io.grpc.stub.StreamObserver<ma.ump.grpc.Puiss2Reply> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getGenererPuissancesMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * 4. Streaming Bidirectionnel : Flux continu de (x, y) et réponses au fil de l'eau
     * </pre>
     */
    public io.grpc.stub.StreamObserver<ma.ump.grpc.MultXYRequest> multStream(
        io.grpc.stub.StreamObserver<ma.ump.grpc.MultXYReply> responseObserver) {
      return io.grpc.stub.ClientCalls.asyncBidiStreamingCall(
          getChannel().newCall(getMultStreamMethod(), getCallOptions()), responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service CalculService.
   */
  public static final class CalculServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<CalculServiceBlockingStub> {
    private CalculServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected CalculServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new CalculServiceBlockingStub(channel, callOptions);
    }

    /**
     * <pre>
     * 1. Unaire : Calcule 2^n
     * </pre>
     */
    public ma.ump.grpc.Puiss2Reply puiss2(ma.ump.grpc.Puiss2Request request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getPuiss2Method(), getCallOptions(), request);
    }

    /**
     * <pre>
     * 2. Unaire : Calcule x * y
     * </pre>
     */
    public ma.ump.grpc.MultXYReply multXY(ma.ump.grpc.MultXYRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getMultXYMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * 3. Server Streaming : Calcule et diffuse 2^0 jusqu'à 2^n
     * </pre>
     */
    public java.util.Iterator<ma.ump.grpc.Puiss2Reply> genererPuissances(
        ma.ump.grpc.Puiss2Request request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getGenererPuissancesMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service CalculService.
   */
  public static final class CalculServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<CalculServiceFutureStub> {
    private CalculServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected CalculServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new CalculServiceFutureStub(channel, callOptions);
    }

    /**
     * <pre>
     * 1. Unaire : Calcule 2^n
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<ma.ump.grpc.Puiss2Reply> puiss2(
        ma.ump.grpc.Puiss2Request request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getPuiss2Method(), getCallOptions()), request);
    }

    /**
     * <pre>
     * 2. Unaire : Calcule x * y
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<ma.ump.grpc.MultXYReply> multXY(
        ma.ump.grpc.MultXYRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getMultXYMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_PUISS2 = 0;
  private static final int METHODID_MULT_XY = 1;
  private static final int METHODID_GENERER_PUISSANCES = 2;
  private static final int METHODID_MULT_STREAM = 3;

  private static final class MethodHandlers<Req, Resp> implements
      io.grpc.stub.ServerCalls.UnaryMethod<Req, Resp>,
      io.grpc.stub.ServerCalls.ServerStreamingMethod<Req, Resp>,
      io.grpc.stub.ServerCalls.ClientStreamingMethod<Req, Resp>,
      io.grpc.stub.ServerCalls.BidiStreamingMethod<Req, Resp> {
    private final AsyncService serviceImpl;
    private final int methodId;

    MethodHandlers(AsyncService serviceImpl, int methodId) {
      this.serviceImpl = serviceImpl;
      this.methodId = methodId;
    }

    @java.lang.Override
    @java.lang.SuppressWarnings("unchecked")
    public void invoke(Req request, io.grpc.stub.StreamObserver<Resp> responseObserver) {
      switch (methodId) {
        case METHODID_PUISS2:
          serviceImpl.puiss2((ma.ump.grpc.Puiss2Request) request,
              (io.grpc.stub.StreamObserver<ma.ump.grpc.Puiss2Reply>) responseObserver);
          break;
        case METHODID_MULT_XY:
          serviceImpl.multXY((ma.ump.grpc.MultXYRequest) request,
              (io.grpc.stub.StreamObserver<ma.ump.grpc.MultXYReply>) responseObserver);
          break;
        case METHODID_GENERER_PUISSANCES:
          serviceImpl.genererPuissances((ma.ump.grpc.Puiss2Request) request,
              (io.grpc.stub.StreamObserver<ma.ump.grpc.Puiss2Reply>) responseObserver);
          break;
        default:
          throw new AssertionError();
      }
    }

    @java.lang.Override
    @java.lang.SuppressWarnings("unchecked")
    public io.grpc.stub.StreamObserver<Req> invoke(
        io.grpc.stub.StreamObserver<Resp> responseObserver) {
      switch (methodId) {
        case METHODID_MULT_STREAM:
          return (io.grpc.stub.StreamObserver<Req>) serviceImpl.multStream(
              (io.grpc.stub.StreamObserver<ma.ump.grpc.MultXYReply>) responseObserver);
        default:
          throw new AssertionError();
      }
    }
  }

  public static final io.grpc.ServerServiceDefinition bindService(AsyncService service) {
    return io.grpc.ServerServiceDefinition.builder(getServiceDescriptor())
        .addMethod(
          getPuiss2Method(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              ma.ump.grpc.Puiss2Request,
              ma.ump.grpc.Puiss2Reply>(
                service, METHODID_PUISS2)))
        .addMethod(
          getMultXYMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              ma.ump.grpc.MultXYRequest,
              ma.ump.grpc.MultXYReply>(
                service, METHODID_MULT_XY)))
        .addMethod(
          getGenererPuissancesMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              ma.ump.grpc.Puiss2Request,
              ma.ump.grpc.Puiss2Reply>(
                service, METHODID_GENERER_PUISSANCES)))
        .addMethod(
          getMultStreamMethod(),
          io.grpc.stub.ServerCalls.asyncBidiStreamingCall(
            new MethodHandlers<
              ma.ump.grpc.MultXYRequest,
              ma.ump.grpc.MultXYReply>(
                service, METHODID_MULT_STREAM)))
        .build();
  }

  private static abstract class CalculServiceBaseDescriptorSupplier
      implements io.grpc.protobuf.ProtoFileDescriptorSupplier, io.grpc.protobuf.ProtoServiceDescriptorSupplier {
    CalculServiceBaseDescriptorSupplier() {}

    @java.lang.Override
    public com.google.protobuf.Descriptors.FileDescriptor getFileDescriptor() {
      return ma.ump.grpc.CalculeProto.getDescriptor();
    }

    @java.lang.Override
    public com.google.protobuf.Descriptors.ServiceDescriptor getServiceDescriptor() {
      return getFileDescriptor().findServiceByName("CalculService");
    }
  }

  private static final class CalculServiceFileDescriptorSupplier
      extends CalculServiceBaseDescriptorSupplier {
    CalculServiceFileDescriptorSupplier() {}
  }

  private static final class CalculServiceMethodDescriptorSupplier
      extends CalculServiceBaseDescriptorSupplier
      implements io.grpc.protobuf.ProtoMethodDescriptorSupplier {
    private final java.lang.String methodName;

    CalculServiceMethodDescriptorSupplier(java.lang.String methodName) {
      this.methodName = methodName;
    }

    @java.lang.Override
    public com.google.protobuf.Descriptors.MethodDescriptor getMethodDescriptor() {
      return getServiceDescriptor().findMethodByName(methodName);
    }
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (CalculServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .setSchemaDescriptor(new CalculServiceFileDescriptorSupplier())
              .addMethod(getPuiss2Method())
              .addMethod(getMultXYMethod())
              .addMethod(getGenererPuissancesMethod())
              .addMethod(getMultStreamMethod())
              .build();
        }
      }
    }
    return result;
  }
}

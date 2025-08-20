package main

import (
	"context"
	"log"
	"time"

	pb "github.com/shoshtari/utils/grpc_example/api/proto"

	"google.golang.org/grpc"
	"google.golang.org/grpc/metadata"
)

type apiKey struct{}

// GetRequestMetadata implements credentials.PerRPCCredentials.
func (a apiKey) GetRequestMetadata(ctx context.Context, uri ...string) (map[string]string, error) {
	return map[string]string{
		"foo2": "bar2",
	}, nil
}

// RequireTransportSecurity implements credentials.PerRPCCredentials.
func (a apiKey) RequireTransportSecurity() bool {
	return false
}

func main() {
	conn, err := grpc.Dial("localhost:50051", grpc.WithInsecure(), grpc.WithPerRPCCredentials(apiKey{}))
	if err != nil {
		log.Fatalf("did not connect: %v", err)
	}
	defer conn.Close()

	c := pb.NewGreeterClient(conn)

	ctx, cancel := context.WithTimeout(context.Background(), time.Second)
	ctx = metadata.AppendToOutgoingContext(ctx, "foo", "bar")
	defer cancel()

	r, err := c.SayHello(ctx, &pb.HelloRequest{Name: "World"})
	if err != nil {
		log.Fatalf("could not greet: %v", err)
	}
	log.Printf("Greeting: %s", r.Message)
}

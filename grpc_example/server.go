package main

import (
	"context"
	"fmt"
	"log"
	"net"

	pb "github.com/shoshtari/utils/grpc_example/api/proto"

	"google.golang.org/grpc"
	"google.golang.org/grpc/metadata"
)

type greeterServer struct {
	pb.UnimplementedGreeterServer
}

func (s *greeterServer) SayHello(ctx context.Context, req *pb.HelloRequest) (*pb.HelloReply, error) {
	if md, ok := metadata.FromIncomingContext(ctx); ok {
		fmt.Println(md)
	}
	fmt.Println("Got request")
	return &pb.HelloReply{Message: fmt.Sprintf("Hello, %s!", req.Name)}, nil
}

func main() {
	lis, err := net.Listen("tcp", ":50051")
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}

	s := grpc.NewServer()
	pb.RegisterGreeterServer(s, &greeterServer{})

	log.Println("Server listening on :50051")
	if err := s.Serve(lis); err != nil {
		log.Fatalf("failed to serve: %v", err)
	}
}

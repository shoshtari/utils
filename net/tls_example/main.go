package main

import (
	"crypto/tls"
	"flag"
	"fmt"
	"log"
	"net/http"
)

var addr string
var tlsEnable bool

func handler(w http.ResponseWriter, r *http.Request) {
	fmt.Fprintf(w, "Salam your address is %s\ntls status in final server is %v", r.RemoteAddr, tlsEnable)
}

func main() {
	flag.StringVar(&addr, "address", ":8000", "address of the server")
	flag.BoolVar(&tlsEnable, "tls", false, "is tls enabled")
	flag.Parse()

	var config *tls.Config
	if tlsEnable {
		cert, err := tls.LoadX509KeyPair("server.crt", "server.key")
		if err != nil {
			log.Fatalf("Failed to load cert and key: %s", err)
		}
		config = &tls.Config{
			Certificates: []tls.Certificate{cert},
			NextProtos:   []string{"h2", "http/1.1"},
		}
	}

	server := &http.Server{
		Addr:      addr,
		TLSConfig: config,
		Handler:   http.HandlerFunc(handler),
	}

	fmt.Println("Starting server on ", addr)
	if tlsEnable {
		if err := server.ListenAndServeTLS("", ""); err != nil {
			log.Fatalf("Failed to start server: %s", err)
		}
	} else {
		if err := server.ListenAndServe(); err != nil {
			log.Fatalf("Failed to start server: %s", err)
		}
	}

}

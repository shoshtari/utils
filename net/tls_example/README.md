# introduction 
this is a simple example of TLS. You can use nginx to terminate SSL or use the golang server. You can use self-signed certificates or use a lets encrypt generated certificate.
## Cert generation 
you can run the `make_cert` script to generate certs. By default, it generates a self-signed certificate but you can change its mode and use it to generate a lets encrypt certificate.
## Docker 
I used a multistage dockerfile for my server which is written in golang. Also, there is a docker-compose file to help you bring up the server and nginx. After using docker-compose, there will be two available ports in your host `8080` and `8443`. `8080` will use plaintext http and `8443` will use TLS.
## nginx 
I configured nginx to listen on port `80` and `443` and forward traffic to the server which I used its internal docker address format since they were brought up together in docker-compose 
## Server 
the server is a simple golang program that listens for connection and in response, returns a simple message saying the client IP address (it may not be what the client believes its IP is since there can be NAT on its path to the server)


FROM golang:1.23.2 AS build

COPY go.mod .
COPY go.sum .
RUN go mod download 

COPY . . 
RUN CGO_ENABLED=0 go build -o /opt/server main.go 



FROM scratch
COPY --from=build /opt/server /opt/server
EXPOSE 80

CMD ["/opt/server", "-address", ":80"]

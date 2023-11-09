package main

import (
	"fmt"
	"github.com/gin-gonic/gin"
	"os"
	"serve/api"
	"serve/internal"
)

func getRootDir() string {
	if len(os.Args) == 1 {
		return "."
	}
	return os.Args[0]
}

func main() {

	c := internal.ParseConfigs()

	r := gin.Default()
	api.AddRoutes(r, c.RootDir)
	r.Run(fmt.Sprintf(":%d", c.Port))
}

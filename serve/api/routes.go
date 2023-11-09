package api

import "github.com/gin-gonic/gin"

func AddRoutes(r *gin.Engine, path string) {
	r.StaticFS("/", gin.Dir(path, true))
}

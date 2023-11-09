package internal

import (
	"flag"
)

type ServeConfig struct {
	RootDir string
	Port    int
}

func ParseConfigs() ServeConfig {

	var ans ServeConfig
	flag.StringVar(&ans.RootDir, "root", ".", "Root directory")
	flag.IntVar(&ans.Port, "port", 8080, "Port to listen on")

	flag.Parse()

	if ans.RootDir == "." && len(flag.Args()) > 0 {
		ans.RootDir = flag.Args()[0]
	}

	return ans

}

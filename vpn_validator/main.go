package main

import (
	"fmt"
	"io"
	"log"
	"net/http"
	"net/url"
	"os"
	"strings"
)

const SUBLINK string = "https://raw.githubusercontent.com/barry-far/V2ray-Configs/main/Sub3.txt"
const SUBFILE = "/tmp/subs.txt"

func getSubDataFromLink() ([]byte, error) {
	res, err := http.Get(SUBLINK)
	if err != nil {
		return nil, err
	}

	return io.ReadAll(res.Body)

}
func getSubDataFromFile() ([]byte, error) {
	return os.ReadFile(SUBFILE)
}

func getSubData() []string {
	data, err := getSubDataFromFile()
	if err != nil {
		log.Println("couldn't get sub data from file, err: ", err)
		data, err = getSubDataFromLink()
		if err != nil {
			log.Println("couldn't get sub data from url, err: ", err)
			panic("couldn't get sub data")
		}
	}
	splittedData := strings.Split(string(data), "\n")
	return splittedData
}

func getLinks(subData []string) []*url.URL {

	var links []*url.URL
	for _, l := range subData {
		tmp := strings.TrimSpace(l)
		link := strings.Split(tmp, " ")[0]
		if strings.Contains(link, "#") {
			link = link[:strings.Index(link, "#")]
		}
		if len(link) > 0 {
			u, err := url.Parse(link)
			if err != nil {
				log.Println("dropping invalid url: ", link)
				continue
			}
			links = append(links, u)
		}
	}

	return links
}

func checkUrl(u *url.URL){
	
}

func main() {
	subData := getSubData()
	log.Printf("Got %d links from sub", len(subData))
	links := getLinks(subData)
	log.Printf("Parsed %d links", len(links))
	for _, l := range links{
		fmt.Println(l.Host, l.Hostname(), l.Port())
	}
}

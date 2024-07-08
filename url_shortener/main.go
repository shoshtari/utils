package main

import (
	"fmt"
	"log"
	"math/rand"
	"net/http"
)

var data map[string]string

const UrlLength = 4
const addr = ":8000"

func generateRandomString(length int) string {
	var ans string
	alphabet := "abcdefghijklmnopqrstuvwxyz"
	for i := 0; i < length; i++ {
		ans += string(alphabet[rand.Intn(len(alphabet))])
	}
	if _, exists := data[ans]; exists {
		ans = generateRandomString(length)
	}

	return ans
}
func handler(w http.ResponseWriter, r *http.Request) {
	switch r.Method {

	case http.MethodGet:
		fmt.Println(r.URL.String())
		url := r.URL.String()[1:] // avoid precceding '/'
		if original, exists := data[url]; !exists {
			w.WriteHeader(http.StatusNotFound)
			fmt.Fprintf(w, "url not found")
		} else {
			http.Redirect(w, r, original, http.StatusMovedPermanently)
		}
	case http.MethodPost:
		url := r.URL.Query().Get("url")
		if url == "" {
			w.WriteHeader(http.StatusBadRequest)
			fmt.Fprintf(w, "link is empty")
			return
		}

		token := generateRandomString(UrlLength)
		data[token] = url

		fmt.Fprintf(w, token)
	}

}

func main() {
	data = make(map[string]string)

	http.HandleFunc("/", handler)
	log.Println("running on", addr)
	if err := http.ListenAndServe(addr, nil); err != nil {
		panic(err)
	}
}

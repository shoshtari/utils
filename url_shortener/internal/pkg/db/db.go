package db

import (
	"database/sql"

	_ "github.com/mattn/go-sqlite3"
)

func NewDB() {
	conn, err := sql.Open("sqlite", ":memory:")
}

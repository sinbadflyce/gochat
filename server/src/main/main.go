package main

import (
	"fmt"
	"log"
	"net"
	"net/http"

	"github.com/boltdb/bolt"
	"github.com/golang/protobuf/proto"
	"github.com/gorilla/websocket"
)

//global variable for handling all chat traffic
var crowd Crowd

// UDPServer ...
var udpServer UDPServer

// websocket
var upgrader = websocket.Upgrader{
	ReadBufferSize:  1024,
	WriteBufferSize: 1024,
	CheckOrigin:     func(r *http.Request) bool { return true }, //not checking origin
}

// connected fire
func connected(w http.ResponseWriter, r *http.Request) {
	fmt.Println("\nNew connection")
	conn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		fmt.Println("Error upgrading to websocket:", err)
		return
	}

	sessionID := ""
	ok := true

	go func() {

		for {
			_, data, err := conn.ReadMessage()
			if err != nil {
				fmt.Println("\nConnection closed for session " + sessionID)
				fmt.Println("TCP error close-connection:", err)
				crowd.updatePresence(sessionID, false)
				udpServer.CloseClient(sessionID)
				conn.Close()
				return
			}

			wire := &Wire{}
			err = proto.Unmarshal(data, wire)
			if err != nil {
				fmt.Println("\nUnmarshaling error: ", err)
				return
			}

			sessionID, ok = crowd.messageArrived(conn, wire, sessionID)
			if !ok {
				fmt.Println("\nReceived error, stop loop")
				return
			}
		}
	}()
}

// Printing out the various ways the server can be reached by the clients
func printClientConnInfo() {
	addrs, err := net.InterfaceAddrs()
	if err != nil {
		log.Fatal(err)
		return
	}

	fmt.Println("clients can connect at the following addresses:")
	for _, a := range addrs {
		if a.String() != "0.0.0.0" {
			fmt.Println("http://" + a.String() + ":8000/\n")
		}
	}
}

// Database

func openDb() *bolt.DB {
	db, err := bolt.Open("server.db", 0600, nil)
	if err != nil {
		log.Fatal(err)
	}
	return db
}

// main

func main() {
	db := openDb()
	printClientConnInfo()
	http.HandleFunc("/ws", connected)
	crowd.Init(db)
	go udpServer.Start(&crowd)
	http.ListenAndServe(":8000", nil)
	defer db.Close()
}

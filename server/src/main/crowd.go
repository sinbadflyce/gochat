package main

import (
	"fmt"
	"math/rand"
	"net/http"
	"sync"

	"github.com/boltdb/bolt"
	"github.com/gorilla/websocket"
)

// Crowd ...
type Crowd struct {
	clients             map[string]*Client
	presenceSubscribers map[string][]string // set of subscribers to each client
	clientsMtx          sync.Mutex
	queue               chan Wire
	db                  *bolt.DB
}

// Init ...
func (crowd *Crowd) Init(db *bolt.DB) {
	crowd.queue = make(chan Wire, 5)
	crowd.clients = make(map[string]*Client)
	crowd.presenceSubscribers = make(map[string][]string)
	crowd.db = db

	// loop to send messages from queue
	go func() {
		for {
			message := <-crowd.queue
			to := message.GetTo()
			if to == "" {
				fmt.Println("Send " + message.GetWhich().String() + " to whom?")
				return
			}

			client, ok := crowd.clients[to]
			if ok == false {
				fmt.Println("Can't find " + to)
				return
			}

			which := message.Which
			if which != Wire_CONTACTS { // don't forward sessionId
				message.SessionId = ""
			}
			fmt.Printf("Send %s from %s to %s\n", message.GetWhich().String(), message.From, message.To)
			client.Send(&message)
		}
	}()
}

// messageArrived ...
func (crowd *Crowd) messageArrived(conn *websocket.Conn, wire *Wire, sessionId string) (string, bool) {
	if wire.GetWhich() == Wire_LOGIN {
		sessionId := crowd.receivedLogin(conn, wire.GetLogin())
		return sessionId, true
	}
	sessionId = wire.GetSessionId()
	if sessionId != "" {
		fmt.Println("\nSessionId is " + sessionId + " Wire: " + wire.GetWhich().String() + " From " + wire.From + " To " + wire.To)
		crowd.updatePresence(sessionId, true)
	}

	client, ok := crowd.clients[sessionId]
	if !ok {
		if client == nil && sessionId != "" {
			fmt.Println("no client for " + sessionId)
			return sessionId, false
		}

		// session id is empty
		fmt.Println("Session Id is empty, which = " + wire.GetWhich().String())
	}

	switch wire.GetWhich() {
	case Wire_CONTACTS:
		client.receivedContacts(wire)
	case Wire_STORE:
		client.receivedStore(wire)
	case Wire_LOAD:
		client.receivedLoad(wire)
	case Wire_PUBLIC_KEY:
		fallthrough
	case Wire_PUBLIC_KEY_RESPONSE:
		fallthrough
	case Wire_HANDSHAKE:
		fallthrough
	case Wire_PAYLOAD:
		if client == nil {
			fmt.Printf("client is nil %d\n", len(crowd.clients))
		}
		if wire == nil {
			fmt.Println("wire is nil")
		}
		forward(client, wire)
	default:
		fmt.Println("No handler for " + wire.GetWhich().String())
	}
	return sessionId, true
}

// receivedLogin ...
func (crowd *Crowd) receivedLogin(conn *websocket.Conn, login *Login) string {
	fmt.Println("Received Login: " + login.UserName)
	typeLogin := login.Type
	name := login.UserName
	authenToken := login.AuthenToken
	deviceToken := login.DeviceToken

	defer crowd.clientsMtx.Unlock()
	defer crowd.clientsMtx.Lock()

	sessionId := createSessionId()

	var client *Client
	if c, ok := crowd.clients[name]; ok {
		client = c
	} else {
		client = &Client{
			id:          name,
			name:        name,
			sessions:    make(map[string]*websocket.Conn),
			online:      false,
			deviceToken: deviceToken,
		}
		fmt.Println("\ttoken notification: " + deviceToken)
	}
	client.sessions[sessionId] = conn
	crowd.clients[name] = client
	switch typeLogin {
	case 1:
		loginSuccess(client, sessionId, crowd)
	case 2:
		if verifyToken(authenToken) {
			loginSuccess(client, sessionId, crowd)
		} else {
			crowd.clients[sessionId] = client
			client.loginFail(sessionId)
			crowd.updatePresence(sessionId, false)
		}
	}
	return sessionId
}

// loginSuccess ...
func loginSuccess(client *Client, sessionId string, crowd *Crowd) {
	fmt.Printf("\tNew client id = %s, session = %s, len = %d, tokenNotification = %s\n",
		client.id, sessionId, len(client.sessions), client.deviceToken)
	client.Load(crowd.db)
	crowd.clients[sessionId] = client
	client.sendContacts(sessionId)
	crowd.updatePresence(sessionId, true)
}

// verifyToken ...
func verifyToken(token string) bool {
	client := &http.Client{}
	req, err := http.NewRequest("GET", "https://graph.microsoft.com/v1.0/me", nil)
	req.Header.Add("Authorization", "Bearer "+token)
	req.Header.Add("Content-Type", "application/json")
	resp, err := client.Do(req)
	if err != nil {
		fmt.Println("Get Api MS", err)
		return false
	}
	defer resp.Body.Close()
	statusCode := resp.StatusCode
	fmt.Println("\tVerify token: StatusCode: ", statusCode)
	if statusCode == 200 {
		return true
	} else {
		return false
	}
}

// todo: need a real GUID generator
func createSessionId() string {
	alphanum := "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
	var bytes = make([]byte, 10)
	rand.Read(bytes)
	for i, b := range bytes {
		bytes[i] = alphanum[b%byte(len(alphanum))]
	}
	return string(bytes)
}

// updatePresence ...
func (crowd *Crowd) updatePresence(sessionId string, online bool) {
	client, ok := crowd.clients[sessionId]
	if !ok {
		fmt.Println("\t can't find " + sessionId)
		return
	}

	crowd.clients[sessionId] = client
	if online == client.online {
		return
	} else {
		fmt.Printf("Update Presence sessionId = %s online=%t\n", sessionId, online)
	}
	fmt.Printf("sessionId = %s device token = %s\n", sessionId, client.deviceToken)
	client.updatePresence(sessionId, online)

	// inform subscribers
	from := client.id
	name := client.name
	deviceToken := client.deviceToken
	contact := &Contact{
		Id:          from,
		Name:        name,
		Online:      online,
		DeviceToken: deviceToken,
	}
	fmt.Printf("\t from: %s deviceToken: %s\n", name, deviceToken)
	for _, subscriber := range crowd.presenceSubscribers[from] {
		fmt.Println("\t subscriber = " + subscriber)
		update := &Wire{
			Which:    Wire_PRESENCE,
			Contacts: []*Contact{contact},
			To:       subscriber,
		}

		data := crowd.clients[subscriber]
		fmt.Printf("\t deviceToken = %s        contacts length = %d\n", data.deviceToken, len(update.GetContacts()))
		//push notification client online/offline
		if client.online {
			content := map[string]interface{}{"message": from + " is Online"}
			client.pushNotification(content, data.deviceToken)
		} else {
			content := map[string]interface{}{"message": from + " is Offline"}
			client.pushNotification(content, data.deviceToken)
		}

		crowd.queue <- *update
	}
	client.subscribeToContacts()
}

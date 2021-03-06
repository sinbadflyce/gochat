package main

import (
  "github.com/gorilla/websocket"
  "fmt"
  "math/rand"
  "sync"
  "github.com/boltdb/bolt"
)

type Crowd struct {
  clients             map[string]*Client
  presenceSubscribers map[string][]string // set of subscribers to each client
  clientsMtx          sync.Mutex
  queue               chan Wire
  db                  *bolt.DB
}

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
      fmt.Printf("Send %s from %s to %s\n", message.GetWhich().String(), message.From, message.To);
      client.Send(&message)
    }
  }()
}

func (crowd *Crowd) messageArrived(conn *websocket.Conn, wire *Wire, sessionId string) (string, bool) {
  if wire.GetWhich() == Wire_LOGIN {
    sessionId := crowd.receivedLogin(conn, wire.GetLogin())
    return sessionId, true
  }
  sessionId = wire.GetSessionId()
  if sessionId != "" {
    fmt.Println("\nsessionId is " + sessionId)
    crowd.updatePresence(sessionId, true)
  }

  client, ok := crowd.clients[sessionId]
  if !ok {
    if client == nil && sessionId != "" {
      fmt.Println("no client for " + sessionId)
      return sessionId, false
    } else {
      fmt.Println("sessionId is empty, which=" + wire.GetWhich().String())
    }
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

func (crowd *Crowd) receivedLogin(conn *websocket.Conn, id string) string {
  fmt.Println("receivedLogin: " + id)
  defer crowd.clientsMtx.Unlock()
  crowd.clientsMtx.Lock()

  sessionId := createSessionId()

  var client *Client
  if c, ok := crowd.clients[id]; ok {
    client = c
  } else {
    client = &Client{
      id:       id,
      sessions: make(map[string]*websocket.Conn),
      online:   false,
    }
  }
  client.sessions[sessionId] = conn
  crowd.clients[id] = client
  fmt.Printf("new client id=%s, session=%s, len=%d\n", client.id, sessionId, len(client.sessions))
  client.Load(crowd.db)
  crowd.clients[sessionId] = client
  client.sendContacts(sessionId)
  crowd.updatePresence(sessionId, true)
  return sessionId
}

// todo: need a real GUID generator
func createSessionId() string{
  alphanum := "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
  var bytes = make([]byte, 10)
  rand.Read(bytes)
  for i, b := range bytes {
    bytes[i] = alphanum[b%byte(len(alphanum))]
  }
  return string(bytes)
}

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
    fmt.Printf("updatePresence sessionId=%s online=%t\n", sessionId, online)
  }
  client.updatePresence(sessionId, online)

  // inform subscribers
  from := client.id
  contact := &Contact{
    Id: from,
    Online: online,
  }

  for _,subscriber := range crowd.presenceSubscribers[from] {
    fmt.Println("\t subscriber=" + subscriber)
    update := &Wire {
      Which: Wire_PRESENCE,
      Contacts: []*Contact{contact},
      To: subscriber,
    }
    fmt.Printf("\t contacts length=%d\n", len(update.GetContacts()))
    crowd.queue <- *update
  }
  client.subscribeToContacts()
}
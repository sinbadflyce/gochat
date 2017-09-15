package main

import (
  "fmt"
  "github.com/boltdb/bolt"
  "github.com/golang/protobuf/proto"
  "github.com/gorilla/websocket"
  "github.com/google/go-gcm"
  "log"
  "github.com/timehop/apns"
)

type Client struct {
  id          string
  name        string
  contacts    []*Contact
  sessions    map[string]*websocket.Conn
  online      bool
  crowd       *Crowd
  deviceToken string
  platform string
}

func (client *Client) Save(db *bolt.DB, wire *Wire) {
  db.Update(func(tx *bolt.Tx) error {
	b, err := tx.CreateBucketIfNotExists([]byte(client.id))
	if err != nil {
	  fmt.Println("Error opening bucket:", err)
	}
	encoded, err := proto.Marshal(wire)
	if err != nil {
	  fmt.Println("Error marshalling:", err)
	}
	return b.Put([]byte("contacts"), encoded)
  })
}

func (client *Client) Load(db *bolt.DB) {
  db.View(func(tx *bolt.Tx) error {
	b := tx.Bucket([]byte(client.id))
	if b != nil {
	  data := b.Get([]byte("contacts"))
	  if data == nil {
		fmt.Println("contacts are nil for " + client.id)
	  } else {
		var wire Wire
		err := proto.Unmarshal(data, &wire)
		if err != nil {
		  fmt.Println("Error unmarshalling:", err)
		} else {
		  client.contacts = wire.GetContacts()
		}
	  }
	}
	return nil
  })
}

func (client *Client) isOnline() bool {
  return len(client.sessions) > 0
}

func (client *Client) Send(wire *Wire) {
  fmt.Println("Client.Send " + wire.Which.String())
  data, err := proto.Marshal(wire)
  if err != nil || data == nil {
	fmt.Println("Error marshalling:", err)
  } else if client == nil {
	fmt.Println("Send - cl is nil")
  } else {
	fmt.Printf("\t there are %d connections\n", len(client.sessions))
	for _, conn := range client.sessions {
	  conn.WriteMessage(websocket.BinaryMessage, data)
	}
  }
}

func (client *Client) receivedLoad(wire *Wire) {
  crowd.db.View(func(tx *bolt.Tx) error {
	b := tx.Bucket([]byte(client.id))
	if b == nil {
	  fmt.Println("no bucket for " + client.id)
	  return nil
	}
	key := wire.GetPayload()
	data := b.Get(key)
	if data == nil {
	  fmt.Println("data is nil for " + string(key[:]))
	  return nil
	}
	store := &Store{
	  Key: key,
	}
	update := &Wire{
	  Which:   Wire_STORE,
	  Store:   store,
	  To:      client.id,
	  Payload: data,
	}
	crowd.queue <- *update
	return nil
  })
}

//save message
func (client *Client) receivedStore(wire *Wire) {
  crowd.db.Update(func(tx *bolt.Tx) error {
	b, err := tx.CreateBucketIfNotExists([]byte(client.id))
	if err != nil {
	  fmt.Println("Error opening bucket:", err)
	}
	return b.Put(wire.Store.Key, wire.GetPayload())
  })
}

func (client *Client) subscribeToContacts() {
  from := client.id
  fmt.Println("Subscribe to contacts from " + from)
  for _, contact := range client.contacts {
	contactId := contact.GetId()
	fmt.Println("\t contactId = " + contactId)
	if client.online {
	  if _, ok := crowd.presenceSubscribers[contactId]; !ok {
		crowd.presenceSubscribers[contactId] = []string{from}
	  } else {
		crowd.presenceSubscribers[contactId] = append(crowd.presenceSubscribers[contactId], from)
	  }
	} else { // offline
	  crowd.presenceSubscribers[contactId] = remove(crowd.presenceSubscribers[contactId], from)
	  if len(crowd.presenceSubscribers[contactId]) == 0 {
		delete(crowd.presenceSubscribers, contactId)
	  }
	}
  }
}

func (client *Client) updatePresence(sessionId string, online bool) {
  if !online {
	delete(client.sessions, sessionId)
  }
  client.online = len(client.sessions) > 0
}

// remove a string from a list of strings
func remove(s []string, r string) []string {
  for i, v := range s {
	if v == r {
	  return append(s[:i], s[i+1:]...)
	}
  }
  return s
}

func (client *Client) sendContacts(sessionId string) {
  for _, contact := range client.contacts {
	_, ok := crowd.clients[contact.GetId()]
	contact.Online = ok
  }

  buds := &Wire{
	Which:     Wire_CONTACTS,
	SessionId: sessionId,
	Contacts:  client.contacts,
	To:        client.id,
  }
  crowd.queue <- *buds
}

func (client *Client) loginFail(sessionId string) {
  buds := &Wire{
	Which:     Wire_LOGIN_RESPONSE,
	SessionId: sessionId,
	To:        client.id,
  }
  crowd.queue <- *buds
}

func forward(client *Client, wire *Wire) {
  wire.From = client.id
  crowd.queue <- *wire // forward to all devices with source's and destination's ids
}

func (client *Client) receivedContacts(wire *Wire) {
  fmt.Println("Received Contacts for " + client.id)
  client.contacts = wire.GetContacts()
  client.subscribeToContacts()
  client.Save(crowd.db, wire)

  for _, contact := range wire.Contacts {
	if c, ok := crowd.clients[contact.GetId()]; ok {
	  contact.Online = c.online
	}
  }
  wire.To = client.id
  forward(client, wire)
}

func (client *Client) pushNewMessageNotification(wire *Wire) {
	// inform subscribers
	from := client.id
	name := client.name

	for _, subscriber := range crowd.presenceSubscribers[from] {
		data := crowd.clients[subscriber]
		if data.id == wire.To {
			fmt.Println("\nPush notification to: " + data.name + " (" + data.platform + ")" )

			message := name + " has sent a new message to you"
			if data.platform == "ios" || data.platform == "osx" {
				client.pushWithAPNs(message, data.deviceToken, data.platform)
			} else if data.platform == "android" {
				content := map[string]interface{}{"message": message}
				client.pushWithGCM(content, data.deviceToken)
			}
			break
		}
	}
}

func (client *Client) pushWithGCM(data map[string]interface{}, pushToken string) {
  serverKey := "AIzaSyB14mtQyetuI127fV11JGb-bTqVkfBDQJY"
  var msg gcm.HttpMessage
  regIDs := []string{pushToken}

  msg.RegistrationIds = regIDs
  msg.Data = data
  msg.ContentAvailable = true
  response, err := gcm.SendHttp(serverKey, msg)
  if err != nil {
	fmt.Println(err.Error())
  } else {
	fmt.Println("============PUSH NOTIFICATION============")
	fmt.Println("\tResponse ", response.Success)
	fmt.Println("\tMessageID ", response.MessageId)
	fmt.Println("\tFailure ", response.Failure)
	fmt.Println("\tError ", response.Error)
	fmt.Println("\tResults ", response.Results)
	fmt.Println("=========================================")
  }
}

func (client *Client) pushWithAPNs(content string, deviceToken string, platform string) {
	cerfile := ""
	keyFile := ""

	if platform == "ios" {
		cerfile = "CK_DEV_APN_CER.pem"
		keyFile = "CK_DEV_APN_NOPASS_KEY.pem"
	} else if platform == "osx" {
		cerfile = "CK_DEV_MacOS_APN_CER.pem"
		keyFile = "CK_DEV_APN_MACOS_NOPASS_KEY.pem"
	}

	if cerfile == "" {
		fmt.Println("\nThere is no certfile by platform")
		return
	}

	c, err := apns.NewClientWithFiles(apns.SandboxGateway, cerfile, keyFile)
	if err != nil {
		log.Fatal("could not create new client", err.Error())
	}

	go func() {
		for f := range c.FailedNotifs {
			fmt.Println("Notif", f.Notif.ID, "failed with", f.Err.Error())
		}
	}()

	p := apns.NewPayload()
	p.APS.Alert.Body = content
	//p.APS.Badge.Set(1)
	//p.APS.Sound = "turn_down_for_what.aiff"
	p.APS.ContentAvailable = 1

	m := apns.NewNotification()
	m.Payload = p
	m.DeviceToken = deviceToken
	m.Priority = apns.PriorityImmediate
	m.Identifier = 12312       // Integer for APNS
	m.ID = "user_id:timestamp" // ID not sent to Apple – to identify error notifications

	erro := c.Send(m)

	fmt.Println("============PUSH NOTIFICATION============")
	fmt.Println("\tError ", erro)
	fmt.Println("========================================")
}
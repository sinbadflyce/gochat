package main

import (
	fmt "fmt"
	"net"
	"sync"
)

// UDPCrowd ...
type UDPCrowd struct {
	tcpCrowd   *Crowd
	clients    map[string]*UDPClient
	clientsMtx sync.Mutex
	queue      chan Wire
}

// Init ...
func (crowd *UDPCrowd) Init() {

	// make udp crowd properties
	crowd.clients = make(map[string]*UDPClient)
	crowd.queue = make(chan Wire, 10)

	// routine for messages queue
	go func() {
		for {

			// get message from queue
			message := <-crowd.queue

			// only process with payload message
			if message.Which != Wire_PAYLOAD {
				fmt.Println("UDP received wire, only process PAYLOAD, but ingore")
				continue
			}

			// session ID
			to := message.GetTo()
			si := message.GetSessionId()

			// sure that session id is valid
			if to == "" {
				fmt.Println("UDP received wire, cannot process PAYLOAD without TO")
				continue
			}

			// sure that session id is valid
			if si == "" {
				fmt.Println("UDP received wire, cannot process PAYLOAD without session id")
				continue
			}

			fromClient, ok := crowd.clients[si]

			// not exist
			if ok == false {
				fmt.Println("UDP can't find to from client: " + si)
				continue
			}

			// assign message.from by name
			message.From = fromClient.name

			// get to-client from crowd
			toClient, ok := crowd.clients[to]

			// not exist
			if ok == false {
				fmt.Println("UDP can't find to client " + to)
				continue
			}

			// send
			toClient.send(&message)
		}
	}()
}

// MessageArrived ...
func (crowd *UDPCrowd) MessageArrived(peerAddr *net.UDPAddr, conn *net.UDPConn, wire *Wire) {

	// process login
	if wire.Which == Wire_LOGIN {

		// login
		login := wire.Login
		name := login.UserName
		var client *UDPClient

		// already exist client
		if c, ok := crowd.clients[name]; ok {
			client = c
		} else {

			// not yet, make this by name
			client = &UDPClient{
				name,
				name,
				conn,
				peerAddr,
			}
		}

		// keep client by name
		if client != nil {
			crowd.clients[name] = client
			fmt.Printf("UDP create client with name = %s, sessionId = %s\n", client.name, client.id)
		} else {
			fmt.Printf("UDP cannot create client with name = %s\n", name)
		}

		// response login
		w := &Wire{
			Which: Wire_LOGIN_RESPONSE,
			To:    client.id,
		}
		client.send(w)

		// wire is established
	} else if wire.Which == Wire_UDP_ESTABLISHED {

		// now, create client by session id
		c := &UDPClient{
			wire.SessionId,
			wire.From,
			conn,
			peerAddr,
		}

		// keep client by session id
		crowd.clients[wire.SessionId] = c
		fmt.Printf("UDP create client with name = %s, sessionId = %s\n", c.name, c.id)

		// response UDP established
		w := &Wire{
			Which:     Wire_UDP_ESTABLISHED,
			SessionId: wire.SessionId,
			To:        c.id,
		}
		c.send(w)
	}

	// put wire to the queue
	crowd.queue <- *wire
}

// UpdateClientSession ...
func (crowd *UDPCrowd) UpdateClientSession(sessionID string) {

	// get client
	client, ok := crowd.clients[sessionID]
	if !ok {
		fmt.Println("UDP can't find " + sessionID)
		return
	}

	// re-assign
	crowd.clients[sessionID] = client
}

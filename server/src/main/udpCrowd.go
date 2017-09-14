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
	crowd.clients = make(map[string]*UDPClient)
	crowd.queue = make(chan Wire, 5)

	go func() {
		for {
			message := <-crowd.queue

			if message.Which != Wire_PAYLOAD {
				fmt.Println("UDP received wire, but ingore")
				continue
			}

			to := message.GetTo()

			client, ok := crowd.clients[to]
			message.From = client.name

			if ok == false {
				fmt.Println("UDP can't find client " + to)
				continue
			}

			// send
			client.send(&message)
		}
	}()
}

// MessageArrived ...
func (crowd *UDPCrowd) MessageArrived(peerAddr *net.UDPAddr, conn *net.UDPConn, wire *Wire) {
	if wire.Which == Wire_UDP_ESTABLISHED {
		var c = new(UDPClient)
		c.id = wire.SessionId
		c.name = wire.From
		c.address = peerAddr
		c.conn = conn
		crowd.clients[c.name] = c
		fmt.Printf("Create UDP client with name = %s, sessionId = %s\n", c.name, c.id)
	}

	sessionID := wire.GetSessionId()

	if sessionID != "" {
		crowd.tcpCrowd.updatePresence(sessionID, true)
	}

	fmt.Printf("UDP crowd received wire, which = %d\n", wire.Which)
	crowd.queue <- *wire
}

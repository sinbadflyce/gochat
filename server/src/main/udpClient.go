package main

import (
	"fmt"
	"net"

	proto "github.com/golang/protobuf/proto"
)

// UDPClient ...
type UDPClient struct {
	id      string
	name    string
	conn    *net.UDPConn
	address *net.UDPAddr
}

// UDPClient.send via wire ...
func (client *UDPClient) send(wire *Wire) {

	if client == nil {
		fmt.Printf("Client is not available\n")
		return
	}

	// check to be available client connection
	if client.conn == nil {
		fmt.Printf("Client %s doesn't connect yet\n", client.name)
		return
	}

	// encode data
	data, err := proto.Marshal(wire)

	// error
	if err != nil {
		fmt.Printf("Proto.Marshal gets error %s\n", err.Error())
		return
	}

	// send udp to client address

	len := len(data)
	var totalSent = 0

	// split bytes to send
	for len > 0 {

		// UDP max bytes to send
		var bysend = 4096

		// less than, use len
		if len < bysend {
			bysend = len
		}

		// move to bytes range to send
		bytes := data[totalSent:(totalSent + bysend)]

		// send
		n, err := client.conn.WriteToUDP(bytes, client.address)

		// error
		if err != nil {
			fmt.Printf("WriteToUDP gets error %s\n", err.Error())
			return
		}

		// logging
		fmt.Printf("UDP sent %d bytes to from %s to %s\n", n, wire.From, client.name)

		totalSent = totalSent + bysend
		len = len - bysend
	}
}

// UDPClient.receive
func (client *UDPClient) receive(wire *Wire) {

	// nothing implemented
	fmt.Println("UDP client recevied not implemented yet")
}

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

	// check to be available client connection
	if client.conn == nil {
		fmt.Printf("Client %s doesn't connect yet\n", client.name)
		return
	}

	data, err := proto.Marshal(wire)

	if err != nil {
		fmt.Printf("Proto.Marshal gets error %s\n", err.Error())
		return
	}

	n, err := client.conn.WriteToUDP(data, client.address)

	if err != nil {
		fmt.Printf("WriteToUDP gets error %s\n", err.Error())
		return
	}

	fmt.Printf("Sent %d bytes to client [%s]\n", n, client.address.String())
}

// UDPClient.receive
func (client *UDPClient) receive(wire *Wire) {
}

package main

import "net"

// UDPClient ...
type UDPClient struct {
	id       string
	name     string
	sessions map[string]*net.UDPConn
	udpCrowd *UDPCrowd
}

// UDPClient.send via wire ...
func (client *UDPClient) send(wire *Wire) {
}

// UDPClient.receive
func (client *UDPClient) receive(wire *Wire) {
}

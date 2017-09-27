package main

import (
	fmt "fmt"
	"net"

	proto "github.com/golang/protobuf/proto"
)

// UDPServer ...
type UDPServer struct {
	udpCrowd      *UDPCrowd
	conn          *net.UDPConn
	buffer        []byte
	maxSizeBuffer int
}

// Start ...
func (server *UDPServer) Start(tcpCrowd *Crowd) {
	server.udpCrowd = new(UDPCrowd)
	server.udpCrowd.tcpCrowd = tcpCrowd
	server.maxSizeBuffer = 65536
	server.udpCrowd.Init()
	server.buffer = make([]byte, server.maxSizeBuffer)

	//Build the address
	udpAddr, err := net.ResolveUDPAddr("udp", ":8001")
	if err != nil {
		fmt.Println("Wrong Address")
		return
	}

	udpConn, err := net.ListenUDP("udp", udpAddr)
	if err != nil {
		println(err.Error())
		return
	}

	println("UDP server listen on :8001")

	server.conn = udpConn
	server.HandleReadData()
}

// HandleReadData ...
func (server *UDPServer) HandleReadData() {

	// loop to read data
	for {

		// read
		n, addr, err := server.conn.ReadFromUDP(server.buffer[0:])

		// error
		if err != nil {
			fmt.Println(err.Error())
			return
		}

		// bytes are read
		fmt.Printf("UDP server received bytes lenght: %d\n", n)

		// copy
		readBuf := make([]byte, n)
		copy(readBuf, server.buffer)

		// convert to wire
		wire := &Wire{}
		err = proto.Unmarshal(readBuf, wire)

		// error
		if err != nil {
			fmt.Println(err.Error())
			return
		}

		// fallback to udp crowd
		server.udpCrowd.MessageArrived(addr, server.conn, wire)
	}
}

// CloseClient ...
func (server *UDPServer) CloseClient(sessionID string) {

	// peer client is available
	c1, ok := server.udpCrowd.clients[sessionID]

	// error
	if !ok {
		return
	}

	// remove
	delete(server.udpCrowd.clients, sessionID)
	fmt.Printf("Remove client with session id = %s\n", sessionID)

	_, ok = server.udpCrowd.clients[c1.name]

	// error
	if !ok {
		return
	}

	// remove
	delete(server.udpCrowd.clients, c1.name)
	fmt.Printf("Remove client with session id = %s\n", c1.name)
}

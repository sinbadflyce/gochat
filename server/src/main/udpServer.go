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
func (server *UDPServer) CloseClient(peerName string) {

	// peer client is available
	_, ok := server.udpCrowd.clients[peerName]

	// error
	if !ok {
		fmt.Printf("No client with sessionId = %s\n", peerName)
	}

	// remove
	server.udpCrowd.clients[peerName] = nil
}

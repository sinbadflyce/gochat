package main

import (
	fmt "fmt"
	"net"

	proto "github.com/golang/protobuf/proto"
)

// UDPServer ...
type UDPServer struct {
	crowd *UDPCrowd
	conn  *net.UDPConn
}

// Start ...
func (server *UDPServer) Start() {
	server.crowd = new(UDPCrowd)

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
	buf := make([]byte, 1024)

	for {
		n, err := server.conn.Read(buf[0:])

		if err != nil {
			fmt.Println(err.Error())
			return
		}

		fmt.Printf("UDP server received bytes lenght: %d\n", n)

		wire := &Wire{}
		err = proto.Unmarshal(buf, wire)

		if err != nil {
			fmt.Println(err.Error())
			return
		}

		if wire.Which == Wire_LOGIN {

		}
	}
}

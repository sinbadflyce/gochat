package main

import (
	"sync"
)

// UDPCrowd ...
type UDPCrowd struct {
	clients    map[string]*UDPClient
	clientsMtx sync.Mutex
	queue      chan Wire
}

// Init ...
func (crowd *UDPCrowd) Init() {
	crowd.clients = make(map[string]*UDPClient)
	crowd.queue = make(chan Wire, 5)
}

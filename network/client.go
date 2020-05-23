// Vikman Fernandez-Castro
// May 17, 2020

package network

import (
	"log"
	"net"
	"time"

	"../cluster"
)

// Client is a client structure
type Client struct {
	address string
}

// NewClient creates a new client
func NewClient(address string) *Client {
	return &Client{address}
}

// Handle connects the node and dispatches messages
func (s *Client) Handle() {
	delayTime := time.Second

	for {
		log.Println("INFO: Trying to connect to", s.address)
		conn, err := net.Dial("tcp", s.address)

		if err != nil {
			log.Println("WARNING: ", err)

			time.Sleep(delayTime)

			if delayTime.Seconds() < 60 {
				delayTime += time.Second
			}

			continue
		}

		cluster.HandshakeClient(conn)
		conn.Close()

		// TODO: remove
		time.Sleep(delayTime)
	}
}

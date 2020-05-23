// Vikman Fernandez-Castro
// May 17, 2020

package network

import (
	"log"
	"net"
	"time"

	"../cluster"
	"../codes"
)

// Client is a client structure
type Client struct {
	address string
}

type clientConn struct {
	conn net.Conn
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

		cc := newClientConn(conn)
		cc.handle()

		// TODO: remove
		time.Sleep(delayTime)
	}
}

func newClientConn(conn net.Conn) *clientConn {
	return &clientConn{conn}
}

func (cc *clientConn) handle() {
	defer cc.conn.Close()

	if cc.handshake() {
		log.Println("INFO: Connection granted")
	} else {
		log.Println("INFO: Connection rejected")
	}
}

func (cc *clientConn) handshake() bool {
	helo, err := ReadHelo(cc.conn)
	if err != nil {
		log.Println("WARNING: Cannot receive HELO message from the server:", err)
		return false
	}

	ping := NewPing(cluster.PasswordHash(helo.Salt))

	err = ping.Write(cc.conn)
	if err != nil {
		log.Println("WARNING: Cannot send PING message to the server:", err)
		return false
	}

	pong, err := ReadPong(cc.conn)
	if err != nil {
		log.Println("WARNING: Cannot receive PONG message from the server:", err)
		return false
	}

	if pong.Result == codes.OK {
	} else {
		log.Println("WARNING: Connection denied:", codes.Message(pong.Result))
	}

	return true
}

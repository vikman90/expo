// Vikman Fernandez-Castro
// May 17, 2020

package network

import (
	"bytes"
	"log"
	"net"

	"../cluster"
	"../codes"
)

// Server is a server structure
type Server struct {
	address string
	ln      net.Listener
}

type serverConn struct {
	conn net.Conn
}

// NewServer creates a new server
func NewServer(address string) *Server {
	ln, err := net.Listen("tcp", address)
	if err != nil {
		log.Fatal("FATAL: ", err)
	}

	return &Server{address, ln}
}

// Handle dispatches incoming connections
func (s *Server) Handle() {
	log.Println("INFO: Listening", s.address)
	for {
		conn, err := s.ln.Accept()
		if err != nil {
			log.Println("ERROR: Cannot accept connection:", err)
		}

		sc := newServerConn(conn)
		go sc.handle()
	}
}

func newServerConn(conn net.Conn) *serverConn {
	return &serverConn{conn}
}

func (sc *serverConn) handle() {
	defer sc.conn.Close()

	if sc.handshake() {
		log.Println("INFO: New node connected")
	} else {
		log.Println("INFO: Node rejected")
	}
}

func (sc *serverConn) handshake() bool {
	helo := NewHelo()

	err := helo.Write(sc.conn)
	if err != nil {
		log.Println("WARNING: Cannot send HELO message to the client:", err)
		return false
	}

	ping, err := ReadPing(sc.conn)
	if err != nil {
		log.Println("WARNING: Cannot receive PING message from the client:", err)
		return false
	}

	hash := cluster.PasswordHash(helo.Salt)
	result := bytes.Equal(ping.Password, hash[:])
	var pong *PongType

	if result {
		pong = NewPong(codes.OK)
	} else {
		pong = NewPong(codes.Unauthorized)
	}

	err = pong.Write(sc.conn)
	if err != nil {
		log.Println("WARNING: Cannot send PONG message to the client:", err)
		return false
	}

	return result
}

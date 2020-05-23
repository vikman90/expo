// Vikman Fernandez-Castro
// May 17, 2020

package network

import (
	"log"
	"net"

	"../cluster"
)

// Server is a server structure
type Server struct {
	address string
	ln      net.Listener
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

		go cluster.HandshakeServer(conn)
	}
}

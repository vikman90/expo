// Vikman Fernandez-Castro
// May 17, 2020

package cluster

import (
	"bytes"
	"crypto/rand"
	"crypto/sha256"
	"encoding/json"
	"errors"
	"log"
	"net"

	"../misc"
)

type heloType struct {
	Type string `json:"type"`
	Salt []byte `json:"salt"`
}

type pingType struct {
	Type     string `json:"type"`
	Password []byte `json:"password"`
}

type pongType struct {
	Type   string `json:"type"`
	Result bool   `json:"result"`
	Reason string `json:"reason"`
}

func newHelo() *heloType {
	salt := make([]byte, 8)
	rand.Read(salt)
	return &heloType{"HELO", salt}
}

func writeHelo(conn net.Conn, helo *heloType) error {
	payload, err := json.Marshal(helo)

	if err != nil {
		log.Panic("PANIC: Cannot create JSON:", err)
	}

	_, err = conn.Write(payload)
	return err
}

func readHelo(conn net.Conn) (*heloType, error) {
	buffer := make([]byte, misc.BufferLength)

	n, err := conn.Read(buffer)
	if err != nil {
		return nil, err
	}

	helo := new(heloType)
	err = json.Unmarshal(buffer[:n], helo)

	if helo.Type != "HELO" {
		return nil, errors.New("Message received is not HELO")
	}

	return helo, err
}

func newPing(password []byte) *pingType {
	return &pingType{"PING", password}
}

func writePing(conn net.Conn, salt []byte) error {
	hash := sha256.Sum256(append(salt, cluster.password...))

	payload, err := json.Marshal(newPing(hash[:]))
	if err != nil {
		log.Panic("PANIC: Cannot create JSON:", err)
	}

	_, err = conn.Write(payload)
	return err
}

func readPing(conn net.Conn) (*pingType, error) {
	buffer := make([]byte, misc.BufferLength)

	n, err := conn.Read(buffer)
	if err != nil {
		return nil, err
	}

	ping := new(pingType)
	err = json.Unmarshal(buffer[:n], ping)

	if ping.Type != "PING" {
		return nil, errors.New("Message received is not PING")
	}

	return ping, err
}

func newPong(result bool, reason string) *pongType {
	return &pongType{"PONG", result, reason}
}

func writePong(conn net.Conn, result bool, reason string) error {
	payload, err := json.Marshal(newPong(result, reason))
	if err != nil {
		log.Panic("PANIC: Cannot create JSON:", err)
	}

	_, err = conn.Write(payload)
	return err
}

func readPong(conn net.Conn) (*pongType, error) {
	buffer := make([]byte, misc.BufferLength)

	n, err := conn.Read(buffer)
	if err != nil {
		return nil, err
	}

	pong := new(pongType)
	err = json.Unmarshal(buffer[:n], pong)

	if pong.Type != "PONG" {
		return nil, errors.New("Message received is not PONG")
	}

	return pong, err
}

// HandshakeServer performs a handshake at server-side
func HandshakeServer(conn net.Conn) {
	defer conn.Close()
	helo := newHelo()

	println("DEBUG: New connection")

	err := writeHelo(conn, helo)
	if err != nil {
		log.Println("WARNING: Cannot send HELO message to the client:", err)
		return
	}

	ping, err := readPing(conn)
	if err != nil {
		log.Println("WARNING: Cannot receive PING message to the client:", err)
		return
	}

	hash := sha256.Sum256(append(helo.Salt, cluster.password...))

	if bytes.Equal(ping.Password, hash[:]) {
		println("DEBUG: Access granted.")
		err = writePong(conn, true, "")

		if err != nil {
			log.Println("WARNING: Cannot send PONG message to the client:", err)
			return
		}

		println("DEBUG: Handshake successful.")
	} else {
		println("DEBUG: Access denied.")
		err = writePong(conn, false, "Incorrect password")

		if err != nil {
			log.Println("WARNING: Cannot send PONG message to the client:", err)
		}
	}
}

// HandshakeClient performs a handshake at client-side
func HandshakeClient(conn net.Conn) {
	helo, err := readHelo(conn)
	if err != nil {
		log.Println("WARNING: Cannot receive HELO message from the server:", err)
		return
	}

	err = writePing(conn, helo.Salt)
	if err != nil {
		log.Println("WARNING: Cannot send PING message to the server:", err)
		return
	}

	pong, err := readPong(conn)
	if err != nil {
		log.Println("WARNING: Cannot receive PONG message from the server:", err)
		return
	}

	if pong.Result {
		println("DEBUG: Handshake successful.")
	} else {
		log.Println("WARNING: Connection denied:", pong.Reason)
	}
}

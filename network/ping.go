// Vikman Fernandez-Castro
// May 23, 2020

package network

import (
	"encoding/json"
	"errors"
	"io"
	"log"

	"../misc"
)

// PingType represents a PING message
type PingType struct {
	Type     string `json:"type"`
	Password []byte `json:"password"`
}

// NewPing creates a PING message
func NewPing(password []byte) *PingType {
	return &PingType{"PING", password}
}

// WritePing writes a PING message
func (ping *PingType) Write(conn io.Writer) error {
	payload, err := json.Marshal(ping)
	if err != nil {
		log.Panic("PANIC: Cannot create JSON:", err)
	}

	_, err = conn.Write(payload)
	return err
}

// ReadPing reads a PING message
func ReadPing(conn io.Reader) (*PingType, error) {
	buffer := make([]byte, misc.BufferLength)

	n, err := conn.Read(buffer)
	if err != nil {
		return nil, err
	}

	ping := new(PingType)
	err = json.Unmarshal(buffer[:n], ping)

	if ping.Type != "PING" {
		return nil, errors.New("Message received is not PING")
	}

	return ping, err
}

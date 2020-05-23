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

// PongType represents a PONG message
type PongType struct {
	Type   string `json:"type"`
	Result int    `json:"result"`
}

// NewPong creates a PONG message
func NewPong(result int) *PongType {
	return &PongType{"PONG", result}
}

// WritePong writes a PONG message
func (pong *PongType) Write(conn io.Writer) error {
	payload, err := json.Marshal(pong)
	if err != nil {
		log.Panic("PANIC: Cannot create JSON:", err)
	}

	_, err = conn.Write(payload)
	return err
}

// ReadPong reads a PONG message
func ReadPong(conn io.Reader) (*PongType, error) {
	buffer := make([]byte, misc.BufferLength)

	n, err := conn.Read(buffer)
	if err != nil {
		return nil, err
	}

	pong := new(PongType)
	err = json.Unmarshal(buffer[:n], pong)

	if pong.Type != "PONG" {
		return nil, errors.New("Message received is not PONG")
	}

	return pong, err
}

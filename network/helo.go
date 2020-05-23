// Vikman Fernandez-Castro
// May 23, 2020

package network

import (
	"crypto/rand"
	"encoding/json"
	"errors"
	"io"
	"log"

	"../misc"
)

// HeloType represents a HELO message
type HeloType struct {
	Type string `json:"type"`
	Salt []byte `json:"salt"`
}

// NewHelo creates a HELO message
func NewHelo() *HeloType {
	salt := make([]byte, 8)
	rand.Read(salt)
	return &HeloType{"HELO", salt}
}

// ReadHelo reads a HELO message
func ReadHelo(conn io.Reader) (*HeloType, error) {
	buffer := make([]byte, misc.BufferLength)

	n, err := conn.Read(buffer)
	if err != nil {
		return nil, err
	}

	helo := new(HeloType)
	err = json.Unmarshal(buffer[:n], helo)

	if helo.Type != "HELO" {
		return nil, errors.New("Message received is not HELO")
	}

	return helo, err
}

// WriteHelo writes a HELO message
func (helo *HeloType) Write(conn io.Writer) error {
	payload, err := json.Marshal(helo)

	if err != nil {
		log.Panic("PANIC: Cannot create JSON:", err)
	}

	_, err = conn.Write(payload)
	return err
}

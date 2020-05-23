// Vikman Fernandez-Castro
// May 17, 2020

package cluster

import (
	"crypto/sha256"
	"log"
	"net"
)

type clusterType struct {
	nodeName    string
	password    string
	nodes       map[*net.Conn][]string
	connections map[string]*net.Conn
}

var cluster clusterType

// Setup starts a cluster
func Setup(nodeName, password string) {
	log.Println("INFO: Starting node named:", nodeName)

	cluster.nodeName = nodeName
	cluster.password = password
	cluster.nodes = make(map[*net.Conn][]string)
	cluster.connections = make(map[string]*net.Conn)
}

// CheckPassword tests whether the input password is correct
func CheckPassword(password string) bool {
	return password == cluster.password
}

// NodeName returns the name of the node
func NodeName() string {
	return cluster.nodeName
}

// PasswordHash gets the cluster password hash
func PasswordHash(salt []byte) []byte {
	hash := sha256.Sum256(append(salt, cluster.password...))
	return hash[:]
}

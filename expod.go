// Vikman Fernandez-Castro
// May 17, 2020

package main

import (
	"flag"
	"log"
	"math/rand"
	"os"
	"os/signal"
	"syscall"
	"time"

	"./cluster"
	"./misc"
	"./network"
)

type optionSet struct {
	clientAddress string
	serverAddress string
	nodeName      string
	password      string
}

func main() {
	rand.Seed(time.Now().UnixNano())

	options := parseArguments()

	if len(options.nodeName) == 0 {
		options.nodeName = misc.RandomName()
	}

	if len(options.password) == 0 {
		options.password = misc.RandomPassword()
		log.Println("INFO: Generating password:", options.password)
	}

	cluster.Setup(options.nodeName, options.password)

	if len(options.serverAddress) != 0 {
		mserver := network.NewServer(options.serverAddress)
		go mserver.Handle()
	}

	if len(options.clientAddress) != 0 {
		mclient := network.NewClient(options.clientAddress)
		go mclient.Handle()
	}

	loop()
	log.Println("INFO: Exiting")
}

func loop() {
	exit := make(chan os.Signal)
	signal.Notify(exit, syscall.SIGINT, syscall.SIGTERM)
	s := <-exit

	if s == syscall.SIGINT {
		os.Stderr.WriteString("\n")
	}
}

func parseArguments() *optionSet {
	var clientAddress = flag.String("c", "", "Client mode: connect to address.")
	var nodeName = flag.String("n", "", "Node name.")
	var password = flag.String("p", "", "Cluster password.")
	var serverAddress = flag.String("s", "", "Server mode: bind to address.")

	flag.Parse()

	if len(*clientAddress) == 0 && len(*serverAddress) == 0 {
		os.Stderr.WriteString("ERROR: No server or client defined.\n")
		flag.Usage()
		os.Exit(1)
	}

	if len(*clientAddress) != 0 && len(*password) == 0 {
		os.Stderr.WriteString("ERROR: Password is undefined.\n")
		flag.Usage()
		os.Exit(1)
	}

	return &optionSet{*clientAddress, *serverAddress, *nodeName, *password}
}

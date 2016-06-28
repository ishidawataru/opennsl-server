// Copyright (C) 2016 Nippon Telegraph and Telephone Corporation.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package main

import (
	"io"
	"net"
	"strconv"

	log "github.com/Sirupsen/logrus"
	"github.com/ishidawataru/opennsl-server/client/proto/l2"
	"github.com/ishidawataru/opennsl-server/client/proto/l2service"
	"github.com/spf13/cobra"
	"golang.org/x/net/context"
)

var l2Client l2service.L2Client

func NewL2Cmd() *cobra.Command {
	l2Cmd := &cobra.Command{
		Use: "l2",
		PersistentPreRun: func(cmd *cobra.Command, args []string) {
			conn, err := connGrpc()
			if err != nil {
				log.Fatal(err)
			}
			l2Client = l2service.NewL2Client(conn)
		},
	}

	list := &cobra.Command{
		Use: "list",
		Run: func(cmd *cobra.Command, args []string) {
			res, err := l2Client.List(context.Background(), &l2.ListRequest{})
			if err != nil {
				log.Fatal(err)
			}
			for _, info := range res.List {
				log.Info("response:", info)
			}
		},
	}

	add := &cobra.Command{
		Use: "add",
		Run: func(cmd *cobra.Command, args []string) {
			if len(args) < 2 {
				log.Fatal("l2 add <mac> <port> [<vid>]")
			}
			vid := 1
			mac, err := net.ParseMAC(args[0])
			if err != nil {
				log.Fatal(err)
			}
			port, err := strconv.Atoi(args[1])
			if err != nil {
				log.Fatal(err)
			}
			if len(args) > 2 {
				vid, err = strconv.Atoi(args[2])
				if err != nil {
					log.Fatal(err)
				}
			}
			_, err = l2Client.AddAddress(context.Background(), &l2.AddAddressRequest{
				Address: &l2.Address{
					Flags: (l2.L2Flag_FLAG_L3LOOKUP | l2.L2Flag_FLAG_STATIC),
					Mac:   []byte(mac),
					Vid:   uint32(vid),
					Port:  int64(port),
				},
			})
			if err != nil {
				log.Fatal(err)
			}
		},
	}

	delete := &cobra.Command{
		Use: "delete",
		Run: func(cmd *cobra.Command, args []string) {
			if len(args) < 1 {
				log.Fatal("l2 delete <mac> [<vid>]")
			}
			vid := 1
			mac, err := net.ParseMAC(args[0])
			if err != nil {
				log.Fatal(err)
			}
			if len(args) > 1 {
				vid, err = strconv.Atoi(args[1])
				if err != nil {
					log.Fatal(err)
				}
			}
			_, err = l2Client.DeleteAddress(context.Background(), &l2.DeleteAddressRequest{
				Mac: []byte(mac),
				Vid: uint32(vid),
			})
			if err != nil {
				log.Fatal(err)
			}
		},
	}

	get := &cobra.Command{
		Use: "get",
		Run: func(cmd *cobra.Command, args []string) {
			if len(args) < 1 {
				log.Fatal("l2 get <mac> [<vid>]")
			}
			vid := 1
			mac, err := net.ParseMAC(args[0])
			if err != nil {
				log.Fatal(err)
			}
			if len(args) > 1 {
				vid, err = strconv.Atoi(args[1])
				if err != nil {
					log.Fatal(err)
				}
			}
			_, err = l2Client.GetAddress(context.Background(), &l2.GetAddressRequest{
				Mac: []byte(mac),
				Vid: uint32(vid),
			})
			if err != nil {
				log.Fatal(err)
			}
		},
	}

	monitor := &cobra.Command{
		Use: "monitor",
		Run: func(cmd *cobra.Command, args []string) {
			stream, err := l2Client.Monitor(context.Background(), &l2.MonitorRequest{})
			if err != nil {
				log.Fatal(err)
			}
			for {
				res, err := stream.Recv()
				if err == io.EOF {
					return
				} else if err != nil {
					log.Fatal(err)
				}
				log.Info("response:", res)
			}
		},
	}

	l2Cmd.AddCommand(list, add, delete, get, monitor)
	return l2Cmd
}

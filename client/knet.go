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
	"github.com/ishidawataru/opennsl-server/client/proto/knet"
	"github.com/ishidawataru/opennsl-server/client/proto/knetservice"
	"github.com/spf13/cobra"
	"golang.org/x/net/context"
)

var knetClient knetservice.KNETClient

func NewKNETCmd() *cobra.Command {
	knetCmd := &cobra.Command{
		Use: "knet",
		PersistentPreRun: func(cmd *cobra.Command, args []string) {
			conn, err := connGrpc()
			if err != nil {
				log.Fatal(err)
			}
			knetClient = knetservice.NewKNETClient(conn)
		},
	}

	initCmd := &cobra.Command{
		Use: "init",
		Run: func(cmd *cobra.Command, args []string) {
			_, err := knetClient.Init(context.Background(), &port.InitRequest{})
			if err != nil {
				log.Fatal(err)
			}
		},
	}

	list := &cobra.Command{
		Use: "list",
		Run: func(cmd *cobra.Command, args []string) {
			res, err := knetClient.List(context.Background(), &knet.ListRequest{})
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
				log.Fatal("knet add <mac> <port> [<vid>]")
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
			_, err = knetClient.AddAddress(context.Background(), &knet.AddAddressRequest{
				Address: &knet.Address{
					Flags: (knet.KNETFlag_FLAG_L3LOOKUP | knet.KNETFlag_FLAG_STATIC),
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
				log.Fatal("knet delete <mac> [<vid>]")
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
			_, err = knetClient.DeleteAddress(context.Background(), &knet.DeleteAddressRequest{
				Mac: []byte(mac),
				Vid: uint32(vid),
			})
			if err != nil {
				log.Fatal(err)
			}
		},
	}

	knetCmd.AddCommand(list, initCmd, add, delete)
	return knetCmd
}

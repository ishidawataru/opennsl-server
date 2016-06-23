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
	"strconv"

	log "github.com/Sirupsen/logrus"
	"github.com/ishidawataru/opennsl-server/client/proto/vlan"
	"github.com/ishidawataru/opennsl-server/client/proto/vlanservice"
	"github.com/spf13/cobra"
	"golang.org/x/net/context"
)

var vlanClient vlanservice.VLANClient

func NewVlanCmd() *cobra.Command {
	vlanCmd := &cobra.Command{
		Use: "vlan",
		PersistentPreRun: func(cmd *cobra.Command, args []string) {
			conn, err := connGrpc()
			if err != nil {
				log.Fatal(err)
			}
			vlanClient = vlanservice.NewVLANClient(conn)
		},
	}

	createCmd := &cobra.Command{
		Use: "create",
		Run: func(cmd *cobra.Command, args []string) {
			if len(args) != 1 {
				log.Fatal("usage: vlan create <vid>")
			}
			vid, err := strconv.Atoi(args[0])
			if err != nil {
				log.Fatal(err)
			}
			_, err = vlanClient.Create(context.Background(), &vlan.CreateRequest{
				Vid: uint32(vid),
			})
			if err != nil {
				log.Fatal(err)
			}
		},
	}

	destroyCmd := &cobra.Command{
		Use: "destroy",
		Run: func(cmd *cobra.Command, args []string) {
			if len(args) != 1 {
				log.Fatal("usage: vlan destroy <vid>")
			}
			var err error
			if args[0] == "all" {
				_, err = vlanClient.DestroyAll(context.Background(), &vlan.DestroyAllRequest{})
			} else {
				vid, err := strconv.Atoi(args[0])
				if err != nil {
					log.Fatal(err)
				}
				_, err = vlanClient.Destroy(context.Background(), &vlan.DestroyRequest{
					Vid: uint32(vid),
				})
			}
			if err != nil {
				log.Fatal(err)
			}
		},
	}

	//	addCmd := &cobra.Command{
	//		Use: "add",
	//		Run: func(cmd *cobra.Command, args []string) {
	//		},
	//	}

	listCmd := &cobra.Command{
		Use: "list",
		Run: func(cmd *cobra.Command, args []string) {
			res, err := vlanClient.List(context.Background(), &vlan.ListRequest{})
			if err != nil {
				log.Fatal(err)
			}
			for _, data := range res.List {
				log.Info("response:", data)
			}
		},
	}

	vlanCmd.AddCommand(createCmd, destroyCmd, listCmd)
	return vlanCmd
}

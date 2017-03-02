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
	"fmt"
	"strconv"
	"strings"

	log "github.com/Sirupsen/logrus"
	"github.com/hydratim/opennsl-man/client/proto/port"
	"github.com/hydratim/opennsl-man/client/proto/portservice"
	"github.com/hydratim/opennsl-man/client/proto/vlan"
	"github.com/hydratim/opennsl-man/client/proto/vlanservice"
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

	addCmd := &cobra.Command{
		Use: "add",
		Run: func(cmd *cobra.Command, args []string) {
			if len(args) < 2 {
				log.Fatal("usage: vlan add <vid> <ports> [ untagged | tagged ]")
			}
			vid, err := strconv.Atoi(args[0])
			if err != nil {
				log.Fatal(err)
			}
			pbmp, err := ParsePBMP(args[1])
			if err != nil {
				log.Fatal(err)
			}
			if len(args) > 2 && args[2] == "tagged" {
				_, err = vlanClient.PortAdd(context.Background(), &vlan.PortAddRequest{
					Vid:    uint32(vid),
					Pbmp:   pbmp,
					UtPbmp: NewPBMP(),
				})
			} else {
				_, err = vlanClient.PortAdd(context.Background(), &vlan.PortAddRequest{
					Vid:    uint32(vid),
					Pbmp:   pbmp,
					UtPbmp: pbmp,
				})
			}
			if err != nil {
				log.Fatal(err)
			}
		},
	}

	removeCmd := &cobra.Command{
		Use: "remove",
		Run: func(cmd *cobra.Command, args []string) {
			if len(args) < 2 {
				log.Fatal("usage: vlan remove <vid> <ports>")
			}
			vid, err := strconv.Atoi(args[0])
			if err != nil {
				log.Fatal(err)
			}
			pbmp, err := ParsePBMP(args[1])
			if err != nil {
				log.Fatal(err)
			}
			_, err = vlanClient.PortRemove(context.Background(), &vlan.PortRemoveRequest{
				Vid:  uint32(vid),
				Pbmp: pbmp,
			})
			if err != nil {
				log.Fatal(err)
			}
		},
	}

	listCmd := &cobra.Command{
		Use: "list",
		Run: func(cmd *cobra.Command, args []string) {
			res, err := vlanClient.List(context.Background(), &vlan.ListRequest{})
			if err != nil {
				log.Fatal(err)
			}
			conn, err := connGrpc()
			if err != nil {
				log.Fatal(err)
			}
			portClient := portservice.NewPortClient(conn)
			for _, data := range res.List {
				ports := PBMP(data.Pbmp).Ports()
				utports := PBMP(data.UtPbmp).Ports()
				ss := make([]string, 0, len(ports))
				for _, i := range ports {
					res, err := portClient.GetPortName(context.Background(), &port.GetPortNameRequest{
						Port: int64(i),
					})
					if err != nil {
						log.Fatal(err)
					}
					untagged := false
					for _, j := range utports {
						if i == j {
							untagged = true
							break
						}
					}
					name := res.Name
					if untagged {
						name = fmt.Sprintf("%s(untagged)", name)
					}
					ss = append(ss, name)
				}
				log.Infof("VID: %d, Ports: %s", data.Vid, strings.Join(ss, ", "))
			}
		},
	}

	defaultCmd := &cobra.Command{
		Use: "default",
		Run: func(cmd *cobra.Command, args []string) {
			if len(args) == 0 {
				res, err := vlanClient.DefaultGet(context.Background(), &vlan.DefaultGetRequest{})
				if err != nil {
					log.Fatal(err)
				}
				log.Info("response:", res)
			} else {
				vid, err := strconv.Atoi(args[0])
				if err != nil {
					log.Fatal(err)
				}
				_, err = vlanClient.DefaultSet(context.Background(), &vlan.DefaultSetRequest{
					Vid: uint32(vid),
				})
				if err != nil {
					log.Fatal(err)
				}
			}
		},
	}

	vlanCmd.AddCommand(createCmd, destroyCmd, listCmd, addCmd, removeCmd, defaultCmd)
	return vlanCmd
}

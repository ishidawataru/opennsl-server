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
	"strconv"

	log "github.com/Sirupsen/logrus"
	"github.com/hydratim/opennsl-man/client/proto/link"
	"github.com/hydratim/opennsl-man/client/proto/linkservice"
	"github.com/spf13/cobra"
	"golang.org/x/net/context"
)

var linkClient linkservice.LinkClient

func NewLinkCmd() *cobra.Command {
	linkCmd := &cobra.Command{
		Use: "link",
		PersistentPreRun: func(cmd *cobra.Command, args []string) {
			conn, err := connGrpc()
			if err != nil {
				log.Fatal(err)
			}
			linkClient = linkservice.NewLinkClient(conn)
		},
	}

	detachCmd := &cobra.Command{
		Use: "detach",
		Run: func(cmd *cobra.Command, args []string) {
			_, err := linkClient.Detach(context.Background(), &link.DetachRequest{})
			if err != nil {
				log.Fatal(err)
			}
		},
	}

	scanCmd := &cobra.Command{
		Use: "scan",
		Run: func(cmd *cobra.Command, args []string) {
			if len(args) == 0 {
				res, err := linkClient.LinkscanEnableGet(context.Background(), &link.LinkscanEnableGetRequest{})
				if err != nil {
					log.Fatal(err)
				}
				log.Info("response:", res)
			} else {
				interval, err := strconv.Atoi(args[0])
				if err != nil {
					log.Fatal(err)
				}
				_, err = linkClient.LinkscanEnableSet(context.Background(), &link.LinkscanEnableSetRequest{
					Interval: int64(interval),
				})
			}
		},
	}

	modeCmd := &cobra.Command{
		Use: "mode",
		Run: func(cmd *cobra.Command, args []string) {
			switch len(args) {
			case 1:
				port, err := strconv.Atoi(args[0])
				if err != nil {
					log.Fatal(err)
				}
				res, err := linkClient.LinkscanModeGet(context.Background(), &link.LinkscanModeGetRequest{
					Port: int64(port),
				})
				if err != nil {
					log.Fatal(err)
				}
				log.Info("response:", res)
			case 2:
				port, err := strconv.Atoi(args[0])
				if err != nil {
					log.Fatal(err)
				}

				mode, err := strconv.Atoi(args[1])
				if err != nil {
					log.Fatal(err)
				}
				_, err = linkClient.LinkscanModeSet(context.Background(), &link.LinkscanModeSetRequest{
					Port: int64(port),
					Mode: link.LinkScanMode(mode),
				})
				if err != nil {
					log.Fatal(err)
				}
			default:
				log.Fatal("link scan mode <port> [<mode>]")
			}
		},
	}

	monitorCmd := &cobra.Command{
		Use: "monitor",
		Run: func(cmd *cobra.Command, args []string) {
			stream, err := linkClient.Monitor(context.Background(), &link.MonitorRequest{})
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

	scanCmd.AddCommand(modeCmd)
	linkCmd.AddCommand(detachCmd, scanCmd, monitorCmd)

	return linkCmd
}

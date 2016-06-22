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
	"github.com/ishidawataru/opennsl-server/client/proto/stat"
	"github.com/ishidawataru/opennsl-server/client/proto/statservice"
	"github.com/spf13/cobra"
	"golang.org/x/net/context"
)

var statClient statservice.StatClient

func NewStatCmd() *cobra.Command {
	statCmd := &cobra.Command{
		Use: "stat",
		PersistentPreRun: func(cmd *cobra.Command, args []string) {
			conn, err := connGrpc()
			if err != nil {
				log.Fatal(err)
			}
			statClient = statservice.NewStatClient(conn)
		},
	}

	initCmd := &cobra.Command{
		Use: "init",
		Run: func(cmd *cobra.Command, args []string) {
			_, err := statClient.Init(context.Background(), &stat.InitRequest{})
			if err != nil {
				log.Fatal(err)
			}
		},
	}

	clearCmd := &cobra.Command{
		Use: "clear",
		Run: func(cmd *cobra.Command, args []string) {
			_, err := statClient.Clear(context.Background(), &stat.ClearRequest{})
			if err != nil {
				log.Fatal(err)
			}
		},
	}

	syncCmd := &cobra.Command{
		Use: "sync",
		Run: func(cmd *cobra.Command, args []string) {
			_, err := statClient.Sync(context.Background(), &stat.SyncRequest{})
			if err != nil {
				log.Fatal(err)
			}
		},
	}

	getCmd := &cobra.Command{
		Use: "get",
		Run: func(cmd *cobra.Command, args []string) {
			if len(args) < 2 {
				log.Fatal("usage: get <port> <type>")
			}
			port, err := strconv.Atoi(args[0])
			if err != nil {
				log.Fatal(err)
			}
			typ, err := strconv.Atoi(args[1])
			if err != nil {
				log.Fatal(err)
			}
			res, err := statClient.Get(context.Background(), &stat.GetRequest{
				Port: int64(port),
				Type: stat.StatType(typ),
			})
			if err != nil {
				log.Fatal(err)
			}
			log.Info("response:", res)
		},
	}

	statCmd.AddCommand(initCmd, clearCmd, syncCmd, getCmd)
	return statCmd
}

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
	log "github.com/Sirupsen/logrus"
	//	"github.com/ishidawataru/opennsl-server/client/proto/l2"
	"github.com/ishidawataru/opennsl-server/client/proto/l2service"
	"github.com/spf13/cobra"
	"golang.org/x/net/context"
)

var l2Client l2service.L2Client

func NewL2Cmd() *cobra.Command {
	l2 := &cobra.Command{
		Use: "l2",
		PersistentPreRun: func(cmd *cobra.Command, args []string) {
			conn, err := connGrpc()
			if err != nil {
				log.Fatal(err)
			}
			l2Client = l2service.NewL2Client(conn)
		},
	}

	init := &cobra.Command{
		Use: "init",
		Run: func(cmd *cobra.Command, args []string) {
			_, err := l2Client.Init(context.Background(), &l2service.InitRequest{})
			if err != nil {
				log.Fatal(err)
			}
		},
	}

	l2.AddCommand(init)
	return l2
}

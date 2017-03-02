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
	"github.com/hydratim/opennsl-man/client/proto/driver"
	"github.com/hydratim/opennsl-man/client/proto/driverservice"
	"github.com/spf13/cobra"
	"golang.org/x/net/context"
)

var driverClient driverservice.DriverClient

func NewDriverCmd() *cobra.Command {
	driverCmd := &cobra.Command{
		Use: "driver",
		PersistentPreRun: func(cmd *cobra.Command, args []string) {
			conn, err := connGrpc()
			if err != nil {
				log.Fatal(err)
			}
			driverClient = driverservice.NewDriverClient(conn)
		},
	}

	init := &cobra.Command{
		Use: "init",
		Run: func(cmd *cobra.Command, args []string) {
			_, err := driverClient.Init(context.Background(), &driver.InitRequest{})
			if err != nil {
				log.Fatal(err)
			}
		},
	}

	version := &cobra.Command{
		Use: "version",
		Run: func(cmd *cobra.Command, args []string) {
			res, err := driverClient.GetVersion(context.Background(), &driver.GetVersionRequest{})
			if err != nil {
				log.Fatal(err)
			}
			log.Info("response:", res)
		},
	}

	driverCmd.AddCommand(init, version)
	return driverCmd
}

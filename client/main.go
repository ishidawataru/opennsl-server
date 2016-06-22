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
	"net"
	"strconv"
	"time"

	"github.com/spf13/cobra"
	"google.golang.org/grpc"
)

var opts struct {
	Host string
	Port int
}

var conn *grpc.ClientConn

func connGrpc() (*grpc.ClientConn, error) {
	timeout := grpc.WithTimeout(time.Second)
	target := net.JoinHostPort(opts.Host, strconv.Itoa(opts.Port))
	conn, err := grpc.Dial(target, timeout, grpc.WithBlock(), grpc.WithInsecure())
	if err != nil {
		return nil, err
	}
	return conn, nil
}

func NewRootCmd() *cobra.Command {
	root := &cobra.Command{
		Use: "opennsl-ctl",
	}

	root.PersistentFlags().StringVarP(&opts.Host, "host", "o", "127.0.0.1", "host")
	root.PersistentFlags().IntVarP(&opts.Port, "port", "p", 50051, "port")

	l2 := NewL2Cmd()
	driver := NewDriverCmd()
	port := NewPortCmd()
	stat := NewStatCmd()
	root.AddCommand(l2, driver, port, stat)
	return root
}

func main() {
	grpc.EnableTracing = false
	NewRootCmd().Execute()
}

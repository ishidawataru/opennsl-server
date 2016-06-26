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
	"strings"

	"bytes"
	log "github.com/Sirupsen/logrus"
	"github.com/ishidawataru/opennsl-server/client/proto/port"
	"github.com/ishidawataru/opennsl-server/client/proto/portservice"
	"github.com/spf13/cobra"
	"golang.org/x/net/context"
)

var portClient portservice.PortClient

type Ability uint64

const (
	ABILITY_10MB_HD     Ability = 1 << 0
	ABILITY_10MB_FD     Ability = 1 << 1
	ABILITY_100MB_HD    Ability = 1 << 2
	ABILITY_100MB_FD    Ability = 1 << 3
	ABILITY_1000MB_HD   Ability = 1 << 4
	ABILITY_1000MB_FD   Ability = 1 << 5
	ABILITY_2500MB_HD   Ability = 1 << 6
	ABILITY_2500MB_FD   Ability = 1 << 7
	ABILITY_10GB_HD     Ability = 1 << 8
	ABILITY_10GB_FD     Ability = 1 << 9
	ABILITY_PAUSE_TX    Ability = 1 << 10
	ABILITY_PAUSE_RX    Ability = 1 << 11
	ABILITY_PAUSE_ASYMM Ability = 1 << 12
	ABILITY_TBI         Ability = 1 << 13
	ABILITY_MII         Ability = 1 << 14
	ABILITY_GMII        Ability = 1 << 15
	ABILITY_SGMII       Ability = 1 << 16
	ABILITY_XGMII       Ability = 1 << 17
	ABILITY_LB_MAC      Ability = 1 << 18
	ABILITY_LB_NONE     Ability = 1 << 19
	ABILITY_LB_PHY      Ability = 1 << 20
	ABILITY_AN          Ability = 1 << 21
	ABILITY_3000MB_HD   Ability = 1 << 29
	ABILITY_3000MB_FD   Ability = 1 << 30
	ABILITY_12GB_HD     Ability = 1 << 22
	ABILITY_12GB_FD     Ability = 1 << 23
	ABILITY_13GB_HD     Ability = 1 << 24
	ABILITY_13GB_FD     Ability = 1 << 25
	ABILITY_16GB_HD     Ability = 1 << 26
	ABILITY_16GB_FD     Ability = 1 << 27
	ABILITY_COMBO       Ability = 1 << 31
)

func (a Ability) String() string {
	ss := make([]string, 0, 8)
	if ABILITY_10MB_HD&a > 0 {
		ss = append(ss, "10MB_HD")
	}
	if ABILITY_10MB_FD&a > 0 {
		ss = append(ss, "10MB_FD")
	}
	if ABILITY_100MB_HD&a > 0 {
		ss = append(ss, "100MB_HD")
	}
	if ABILITY_100MB_FD&a > 0 {
		ss = append(ss, "100MB_FD")
	}
	if ABILITY_1000MB_HD&a > 0 {
		ss = append(ss, "1000MB_HD")
	}
	if ABILITY_1000MB_FD&a > 0 {
		ss = append(ss, "1000MB_FD")
	}
	if ABILITY_2500MB_HD&a > 0 {
		ss = append(ss, "2500MB_HD")
	}
	if ABILITY_2500MB_FD&a > 0 {
		ss = append(ss, "2500MB_FD")
	}
	if ABILITY_10GB_HD&a > 0 {
		ss = append(ss, "10GB_HD")
	}
	if ABILITY_10GB_FD&a > 0 {
		ss = append(ss, "10GB_FD")
	}
	if ABILITY_PAUSE_TX&a > 0 {
		ss = append(ss, "PAUSE_TX")
	}
	if ABILITY_PAUSE_RX&a > 0 {
		ss = append(ss, "PAUSE_RX")
	}
	if ABILITY_PAUSE_ASYMM&a > 0 {
		ss = append(ss, "PAUSE_ASYMM")
	}
	if ABILITY_TBI&a > 0 {
		ss = append(ss, "TBI")
	}
	if ABILITY_MII&a > 0 {
		ss = append(ss, "MII")
	}
	if ABILITY_GMII&a > 0 {
		ss = append(ss, "GMII")
	}
	if ABILITY_SGMII&a > 0 {
		ss = append(ss, "SGMII")
	}
	if ABILITY_XGMII&a > 0 {
		ss = append(ss, "XGMII")
	}
	if ABILITY_LB_MAC&a > 0 {
		ss = append(ss, "LB_MAC")
	}
	if ABILITY_LB_NONE&a > 0 {
		ss = append(ss, "LB_NONE")
	}
	if ABILITY_LB_PHY&a > 0 {
		ss = append(ss, "LB_PHY")
	}
	if ABILITY_AN&a > 0 {
		ss = append(ss, "AN")
	}
	if ABILITY_3000MB_HD&a > 0 {
		ss = append(ss, "3000MB_HD")
	}
	if ABILITY_3000MB_FD&a > 0 {
		ss = append(ss, "3000MB_FD")
	}
	return strings.Join(ss, "|")
}

func FormatPortConfig(c *port.PortConfig) string {
	s := bytes.NewBuffer(make([]byte, 0, 64))
	s.WriteString("      ")
	for i := 0; i < PBMP_WORD_MAX; i++ {
		s.WriteString("v               v               ")
	}
	s.WriteString("\nFe:   ")
	s.WriteString(PBMP(c.Fe).String())
	s.WriteString("\nGe:   ")
	s.WriteString(PBMP(c.Ge).String())
	s.WriteString("\nXe:   ")
	s.WriteString(PBMP(c.Xe).String())
	s.WriteString("\nCe:   ")
	s.WriteString(PBMP(c.Ce).String())
	s.WriteString("\nE:    ")
	s.WriteString(PBMP(c.E).String())
	s.WriteString("\nHg:   ")
	s.WriteString(PBMP(c.Hg).String())
	s.WriteString("\nPort: ")
	s.WriteString(PBMP(c.Port).String())
	s.WriteString("\nCPU:  ")
	s.WriteString(PBMP(c.Cpu).String())
	s.WriteString("\nAll:  ")
	s.WriteString(PBMP(c.All).String())
	return s.String()
}

func NewPortCmd() *cobra.Command {
	portCmd := &cobra.Command{
		Use: "port",
		PersistentPreRun: func(cmd *cobra.Command, args []string) {
			conn, err := connGrpc()
			if err != nil {
				log.Fatal(err)
			}
			portClient = portservice.NewPortClient(conn)
		},
	}

	initCmd := &cobra.Command{
		Use: "init",
		Run: func(cmd *cobra.Command, args []string) {
			_, err := portClient.Init(context.Background(), &port.InitRequest{})
			if err != nil {
				log.Fatal(err)
			}
		},
	}

	clearCmd := &cobra.Command{
		Use: "clear",
		Run: func(cmd *cobra.Command, args []string) {
			_, err := portClient.Clear(context.Background(), &port.ClearRequest{})
			if err != nil {
				log.Fatal(err)
			}
		},
	}

	// probe
	// detach

	getConfig := &cobra.Command{
		Use: "config",
		Run: func(cmd *cobra.Command, args []string) {
			res, err := portClient.GetConfig(context.Background(), &port.GetConfigRequest{})
			if err != nil {
				log.Fatal(err)
			}
			log.Info("reponse:\n", FormatPortConfig(res.Config))
		},
	}

	getPortName := &cobra.Command{
		Use: "name",
		Run: func(cmd *cobra.Command, args []string) {
			if len(args) < 1 {
				log.Fatal("usage: name <port>")
			}
			i, err := strconv.Atoi(args[0])
			if err != nil {
				log.Fatal(err)
			}
			res, err := portClient.GetPortName(context.Background(), &port.GetPortNameRequest{
				Port: int64(i),
			})
			if err != nil {
				log.Fatal(err)
			}
			log.Info("response:", res)
		},
	}

	enable := &cobra.Command{
		Use: "enable",
		Run: func(cmd *cobra.Command, args []string) {
			if len(args) < 1 {
				log.Fatal("usage: port enable <port>")
			}
			i, err := strconv.Atoi(args[0])
			if err != nil {
				log.Fatal(err)
			}
			_, err = portClient.PortEnableSet(context.Background(), &port.PortEnableSetRequest{
				Port:   int64(i),
				Enable: true,
			})
			if err != nil {
				log.Fatal(err)
			}
		},
	}

	disable := &cobra.Command{
		Use: "disable",
		Run: func(cmd *cobra.Command, args []string) {
			if len(args) < 1 {
				log.Fatal("usage: port disable <port>")
			}
			i, err := strconv.Atoi(args[0])
			if err != nil {
				log.Fatal(err)
			}
			_, err = portClient.PortEnableSet(context.Background(), &port.PortEnableSetRequest{
				Port:   int64(i),
				Enable: false,
			})
			if err != nil {
				log.Fatal(err)
			}
		},
	}

	enabled := &cobra.Command{
		Use: "enabled",
		Run: func(cmd *cobra.Command, args []string) {
			if len(args) < 1 {
				log.Fatal("usage: port enabled <port>")
			}
			i, err := strconv.Atoi(args[0])
			if err != nil {
				log.Fatal(err)
			}
			res, err := portClient.PortEnableGet(context.Background(), &port.PortEnableGetRequest{
				Port: int64(i),
			})
			if err != nil {
				log.Fatal(err)
			}
			log.Info(res)
		},
	}

	advert := &cobra.Command{
		Use: "advert",
		Run: func(cmd *cobra.Command, args []string) {
			if len(args) < 1 {
				log.Fatal("usage: port advert <port>")
			}
			i, err := strconv.Atoi(args[0])
			if err != nil {
				log.Fatal(err)
			}
			res, err := portClient.PortAdvertGet(context.Background(), &port.PortAdvertGetRequest{
				Port: int64(i),
			})
			if err != nil {
				log.Fatal(err)
			}
			log.Info("advert: ", Ability(res.Ability).String())
		},
	}

	advertRemote := &cobra.Command{
		Use: "remote",
		Run: func(cmd *cobra.Command, args []string) {
			if len(args) < 1 {
				log.Fatal("usage: port advert remote <port>")
			}
			i, err := strconv.Atoi(args[0])
			if err != nil {
				log.Fatal(err)
			}
			res, err := portClient.PortAdvertRemoteGet(context.Background(), &port.PortAdvertRemoteGetRequest{
				Port: int64(i),
			})
			if err != nil {
				log.Fatal(err)
			}
			log.Info("advert remote: ", Ability(res.Ability).String())
		},
	}

	advert.AddCommand(advertRemote)

	ability := &cobra.Command{
		Use: "ability",
		Run: func(cmd *cobra.Command, args []string) {
			if len(args) < 1 {
				log.Fatal("usage: port ability <port>")
			}
			i, err := strconv.Atoi(args[0])
			if err != nil {
				log.Fatal(err)
			}
			res, err := portClient.PortAbilityGet(context.Background(), &port.PortAbilityGetRequest{
				Port: int64(i),
			})
			if err != nil {
				log.Fatal(err)
			}
			log.Info("ability: ", res)
		},
	}

	abilityAdvert := &cobra.Command{
		Use: "advert",
		Run: func(cmd *cobra.Command, args []string) {
			if len(args) < 1 {
				log.Fatal("usage: port ability advert <port>")
			}
			i, err := strconv.Atoi(args[0])
			if err != nil {
				log.Fatal(err)
			}
			res, err := portClient.PortAbilityAdvertGet(context.Background(), &port.PortAbilityAdvertGetRequest{
				Port: int64(i),
			})
			if err != nil {
				log.Fatal(err)
			}
			log.Info("ability advert: ", res)
		},
	}

	abilityRemote := &cobra.Command{
		Use: "remote",
		Run: func(cmd *cobra.Command, args []string) {
			if len(args) < 1 {
				log.Fatal("usage: port ability remote <port>")
			}
			i, err := strconv.Atoi(args[0])
			if err != nil {
				log.Fatal(err)
			}
			res, err := portClient.PortAbilityRemoteGet(context.Background(), &port.PortAbilityRemoteGetRequest{
				Port: int64(i),
			})
			if err != nil {
				log.Fatal(err)
			}
			log.Info("ability remote: ", res)
		},
	}

	abilityLocal := &cobra.Command{
		Use: "local",
		Run: func(cmd *cobra.Command, args []string) {
			if len(args) < 1 {
				log.Fatal("usage: port ability local <port>")
			}
			i, err := strconv.Atoi(args[0])
			if err != nil {
				log.Fatal(err)
			}
			res, err := portClient.PortAbilityLocalGet(context.Background(), &port.PortAbilityLocalGetRequest{
				Port: int64(i),
			})
			if err != nil {
				log.Fatal(err)
			}
			log.Info("ability local: ", res)
		},
	}

	ability.AddCommand(abilityAdvert, abilityRemote, abilityLocal)
	portCmd.AddCommand(initCmd, clearCmd, getConfig, getPortName, enable, disable, enabled, advert, ability)
	return portCmd
}

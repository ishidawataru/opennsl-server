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

type Speed uint64

const (
	SPEED_10MB     Speed = 1 << 0
	SPEED_EXTENDED Speed = 1 << 1
	SPEED_27GB     Speed = 1 << 2
	SPEED_50GB     Speed = 1 << 3
	SPEED_53GB     Speed = 1 << 4
	SPEED_100MB    Speed = 1 << 5
	SPEED_1000MB   Speed = 1 << 6
	SPEED_2500MB   Speed = 1 << 7
	SPEED_3000MB   Speed = 1 << 8
	SPEED_5000MB   Speed = 1 << 9
	SPEED_6000MB   Speed = 1 << 10
	SPEED_10GB     Speed = 1 << 11
	SPEED_11GB     Speed = 1 << 12
	SPEED_12GB     Speed = 1 << 13
	SPEED_12P5GB   Speed = 1 << 14
	SPEED_13GB     Speed = 1 << 15
	SPEED_15GB     Speed = 1 << 16
	SPEED_16GB     Speed = 1 << 17
	SPEED_20GB     Speed = 1 << 18
	SPEED_21GB     Speed = 1 << 19
	SPEED_23GB     Speed = 1 << 20
	SPEED_24GB     Speed = 1 << 21
	SPEED_25GB     Speed = 1 << 22
	SPEED_30GB     Speed = 1 << 23
	SPEED_40GB     Speed = 1 << 24
	SPEED_42GB     Speed = 1 << 25
	SPEED_100GB    Speed = 1 << 26
	SPEED_120GB    Speed = 1 << 27
	SPEED_127GB    Speed = 1 << 28
	SPEED_106GB    Speed = 1 << 29
	SPEED_48GB     Speed = 1 << 30
	SPEED_32GB     Speed = 1 << 31
)

func (a Speed) String() string {
	ss := make([]string, 0, 8)
	if SPEED_10MB&a > 0 {
		ss = append(ss, "10MB")
	}
	if SPEED_EXTENDED&a > 0 {
		ss = append(ss, "EXTENDED")
	}
	if SPEED_27GB&a > 0 {
		ss = append(ss, "27GB")
	}
	if SPEED_50GB&a > 0 {
		ss = append(ss, "50GB")
	}
	if SPEED_53GB&a > 0 {
		ss = append(ss, "53GB")
	}
	if SPEED_100MB&a > 0 {
		ss = append(ss, "100MB")
	}
	if SPEED_1000MB&a > 0 {
		ss = append(ss, "1000MB")
	}
	if SPEED_2500MB&a > 0 {
		ss = append(ss, "2500MB")
	}
	if SPEED_3000MB&a > 0 {
		ss = append(ss, "3000MB")
	}
	if SPEED_5000MB&a > 0 {
		ss = append(ss, "5000MB")
	}
	if SPEED_6000MB&a > 0 {
		ss = append(ss, "6000MB")
	}
	if SPEED_10GB&a > 0 {
		ss = append(ss, "10GB")
	}
	if SPEED_11GB&a > 0 {
		ss = append(ss, "11GB")
	}
	if SPEED_12GB&a > 0 {
		ss = append(ss, "12GB")
	}
	if SPEED_12P5GB&a > 0 {
		ss = append(ss, "12P5GB")
	}
	if SPEED_13GB&a > 0 {
		ss = append(ss, "13GB")
	}
	if SPEED_15GB&a > 0 {
		ss = append(ss, "15GB")
	}
	if SPEED_16GB&a > 0 {
		ss = append(ss, "16GB")
	}
	if SPEED_20GB&a > 0 {
		ss = append(ss, "20GB")
	}
	if SPEED_21GB&a > 0 {
		ss = append(ss, "21GB")
	}
	if SPEED_23GB&a > 0 {
		ss = append(ss, "23GB")
	}
	if SPEED_24GB&a > 0 {
		ss = append(ss, "24GB")
	}
	if SPEED_25GB&a > 0 {
		ss = append(ss, "25GB")
	}
	if SPEED_30GB&a > 0 {
		ss = append(ss, "30GB")
	}
	if SPEED_40GB&a > 0 {
		ss = append(ss, "40GB")
	}
	if SPEED_42GB&a > 0 {
		ss = append(ss, "42GB")
	}
	if SPEED_100GB&a > 0 {
		ss = append(ss, "100GB")
	}
	if SPEED_120GB&a > 0 {
		ss = append(ss, "120GB")
	}
	if SPEED_127GB&a > 0 {
		ss = append(ss, "127GB")
	}
	if SPEED_106GB&a > 0 {
		ss = append(ss, "106GB")
	}
	if SPEED_48GB&a > 0 {
		ss = append(ss, "48GB")
	}
	if SPEED_32GB&a > 0 {
		ss = append(ss, "32GB")
	}
	return strings.Join(ss, "|")
}

type Pause uint64

const (
	PAUSE_TX    Pause = 1 << 0
	PAUSE_RX    Pause = 1 << 1
	PAUSE_ASYMM Pause = 1 << 2
)

func (p Pause) String() string {
	ss := make([]string, 0, 3)
	if PAUSE_TX&p > 0 {
		ss = append(ss, "TX")
	}
	if PAUSE_RX&p > 0 {
		ss = append(ss, "RX")
	}
	if PAUSE_ASYMM&p > 0 {
		ss = append(ss, "ASYMM")
	}
	return strings.Join(ss, "|")
}

type InterfaceType uint64

const (
	INTF_TBI    InterfaceType = 1 << 0 /* TBI mode supported */
	INTF_MII    InterfaceType = 1 << 1 /* MII mode supported */
	INTF_GMII   InterfaceType = 1 << 2 /* GMII mode supported */
	INTF_RGMII  InterfaceType = 1 << 3 /* RGMII mode supported */
	INTF_SGMII  InterfaceType = 1 << 4 /* SGMII mode supported */
	INTF_XGMII  InterfaceType = 1 << 5 /* XGMII mode supported */
	INTF_QSGMII InterfaceType = 1 << 6 /* QSGMII mode supported */
	INTF_CGMII  InterfaceType = 1 << 7 /* CGMII mode supported */
)

func (i InterfaceType) String() string {
	ss := make([]string, 0, 2)
	if INTF_TBI&i > 0 {
		ss = append(ss, "TBI")
	}
	if INTF_MII&i > 0 {
		ss = append(ss, "MII")
	}
	if INTF_GMII&i > 0 {
		ss = append(ss, "GMII")
	}
	if INTF_RGMII&i > 0 {
		ss = append(ss, "RGMII")
	}
	if INTF_SGMII&i > 0 {
		ss = append(ss, "SGMII")
	}
	if INTF_XGMII&i > 0 {
		ss = append(ss, "XGMII")
	}
	if INTF_QSGMII&i > 0 {
		ss = append(ss, "QSGMII")
	}
	if INTF_CGMII&i > 0 {
		ss = append(ss, "CGMII")
	}
	return strings.Join(ss, "|")
}

type Medium uint64

const (
	MEDIUM_COPPER Medium = 1 << 0
	MEDIUM_FIBER  Medium = 1 << 1
)

func (m Medium) String() string {
	ss := make([]string, 0, 1)
	if MEDIUM_COPPER&m > 0 {
		ss = append(ss, "COPPER")
	}
	if MEDIUM_FIBER&m > 0 {
		ss = append(ss, "FIBER")
	}
	return strings.Join(ss, "|")
}

type Loopback uint64

const (
	LOOPBACK_NONE = 1 << 0
	LOOPBACK_MAC  = 1 << 1
	LOOPBACK_PHY  = 1 << 2
	LOOPBACK_LINE = 1 << 3
)

func (l Loopback) String() string {
	ss := make([]string, 0, 2)
	if LOOPBACK_NONE&l > 0 {
		ss = append(ss, "NONE")
	}
	if LOOPBACK_MAC&l > 0 {
		ss = append(ss, "MAC")
	}
	if LOOPBACK_PHY&l > 0 {
		ss = append(ss, "PHY")
	}
	if LOOPBACK_LINE&l > 0 {
		ss = append(ss, "LINE")
	}
	return strings.Join(ss, "|")
}

type Flag uint64

const (
	AUTONEG Flag = 1 << 0
	COMBO   Flag = 1 << 1
)

func (f Flag) String() string {
	ss := make([]string, 0, 2)
	if AUTONEG&f > 0 {
		ss = append(ss, "AUTONEG")
	}
	if COMBO&f > 0 {
		ss = append(ss, "COMBO")
	}
	return strings.Join(ss, "|")
}

type FECType uint64

const (
	FEC         FECType = 1 << 0
	FEC_REQUEST FECType = 1 << 1
)

func (f FECType) String() string {
	ss := make([]string, 0, 2)
	if FEC&f > 0 {
		ss = append(ss, "FEC")
	}
	if FEC_REQUEST&f > 0 {
		ss = append(ss, "REQUEST")
	}
	return strings.Join(ss, "|")
}

func FormatPortAbility(c *port.Ability) string {
	s := bytes.NewBuffer(make([]byte, 0, 64))
	if c.SpeedHalfDuplex > 0 {
		s.WriteString("speed half duplex: ")
		s.WriteString(Speed(c.SpeedHalfDuplex).String())
		s.WriteString("\n")
	}
	if c.SpeedFullDuplex > 0 {
		s.WriteString("speed full duplex: ")
		s.WriteString(Speed(c.SpeedFullDuplex).String())
		s.WriteString("\n")
	}
	if c.Pause > 0 {
		s.WriteString("pause: ")
		s.WriteString(Pause(c.Pause).String())
		s.WriteString("\n")
	}
	if c.Interface > 0 {
		s.WriteString("interface: ")
		s.WriteString(InterfaceType(c.Interface).String())
		s.WriteString("\n")
	}
	if c.Medium > 0 {
		s.WriteString("medium: ")
		s.WriteString(Medium(c.Medium).String())
		s.WriteString("\n")
	}
	if c.Loopback > 0 {
		s.WriteString("loopback: ")
		s.WriteString(Loopback(c.Loopback).String())
		s.WriteString("\n")
	}
	if c.Flags > 0 {
		s.WriteString("flags: ")
		s.WriteString(Flag(c.Flags).String())
		s.WriteString("\n")
	}
	if c.Eee > 0 {
		s.WriteString("eee: ")
		s.WriteString(Ability(c.Eee).String())
		s.WriteString("\n")
	}
	if c.Fcmap > 0 {
		s.WriteString("fcmap: ")
		s.WriteString(Ability(c.Fcmap).String())
		s.WriteString("\n")
	}
	if c.Fec > 0 {
		s.WriteString("fec: ")
		s.WriteString(FECType(c.Fec).String())
		s.WriteString("\n")
	}
	return s.String()
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
			log.Info("ability: ", Ability(res.Ability))
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
			log.Info("ability advert: ", FormatPortAbility(res.Ability))
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
			log.Info("ability remote: ", FormatPortAbility(res.Ability))
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
			log.Info("ability local: ", FormatPortAbility(res.Ability))
		},
	}

	linkscan := &cobra.Command{
		Use: "linkscan",
		Run: func(cmd *cobra.Command, args []string) {
			if len(args) < 1 {
				log.Fatal("usage: port linkscan <port> [<interval>]")
			}
			i, err := strconv.Atoi(args[0])
			if err != nil {
				log.Fatal(err)
			}
			if len(args) == 1 {
				res, err := portClient.PortLinkscanGet(context.Background(), &port.PortLinkscanGetRequest{
					Port: int64(i),
				})
				if err != nil {
					log.Fatal(err)
				}
				log.Info("res: ", res)
			} else {
				linkscan, err := strconv.Atoi(args[1])
				if err != nil {
					log.Fatal(err)
				}
				_, err = portClient.PortLinkscanSet(context.Background(), &port.PortLinkscanSetRequest{
					Port:     int64(i),
					Linkscan: int64(linkscan),
				})
				if err != nil {
					log.Fatal(err)
				}
			}
		},
	}

	autoneg := &cobra.Command{
		Use: "autoneg",
		Run: func(cmd *cobra.Command, args []string) {
			if len(args) < 1 {
				log.Fatal("usage: port autoneg <port> [ enable | disable ]")
			}
			i, err := strconv.Atoi(args[0])
			if err != nil {
				log.Fatal(err)
			}
			if len(args) == 1 {
				res, err := portClient.PortAutonegGet(context.Background(), &port.PortAutonegGetRequest{
					Port: int64(i),
				})
				if err != nil {
					log.Fatal(err)
				}
				log.Info("res: ", res)
			} else if args[1] == "enable" || args[1] == "disable" {
				enable := true
				if args[1] == "disable" {
					enable = false
				}
				_, err = portClient.PortAutonegSet(context.Background(), &port.PortAutonegSetRequest{
					Port:   int64(i),
					Enable: enable,
				})
				if err != nil {
					log.Fatal(err)
				}
			} else {
				log.Fatal("usage: port autoneg <port> [ enable | disable ]")
			}
		},
	}

	speed := &cobra.Command{
		Use: "speed",
		Run: func(cmd *cobra.Command, args []string) {
			if len(args) < 1 {
				log.Fatal("usage: port speed <port> [ max | <speed> ]")
			}
			i, err := strconv.Atoi(args[0])
			if err != nil {
				log.Fatal(err)
			}
			if len(args) == 1 {
				res, err := portClient.PortSpeedGet(context.Background(), &port.PortSpeedGetRequest{
					Port: int64(i),
				})
				if err != nil {
					log.Fatal(err)
				}
				log.Info("res: ", res)
			} else if args[1] == "max" {
				res, err := portClient.PortSpeedMAX(context.Background(), &port.PortSpeedMAXRequest{
					Port: int64(i),
				})
				if err != nil {
					log.Fatal(err)
				}
				log.Info("res: ", res)
			} else {
				speed, err := strconv.Atoi(args[1])
				if err != nil {
					log.Fatal(err)
				}
				_, err = portClient.PortSpeedSet(context.Background(), &port.PortSpeedSetRequest{
					Port:  int64(i),
					Speed: int64(speed),
				})
				if err != nil {
					log.Fatal(err)
				}
			}
		},
	}

	intf := &cobra.Command{
		Use: "interface",
		Run: func(cmd *cobra.Command, args []string) {
			if len(args) < 1 {
				log.Fatal("usage: port interface <port> [<type>]")
			}
			i, err := strconv.Atoi(args[0])
			if err != nil {
				log.Fatal(err)
			}
			if len(args) == 1 {
				res, err := portClient.PortInterfaceGet(context.Background(), &port.PortInterfaceGetRequest{
					Port: int64(i),
				})
				if err != nil {
					log.Fatal(err)
				}
				log.Info("res: ", res)
			} else {
				typ, err := strconv.Atoi(args[1])
				if err != nil {
					log.Fatal(err)
				}
				_, err = portClient.PortInterfaceSet(context.Background(), &port.PortInterfaceSetRequest{
					Port: int64(i),
					Type: port.InterfaceType(typ),
				})
				if err != nil {
					log.Fatal(err)
				}
			}

		},
	}

	status := &cobra.Command{
		Use: "status",
		Run: func(cmd *cobra.Command, args []string) {
			if len(args) < 1 {
				log.Fatal("usage: port status <port>")
			}
			i, err := strconv.Atoi(args[0])
			if err != nil {
				log.Fatal(err)
			}
			res, err := portClient.PortLinkStatusGet(context.Background(), &port.PortLinkStatusGetRequest{
				Port: int64(i),
			})
			if err != nil {
				log.Fatal(err)
			}
			log.Info(res)
		},
	}

	failedClear := &cobra.Command{
		Use: "failed-clear",
		Run: func(cmd *cobra.Command, args []string) {
			if len(args) < 1 {
				log.Fatal("usage: port failed-clear <port>")
			}
			i, err := strconv.Atoi(args[0])
			if err != nil {
				log.Fatal(err)
			}
			_, err = portClient.PortLinkFailedClear(context.Background(), &port.PortLinkFailedClearRequest{
				Port: int64(i),
			})
			if err != nil {
				log.Fatal(err)
			}
		},
	}

	control := &cobra.Command{
		Use: "control",
		Run: func(cmd *cobra.Command, args []string) {
			if len(args) < 1 {
				log.Fatal("usage: port control <port> [<type> <value>]")
			}
			i, err := strconv.Atoi(args[0])
			if err != nil {
				log.Fatal(err)
			}
			if len(args) == 0 {
				for k, v := range port.ControlType_name {
					if k == 0 {
						continue
					}
					res, err := portClient.PortControlGet(context.Background(), &port.PortControlGetRequest{
						Port: int64(i),
						Type: port.ControlType(k),
					})
					if err != nil {
						log.Fatal(err)
					}
					log.Info(fmt.Sprintf("%s(%d): %d", v, k, res.Value))
				}
			} else if len(args) == 3 {
				typ, err := strconv.Atoi(args[1])
				if err != nil {
					log.Fatal(err)
				}
				value, err := strconv.Atoi(args[2])
				if err != nil {
					log.Fatal(err)
				}
				_, err = portClient.PortControlSet(context.Background(), &port.PortControlSetRequest{
					Port:  int64(i),
					Type:  port.ControlType(typ),
					Value: int64(value),
				})
				if err != nil {
					log.Fatal(err)
				}
			} else {
				log.Fatal("usage: port control <port> [<type> <value>]")
			}
		},
	}

	gport := &cobra.Command{
		Use: "gport",
		Run: func(cmd *cobra.Command, args []string) {
			if len(args) < 1 {
				log.Fatal("usage: port gport <port>")
			}
			i, err := strconv.Atoi(args[0])
			if err != nil {
				log.Fatal(err)
			}
			res, err := portClient.PortGportGet(context.Background(), &port.PortGportGetRequest{
				Port: int64(i),
			})
			log.Info("response:", res)
		},
	}

	local := &cobra.Command{
		Use: "local",
		Run: func(cmd *cobra.Command, args []string) {
			if len(args) < 1 {
				log.Fatal("usage: port gport local <gport>")
			}
			i, err := strconv.Atoi(args[0])
			if err != nil {
				log.Fatal(err)
			}
			res, err := portClient.PortLocalGet(context.Background(), &port.PortLocalGetRequest{
				Gport: int64(i),
			})
			log.Info("response:", res)
		},
	}

	ability.AddCommand(abilityAdvert, abilityRemote, abilityLocal)
	gport.AddCommand(local)
	portCmd.AddCommand(initCmd, clearCmd, getConfig, getPortName, enable,
		disable, enabled, advert, ability, linkscan, autoneg, speed,
		intf, status, failedClear, control, gport)
	return portCmd
}

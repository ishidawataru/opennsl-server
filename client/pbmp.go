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
	"regexp"
	"strconv"
	"strings"
)

const (
	PBMP_PORT_MAX   = 256
	PBMP_WORD_WIDTH = 32
)

var PBMP_WIDTH = (((PBMP_PORT_MAX + 32 - 1) / 32) * 32)
var PBMP_WORD_MAX = ((PBMP_WIDTH + PBMP_WORD_WIDTH - 1) / PBMP_WORD_WIDTH)

type PBMP []uint32

func NewPBMP() PBMP {
	return PBMP(make([]uint32, PBMP_WORD_MAX))
}

func (p PBMP) wordEntry(port int) uint32 {
	return uint32(port / PBMP_WORD_WIDTH)
}

func (p PBMP) wordBit(port int) uint32 {
	return 1 << uint(port%PBMP_WORD_WIDTH)
}

func (p PBMP) IsMember(port int) bool {
	return (p[p.wordEntry(port)] & p.wordBit(port)) > 0
}

func (p PBMP) Clear() {
	p = make([]uint32, PBMP_WORD_MAX)
}

func (p PBMP) AddPort(port int) {
	p[p.wordEntry(port)] |= p.wordBit(port)
}

func (p PBMP) SetPort(port int) {
	p.Clear()
	p.AddPort(port)
}

func (p PBMP) RemovePort(port int) {
	p[p.wordEntry(port)] &= ^p.wordBit(port)
}

func (p PBMP) FilpPort(port int) {
	p[p.wordEntry(port)] ^= p.wordBit(port)
}

func reverse(s string) string {
	runes := []rune(s)
	for i, j := 0, len(runes)-1; i < j; i, j = i+1, j-1 {
		runes[i], runes[j] = runes[j], runes[i]
	}
	return string(runes)
}

func (p PBMP) String() string {
	ss := make([]string, 0, len(p))
	for _, b := range p {
		ss = append(ss, reverse(fmt.Sprintf("%032b", b)))
	}
	return strings.Join(ss, "")
}

func (p PBMP) Ports() []int {
	ports := make([]int, 0, PBMP_PORT_MAX/2)
	for i := 0; i < PBMP_PORT_MAX; i++ {
		if p.IsMember(i) {
			ports = append(ports, i)
		}
	}
	return ports
}

func ParsePBMP(str string) (PBMP, error) {
	exp := regexp.MustCompile("^\\d+")
	pbmp := NewPBMP()
	dash := false
	fst := 0
	for len(str) > 0 {
		switch {
		case str[0] == ',':
			str = str[1:]
		case str[0] == '-':
			dash = true
			str = str[1:]
		default:
			r := exp.FindStringIndex(str)
			if len(r) == 0 {
				return nil, fmt.Errorf("invalid format")
			}
			n, _ := strconv.Atoi(str[r[0]:r[1]])
			if dash {
				if fst >= n {
					return nil, fmt.Errorf("invalid format")
				}
				for i := fst + 1; i <= n; i++ {
					pbmp.AddPort(i)
				}
				dash = false
			} else {
				pbmp.AddPort(n)
				fst = n
			}
			str = str[r[1]:]
		}
	}
	return pbmp, nil
}

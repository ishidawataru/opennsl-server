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
	"strings"
)

const (
	PBMP_PORT_MAX   = 256
	PBMP_WORD_WIDTH = 32
)

var PBMP_WIDTH = (((PBMP_PORT_MAX + 32 - 1) / 32) * 32)
var PBMP_WORD_MAX = ((PBMP_WIDTH + PBMP_WORD_WIDTH - 1) / PBMP_WORD_WIDTH)

type PBMP []uint32

func (p PBMP) wordEntry(port int) uint32 {
	return uint32(port / PBMP_WORD_WIDTH)
}

func (p PBMP) wordBit(port int) uint32 {
	return 1 << uint(port%PBMP_WORD_WIDTH)
}

func (p PBMP) IsMember(port int) bool {
	return (p.wordEntry(port) & p.wordBit(port)) > 0
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

func (p PBMP) String() string {
	ss := make([]string, 0, len(p))
	for _, b := range p {
		ss = append(ss, fmt.Sprintf("%b", b))
	}
	return strings.Join(ss, "")
}

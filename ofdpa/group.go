package main

import (
	//#cgo CFLAGS: -I../../usr/include/ofdpa
	//#cgo LDFLAGS: -lofdpa_rpc_client -L ./
	//#include "ofdpa_api.h"
	//#include <stdlib.h>
	"C"

	"bytes"
	"encoding/binary"
	"net"
)

type groupType uint32

const (
	GROUP_TYPE_L2_INTERFACE groupType = iota
	GROUP_TYPE_L2_REWRITE
	GROUP_TYPE_L3_UNICAST
	GROUP_TYPE_L2_MULTICAST
	GROUP_TYPE_L2_FLOOD
	GROUP_TYPE_L3_INTERFACE
	GROUP_TYPE_L3_MULTICAST
	GROUP_TYPE_L3_ECMP
	GROUP_TYPE_L2_OVERLAY
	GROUP_TYPE_MPLS_LABEL
	GROUP_TYPE_MPLS_FORWARDING
	GROUP_TYPE_L2_UNFILTERED_INTERFACE
)

type mplsGroupSubtype uint32

const (
	MPLS_INTERFACE mplsGroupSubtype = iota
	MPLS_L2_VPN_LABEL
	MPLS_L3_VPN_LABEL
	MPLS_TUNNEL_LABEL1
	MPLS_TUNNEL_LABEL2
	MPLS_SWAP_LABEL
)

type GroupID uint32

func NewGroupID(gtype groupType, vid, port, index int, mplsSubtype mplsGroupSubtype) (GroupID, error) {
	var id C.uint32_t
	err := NewOFDPAError(C.ofdpaGroupTypeSet(&id, C.uint32_t(gtype)))
	if err != nil {
		return 0, err
	}
	if vid > 0 {
		err := NewOFDPAError(C.ofdpaGroupVlanSet(&id, C.uint32_t(vid)))
		if err != nil {
			return 0, err
		}
	}
	if port > 0 {
		err := NewOFDPAError(C.ofdpaGroupPortIdSet(&id, C.uint32_t(port)))
		if err != nil {
			return 0, err
		}
	}
	if index > 0 {
		err := NewOFDPAError(C.ofdpaGroupIndexSet(&id, C.uint32_t(index)))
		if err != nil {
			return 0, err
		}
	}
	if gtype == GROUP_TYPE_MPLS_LABEL || gtype == GROUP_TYPE_MPLS_FORWARDING {
		err := NewOFDPAError(C.ofdpaGroupMplsSubTypeSet(&id, C.uint32_t(mplsSubtype)))
		if err != nil {
			return 0, err
		}
	}
	return GroupID(id), nil
}

func newOFDPAMacAddr(mac string) (C.ofdpaMacAddr_t, error) {
	var a C.ofdpaMacAddr_t
	m, err := net.ParseMAC(mac)
	if err != nil {
		return a, err
	}
	for idx, b := range m {
		a.addr[idx] = C.uint8_t(b)
	}
	return a, nil
}

func AddL2InterfaceGroup(vid, port int, pop bool) (GroupID, error) {
	var group C.ofdpaGroupEntry_t
	gid, err := NewGroupID(GROUP_TYPE_L2_INTERFACE, vid, port, 0, 0)
	if err != nil {
		return 0, err
	}
	group.groupId = C.uint32_t(gid)
	rc := C.ofdpaGroupAdd(&group)
	err = NewOFDPAError(rc)
	if err != nil {
		return 0, err
	}

	var bucket C.ofdpaGroupBucketEntry_t
	bucket.groupId = C.uint32_t(gid)
	var l2 C.ofdpaL2InterfaceGroupBucketData_t
	l2.outputPort = C.uint32_t(port)
	if pop {
		l2.popVlanTag = C.uint32_t(1)
	}
	var buf bytes.Buffer
	binary.Write(&buf, binary.LittleEndian, l2)
	var buff [84]byte
	for idx, b := range buf.Bytes() {
		buff[idx] = b
	}
	bucket.bucketData = buff
	return gid, NewOFDPAError(C.ofdpaGroupBucketEntryAdd(&bucket))
}

func AddL2FloodGroup(vid int, ports []int, index int) (GroupID, error) {
	var group C.ofdpaGroupEntry_t
	gid, err := NewGroupID(GROUP_TYPE_L2_FLOOD, vid, 0, index, 0)
	if err != nil {
		return 0, err
	}
	group.groupId = C.uint32_t(gid)
	err = NewOFDPAError(C.ofdpaGroupAdd(&group))
	if err != nil {
		return 0, err
	}
	for idx, port := range ports {
		var bucket C.ofdpaGroupBucketEntry_t
		bucket.groupId = C.uint32_t(gid)
		refGID, _ := NewGroupID(GROUP_TYPE_L2_INTERFACE, vid, port, 0, 0)
		bucket.referenceGroupId = C.uint32_t(refGID)
		bucket.bucketIndex = C.uint32_t(idx)
		err = NewOFDPAError(C.ofdpaGroupBucketEntryAdd(&bucket))
		if err != nil {
			return 0, err
		}
	}
	return gid, nil
}

func AddL3UnicastGroup(vid int, src, dst string, refGID GroupID, index int) (GroupID, error) {
	var group C.ofdpaGroupEntry_t
	gid, err := NewGroupID(GROUP_TYPE_L3_UNICAST, 0, 0, index, 0)
	if err != nil {
		return gid, err
	}
	group.groupId = C.uint32_t(gid)
	err = NewOFDPAError(C.ofdpaGroupAdd(&group))
	if err != nil {
		return gid, err
	}
	var bucket C.ofdpaGroupBucketEntry_t
	bucket.groupId = C.uint32_t(gid)
	bucket.referenceGroupId = C.uint32_t(refGID)
	var l3 C.ofdpaL3UnicastGroupBucketData_t
	s, err := newOFDPAMacAddr(src)
	if err != nil {
		return gid, err
	}
	l3.srcMac = s
	d, err := newOFDPAMacAddr(dst)
	if err != nil {
		return gid, err
	}
	l3.dstMac = d
	l3.vlanId = C.OFDPA_VID_PRESENT | C.uint32_t(vid)

	var buf bytes.Buffer
	binary.Write(&buf, binary.LittleEndian, l3)
	var buff [84]byte
	for idx, b := range buf.Bytes() {
		buff[idx] = b
	}
	bucket.bucketData = buff

	err = NewOFDPAError(C.ofdpaGroupBucketEntryAdd(&bucket))
	if err != nil {
		return gid, err
	}
	return gid, err
}

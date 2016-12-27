package main

import (
	//#cgo CFLAGS: -I../../usr/include/ofdpa
	//#cgo LDFLAGS: -lofdpa_rpc_client -L ./
	//#include "ofdpa_api.h"
	//#include <stdlib.h>
	"C"

	"bytes"
	"encoding/binary"
	"fmt"
	"net"
)

type TableID uint32

const (
	TABLE_ID_INGRESS_PORT    TableID = 0
	TABLE_ID_PORT_DSCP_TRUST TableID = 5
	TABLE_ID_PORT_PCP_TRUST  TableID = 6
	TABLE_ID_VLAN            TableID = 10
	TABLE_ID_VLAN_1          TableID = 11
	TABLE_ID_TERMINATION_MAC TableID = 20
	TABLE_ID_UNICAST_ROUTING TableID = 30
	TABLE_ID_BRIDGING        TableID = 50
	TABLE_ID_ACL_POLICY      TableID = 60
	TABLE_ID_SA_LOOKUP       TableID = 254
)

func (t TableID) String() string {
	switch t {
	case TABLE_ID_INGRESS_PORT:
		return "INGRESS"
	case TABLE_ID_PORT_DSCP_TRUST:
		return "PORT_DSCP_TRUST"
	case TABLE_ID_PORT_PCP_TRUST:
		return "PORT_PCP_TRUST"
	case TABLE_ID_VLAN:
		return "VLAN"
	case TABLE_ID_VLAN_1:
		return "VLAN1"
	case TABLE_ID_TERMINATION_MAC:
		return "TERMINATION_MAC"
	case TABLE_ID_UNICAST_ROUTING:
		return "UNICAST_ROUTING"
	case TABLE_ID_BRIDGING:
		return "BRIDGING"
	case TABLE_ID_ACL_POLICY:
		return "ACL_POLICY"
	case TABLE_ID_SA_LOOKUP:
		return "SA_LOOKUP"
	}
	return fmt.Sprintf("unknown(%d)", t)
}

func flowDataToBytes(data interface{}) [252]byte {
	var buf bytes.Buffer
	binary.Write(&buf, binary.LittleEndian, data)
	var buff [252]byte
	for idx, b := range buf.Bytes() {
		buff[idx] = b
	}
	return buff
}

func AddVLANFlowEntry(vid, port int, tagged bool) error {
	var entry C.ofdpaFlowEntry_t
	C.ofdpaFlowEntryInit(C.OFDPA_FLOW_TABLE_ID_t(TABLE_ID_VLAN), &entry)
	var vlan C.ofdpaVlanFlowEntry_t
	vlan.gotoTableId = C.OFDPA_FLOW_TABLE_ID_t(TABLE_ID_TERMINATION_MAC)
	vlan.match_criteria.inPort = C.uint32_t(port)
	vlan.match_criteria.vlanId = C.OFDPA_VID_PRESENT | C.uint16_t(vid)
	vlan.match_criteria.vlanIdMask = C.OFDPA_VID_PRESENT | C.OFDPA_VID_EXACT_MASK
	entry.flowData = flowDataToBytes(vlan)

	err := NewOFDPAError(C.ofdpaFlowAdd(&entry))
	if err != nil {
		return err
	}

	if !tagged {
		vlan.match_criteria.vlanId = C.uint16_t(0)
		vlan.setVlanIdAction = C.uint16_t(1)
		vlan.newVlanId = C.OFDPA_VID_PRESENT | C.uint16_t(vid)
		entry.flowData = flowDataToBytes(vlan)
		err = NewOFDPAError(C.ofdpaFlowAdd(&entry))
		if err != nil {
			return err
		}
	}
	return nil
}

func AddMACTerminationFlow(vid, port int, dst string) error {
	var entry C.ofdpaFlowEntry_t
	C.ofdpaFlowEntryInit(C.OFDPA_FLOW_TABLE_ID_t(TABLE_ID_TERMINATION_MAC), &entry)
	var mac C.ofdpaTerminationMacFlowEntry_t
	if port > 0 {
		mac.match_criteria.inPort = C.uint32_t(port)
		mac.match_criteria.inPortMask = C.OFDPA_INPORT_EXACT_MASK
	}
	mac.match_criteria.vlanId = C.OFDPA_VID_PRESENT | C.uint16_t(vid)
	mac.match_criteria.vlanIdMask = C.OFDPA_VID_PRESENT | C.OFDPA_VID_EXACT_MASK
	mac.match_criteria.etherType = 0x0800
	d, err := newOFDPAMacAddr(dst)
	if err != nil {
		return err
	}
	mac.match_criteria.destMac = d
	mask, _ := newOFDPAMacAddr("ff:ff:ff:ff:ff:ff")
	mac.match_criteria.destMacMask = mask
	mac.gotoTableId = C.OFDPA_FLOW_TABLE_ID_t(TABLE_ID_UNICAST_ROUTING)
	entry.flowData = flowDataToBytes(mac)

	return NewOFDPAError(C.ofdpaFlowAdd(&entry))
}

func AddUnicastRoutingFlow(dst string, gid GroupID, vrf int, packetIn bool) error {
	ip, n, err := net.ParseCIDR(dst)
	if err != nil {
		return err
	}
	ones, _ := n.Mask.Size()
	var entry C.ofdpaFlowEntry_t
	C.ofdpaFlowEntryInit(C.OFDPA_FLOW_TABLE_ID_t(TABLE_ID_UNICAST_ROUTING), &entry)
	var unicast C.ofdpaUnicastRoutingFlowEntry_t
	if vrf > 0 {
		unicast.match_criteria.vrf = C.uint16_t(vrf)
		unicast.match_criteria.vrfMask = C.OFDPA_VRF_VALUE_MASK
	}
	unicast.match_criteria.etherType = 0x0800
	unicast.match_criteria.dstIp4 = C.in_addr_t(binary.BigEndian.Uint32(ip.To4()))
	unicast.match_criteria.dstIp4Mask = C.in_addr_t((1<<uint(ones) - 1) << (32 - uint(ones)))
	unicast.gotoTableId = C.OFDPA_FLOW_TABLE_ID_t(TABLE_ID_ACL_POLICY)
	if packetIn {
		unicast.outputPort = C.OFDPA_PORT_CONTROLLER
	} else {
		unicast.groupID = C.uint32_t(gid)
	}
	entry.flowData = flowDataToBytes(unicast)

	return NewOFDPAError(C.ofdpaFlowAdd(&entry))
}

func AddBridgingFlow(vid, port int, dstMAC string) error {
	var entry C.ofdpaFlowEntry_t
	C.ofdpaFlowEntryInit(C.OFDPA_FLOW_TABLE_ID_t(TABLE_ID_BRIDGING), &entry)
	var br C.ofdpaBridgingFlowEntry_t
	br.gotoTableId = C.OFDPA_FLOW_TABLE_ID_t(TABLE_ID_ACL_POLICY)
	gid, _ := NewGroupID(GROUP_TYPE_L2_INTERFACE, vid, port, 0, 0)
	br.groupID = C.uint32_t(gid)
	br.match_criteria.vlanId = C.OFDPA_VID_PRESENT | C.uint16_t(vid)
	br.match_criteria.vlanIdMask = C.OFDPA_VID_PRESENT | C.OFDPA_VID_EXACT_MASK
	mac, err := newOFDPAMacAddr(dstMAC)
	if err != nil {
		return err
	}
	br.match_criteria.destMac = mac
	mask, _ := newOFDPAMacAddr("ff:ff:ff:ff:ff:ff")
	br.match_criteria.destMacMask = mask
	entry.flowData = flowDataToBytes(br)

	return NewOFDPAError(C.ofdpaFlowAdd(&entry))
}

func AddACLPolicyFlow(inPort, etherType int, srcMAC, dstMAC, srcPrefix, dstPrefix string) error {
	var entry C.ofdpaFlowEntry_t
	C.ofdpaFlowEntryInit(C.OFDPA_FLOW_TABLE_ID_t(TABLE_ID_ACL_POLICY), &entry)
	var acl C.ofdpaPolicyAclFlowEntry_t
	if inPort > 0 {
		acl.match_criteria.inPort = C.uint32_t(inPort)
		acl.match_criteria.inPortMask = C.OFDPA_INPORT_FIELD_MASK
	}
	if srcMAC != "" {
		mac, err := newOFDPAMacAddr(srcMAC)
		if err != nil {
			return err
		}
		acl.match_criteria.srcMac = mac
		mask, _ := newOFDPAMacAddr("ff:ff:ff:ff:ff:ff")
		acl.match_criteria.srcMacMask = mask
	}
	if dstMAC != "" {
		mac, err := newOFDPAMacAddr(dstMAC)
		if err != nil {
			return err
		}
		acl.match_criteria.destMac = mac
		mask, _ := newOFDPAMacAddr("ff:ff:ff:ff:ff:ff")
		acl.match_criteria.destMacMask = mask
	}
	if etherType > 0 {
		acl.match_criteria.etherType = C.uint16_t(etherType)
		acl.match_criteria.etherTypeMask = C.OFDPA_ETHERTYPE_EXACT_MASK
	} else {
		acl.match_criteria.etherTypeMask = C.OFDPA_ETHERTYPE_ALL_MASK
	}
	acl.outputPort = C.OFDPA_PORT_CONTROLLER
	entry.flowData = flowDataToBytes(acl)

	return NewOFDPAError(C.ofdpaFlowAdd(&entry))
}

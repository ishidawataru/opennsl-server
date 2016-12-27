package main

import (
	//#cgo CFLAGS: -I../../usr/include/ofdpa
	//#cgo LDFLAGS: -lofdpa_rpc_client -L ./
	//#include "ofdpa_api.h"
	//#include <stdlib.h>
	"C"

	"fmt"
	"io"
	"log"
	"net"
	//	"os"
	"sync"
	"unsafe"

	"github.com/google/gopacket"
	"github.com/google/gopacket/layers"
	"github.com/songgao/water"
	"github.com/vishvananda/netlink"
)

var rpcMutex sync.Mutex

type OFDPAError struct {
	rc int
}

func NewOFDPAError(rc C.OFDPA_ERROR_t) error {
	if int(rc) == 0 {
		return nil
	}
	return &OFDPAError{
		rc: int(rc),
	}
}

func (e *OFDPAError) Error() string {
	switch e.rc {
	case 0:
		return "NONE"
	case -20:
		return "RPC"
	case -21:
		return "INTERNAL"
	case -22:
		return "PARAM"
	case -23:
		return "ERROR"
	case -24:
		return "FULL"
	case -25:
		return "EXISTS"
	case -26:
		return "TIMEOUT"
	case -27:
		return "FAIL"
	case -28:
		return "DISABLED"
	case -29:
		return "UNAVAIL"
	case -30:
		return "NOT_FOUND"
	case -31:
		return "EMPTY"
	case -32:
		return "REQUEST_DENIED"
	case -33:
		return "NOT_IMPLEMENTED_YET"
	}
	return fmt.Sprintf("unknown(%d)", e)
}

type packetInReason uint8

const (
	PACKET_IN_REASON_NO_MATCH packetInReason = iota
	PACKET_IN_REASON_ACTION
	PACKET_IN_REASON_INVALID_TTL
	PACKET_IN_REASON_OAM
)

func (r packetInReason) String() string {
	switch r {
	case PACKET_IN_REASON_NO_MATCH:
		return "NO_MATCH"
	case PACKET_IN_REASON_ACTION:
		return "ACTION"
	case PACKET_IN_REASON_INVALID_TTL:
		return "INVALID_TTL"
	case PACKET_IN_REASON_OAM:
		return "OAM"
	}
	return fmt.Sprintf("unknown(%d)", r)
}

func setMACLearningMode(enable bool) error {
	var cfg C.ofdpaSrcMacLearnModeCfg_t
	cfg.destPortNum = C.OFDPA_PORT_CONTROLLER
	mode := C.OFDPA_DISABLE
	if enable {
		mode = C.OFDPA_ENABLE
	}
	return NewOFDPAError(C.ofdpaSourceMacLearningSet(C.OFDPA_CONTROL_t(mode), &cfg))
}

func getMAXPktSize() (int, error) {
	var size uint32
	err := NewOFDPAError(C.ofdpaMaxPktSizeGet((*C.uint32_t)(&size)))
	if err != nil {
		return 0, err
	}
	return int(size), nil
}

func sendLoop(port int, tap io.Reader, size int) error {
	var buff C.ofdpa_buffdesc
	buf := make([]byte, size)
	for {
		l, err := tap.Read(buf)
		if err != nil {
			log.Printf("read err: %v", err)
			continue
		}
		packet := gopacket.NewPacket(buf[:l], layers.LayerTypeEthernet, gopacket.Default)
		fmt.Printf("OUT PORT: %d, PACKET: %s", port, packet)
		if la := packet.Layer(layers.LayerTypeEthernet); la != nil {
			eth := la.(*layers.Ethernet)
			switch eth.EthernetType {
			case layers.EthernetTypeARP, layers.EthernetTypeIPv4:
				p := unsafe.Pointer(C.CString(string(buf)))
				buff.size = C.uint32_t(l)
				buff.pstart = (*C.char)(p)
				fmt.Println("sending....")
				err = NewOFDPAError(C.ofdpaPktSend(&buff, 0, C.uint32_t(port), C.uint32_t(port)))
				fmt.Println("done....")
				if err != nil {
					log.Printf("failed to send packet out:", err)
					continue
				}
				C.free(p)
			}
		}
	}
	return nil
}

type vlan struct {
	vid    int
	ports  []int
	subnet string
	host   string
}

type port struct {
	port int
	vid  int
	mac  string
}

func main() {
	vlans := make(map[int]*vlan)

	vlans[10] = &vlan{
		vid:    10,
		ports:  []int{9, 10},
		subnet: "192.168.10.0/24",
		host:   "192.168.10.10/32",
	}

	vlans[20] = &vlan{
		vid:    20,
		ports:  []int{17, 18},
		subnet: "192.168.20.0/24",
		host:   "192.168.20.10/32",
	}

	ifMap := make(map[int]*port)

	for _, v := range vlans {
		for _, p := range v.ports {
			ifMap[p] = &port{
				port: p,
				vid:  v.vid,
			}
		}
	}

	p := C.CString("test")
	defer C.free(unsafe.Pointer(p))
	fmt.Println(C.ofdpaClientInitialize(p))

	if err := setMACLearningMode(true); err != nil {
		log.Fatal("failed to set MAC learning mode:", err)
	}

	size, err := getMAXPktSize()
	if err != nil {
		log.Fatal("failed to get max pkt size:", err)
	}

	tapMap := make(map[int]*water.Interface)

	for _, v := range vlans {
		vid := v.vid
		pop := true
		for _, p := range v.ports {
			name := fmt.Sprintf("swp%d", p)
			tap, err := water.NewTAP(name)
			if err != nil {
				log.Fatal(err)
			}
			link, err := netlink.LinkByName(name)
			if err != nil {
				log.Fatal(err)
			}
			if err := netlink.LinkSetUp(link); err != nil {
				log.Fatal(err)
			}
			mac := link.Attrs().HardwareAddr.String()
			ifMap[p].mac = mac

			if err := AddMACTerminationFlow(vid, 0, mac); err != nil {
				log.Fatal("failed to add MAC termination flow:", err)
			}

			if err := AddACLPolicyFlow(p, 0x0806, "", mac, "", ""); err != nil {
				log.Fatal("failed to add ACL flow:", err)
			}

			tapMap[p] = tap

			if _, err := AddL2InterfaceGroup(vid, p, pop); err != nil {
				log.Fatal("failed to add L2 interface group:", err)
			}

			if err := AddVLANFlowEntry(vid, p, false); err != nil {
				log.Fatal("failed to add VLAN flow entry:", err)
			}

		}

		if _, err := AddL2FloodGroup(vid, v.ports, 0); err != nil {
			log.Fatal("failed to add L2 flood group:", err)
		}

		if err := AddUnicastRoutingFlow(v.host, 0, 0, true); err != nil {
			log.Fatal("failed to add Host flow:", err)
		}

	}
	if err := AddACLPolicyFlow(0, 0x0806, "", "ff:ff:ff:ff:ff:ff", "", ""); err != nil {
		log.Fatal("failed to add ACL flow:", err)
	}

	if err := NewOFDPAError(C.ofdpaClientPktSockBind()); err != nil {
		log.Fatal("failed to bind packet socket:", err)
	}

	for p, tap := range tapMap {
		go sendLoop(p, tap, size)
	}

	var pkt C.ofdpaPacket_t
	pkt.pktData.size = C.uint32_t(size)
	pp := C.malloc(C.size_t(size))
	defer C.free(pp)
	pkt.pktData.pstart = (*C.char)(pp)

	exists := make(map[string]bool)
	idx := 0

	for {
		if ret := int(C.ofdpaPktReceive(nil, &pkt)); ret < 0 {
			fmt.Printf("failed to receive pkt: %d", ret)
			return
		}
		table := TableID(pkt.tableId)
		port := int(pkt.inPortNum)
		s := int(pkt.pktData.size)
		b := C.GoBytes(unsafe.Pointer(pkt.pktData.pstart), C.int(s))
		fmt.Printf("IN PORT: %d, REASON: %s, TABLE ID: %s\n", pkt.inPortNum, packetInReason(pkt.reason), table)
		packet := gopacket.NewPacket(b, layers.LayerTypeEthernet, gopacket.Default)
		fmt.Println(packet)

		if table == TABLE_ID_SA_LOOKUP {
			if l := packet.Layer(layers.LayerTypeEthernet); l != nil {
				eth := l.(*layers.Ethernet)

				vid := ifMap[port].vid
				dst := eth.SrcMAC.String()
				err := AddBridgingFlow(vid, port, dst)
				if err != nil {
					log.Fatal("failed to add Bridging flow:", err)
				}
			}

		} else {
			var dst string
			if l := packet.Layer(layers.LayerTypeEthernet); l != nil {
				eth := l.(*layers.Ethernet)
				dst = eth.SrcMAC.String()
			}

			tap, ok := tapMap[port]
			if !ok {
				log.Printf("no tap port for %d", port)
				continue
			}
			tap.Write(b)

			var host string
			if l := packet.Layer(layers.LayerTypeARP); l != nil {
				arp := l.(*layers.ARP)
				host = fmt.Sprintf("%s/32", net.IP(arp.SourceProtAddress))
			} else if l = packet.Layer(layers.LayerTypeIPv4); l != nil {
				ip4 := l.(*layers.IPv4)
				host = fmt.Sprintf("%s/32", ip4.SrcIP)
			} else {
				continue
			}

			_, ok = exists[host]
			if !ok {
				exists[host] = true
				vid := ifMap[port].vid
				gid, _ := NewGroupID(GROUP_TYPE_L2_INTERFACE, vid, port, 0, 0)
				src := ifMap[port].mac
				gid, err := AddL3UnicastGroup(vid, src, dst, gid, idx)
				idx++
				if err != nil {
					log.Fatal("failed to add L3 unicast group:", err)
				}
				if err := AddUnicastRoutingFlow(host, gid, 0, false); err != nil {
					log.Fatal("failed to add Host flow:", err)
				}
			}
		}
	}

}

#include <sai.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/if.h>
#include <linux/if_tun.h>

#include <ofdpa/ofdpa_api.h>
#include <ofdpa/ofdpa_datatypes.h>

#include <netlink/route/rtnl.h>
#include <netlink/route/link.h>

#include <arpa/inet.h>

#include <pthread.h>

#define OBJECT_TYPE_SHIFT 48
#define VLAN_OFFSET 10

static sai_status_t ofdpa_sai_add_vlan_flow_entry(int vid, int port, bool tagged);
static sai_status_t ofdpa_sai_add_untagged_vlan_flow_entry(int vid, int port);
static sai_status_t ofdpa_sai_add_tagged_vlan_flow_entry(int vid, int port);
static sai_status_t ofdpa_sai_delete_vlan_flow_entry(int vid, int port, bool tagged);
static sai_status_t ofdpa_sai_delete_untagged_vlan_flow_entry(int vid, int port);
static sai_status_t ofdpa_sai_delete_tagged_vlan_flow_entry(int vid, int port);

static sai_status_t ofdpa_sai_add_bridging_flow(int vid, int port, ofdpaMacAddr_t dst);
static sai_status_t ofdpa_sai_flush_bridging_flows(int vid, int port);

static sai_status_t ofdpa_sai_add_unicast_routing_flow(int gid, int vrf, bool packet_in, int dst_v4, int mask_v4) ;
static sai_status_t ofdpa_sai_add_acl_policy_flow(int port, int ether_type, ofdpaMacAddr_t *src, ofdpaMacAddr_t *dst, int priority);

static sai_status_t ofdpa_sai_add_l2_interface_group(int vid, int port, bool pop);
static sai_status_t ofdpa_sai_delete_l2_interface_group(int vid, int port);

static sai_status_t ofdpa_sai_add_l3_unicast_group(int vid, ofdpaMacAddr_t src, ofdpaMacAddr_t dst, int ref_gid, int index, int *gid);

static sai_status_t ofdpa_sai_add_mac_termination_flow(int vid, int port, ofdpaMacAddr_t dst);
static sai_status_t ofdpa_sai_modify_mac_termination_flow(int vid, int port, ofdpaMacAddr_t dst);
static sai_status_t ofdpa_sai_delete_mac_termination_flow(int vid, int port, ofdpaMacAddr_t dst);

static sai_object_id_t g_switch_id = (sai_object_id_t)SAI_OBJECT_TYPE_SWITCH << OBJECT_TYPE_SHIFT;
static sai_object_id_t g_port_id = (sai_object_id_t)SAI_OBJECT_TYPE_PORT << OBJECT_TYPE_SHIFT;
static sai_object_id_t g_queue_id = (sai_object_id_t)SAI_OBJECT_TYPE_QUEUE << OBJECT_TYPE_SHIFT;
static sai_object_id_t g_cpu_port_id = (sai_object_id_t)SAI_OBJECT_TYPE_PORT << OBJECT_TYPE_SHIFT | 0x1000;
static sai_object_id_t g_virtual_rid = (sai_object_id_t)SAI_OBJECT_TYPE_VIRTUAL_ROUTER << OBJECT_TYPE_SHIFT;
static sai_object_id_t g_router_intf = (sai_object_id_t)SAI_OBJECT_TYPE_ROUTER_INTERFACE << OBJECT_TYPE_SHIFT;
static sai_object_id_t g_default_1q_bridge_id = (sai_object_id_t)SAI_OBJECT_TYPE_BRIDGE << OBJECT_TYPE_SHIFT;
static sai_object_id_t g_default_vid = (sai_object_id_t)SAI_OBJECT_TYPE_VLAN << OBJECT_TYPE_SHIFT;
static sai_object_id_t g_vlan_member = (sai_object_id_t)SAI_OBJECT_TYPE_VLAN_MEMBER << OBJECT_TYPE_SHIFT;
static sai_object_id_t g_bridge_port = (sai_object_id_t)SAI_OBJECT_TYPE_BRIDGE_PORT << OBJECT_TYPE_SHIFT;
static sai_object_id_t g_default_trap_group = (sai_object_id_t)SAI_OBJECT_TYPE_HOSTIF_TRAP_GROUP << OBJECT_TYPE_SHIFT;
static sai_object_id_t g_policer = (sai_object_id_t)SAI_OBJECT_TYPE_POLICER << OBJECT_TYPE_SHIFT;

static uint32_t max_pkt_size = 0;
static uint32_t port_num = 0;
static int l3_unicast_idx = 0;
static pthread_mutex_t m;

#define MAX_NEIGHBORS 1024
#define MAX_PORTS 128

static pthread_t pt;

typedef struct {
    uint32_t ip;
    ofdpaMacAddr_t mac;
    int gid;  // OFDPA l3 unicast group id
    sai_object_id_t oid; // SAI NEXTHOP oid
} ofdpa_sai_neighbor_t;

struct ofdpa_sai_port_s;
struct ofdpa_sai_vlan_member_s;

typedef struct ofdpa_sai_vlan_member_s {
    sai_object_id_t oid; // SAI VLAN MEMBER oid
    int vid;
    bool tagged;
    struct ofdpa_sai_port_s *port;
    struct ofdpa_sai_vlan_member_s *next;
} ofdpa_sai_vlan_member_t;

typedef struct ofdpa_sai_port_s {
    int index; // OFDPA PORT NUM
    char name[32];
    int fd;
    sai_object_id_t port_oid;
    sai_object_id_t hostif_oid;
    pthread_t pt;
    sai_object_id_t router_if_oid;
    int num_neighbor;
    ofdpa_sai_neighbor_t neighbors[MAX_NEIGHBORS];
    ofdpaMacAddr_t mac;
    sai_object_id_t bridge_port_oid;
    int i; // index of the list
    int vid; // current access vid
    bool disabled;
    ofdpa_sai_vlan_member_t *vlans;
} ofdpa_sai_port_t;

static ofdpa_sai_port_t *ports;

static ofdpa_sai_port_t* get_ofdpa_sai_port_by_port_index(int index) {
    int i;
    for ( i = 0; i < port_num; i++ ) {
        if ( ports[i].index == index ) {
            return &ports[i];
        }
    }
    return NULL;
}

static ofdpa_sai_port_t* get_ofdpa_sai_port_by_port_oid(sai_object_id_t oid) {
    int i;
    for ( i = 0; i < port_num; i++ ) {
        if ( ports[i].port_oid == oid ) {
            return &ports[i];
        }
    }
    return NULL;
}

static ofdpa_sai_port_t* get_ofdpa_sai_port_by_bridge_port_oid(sai_object_id_t oid) {
    int i;
    for ( i = 0; i < port_num; i++ ) {
        if ( ports[i].bridge_port_oid == oid ) {
            return &ports[i];
        }
    }
    return NULL;
}

static ofdpa_sai_port_t* get_ofdpa_sai_port_by_vlan_member_id(sai_object_id_t oid) {
    int i;
    ofdpa_sai_vlan_member_t *vlan = NULL;
    for ( i = 0; i < port_num; i++ ) {
        vlan = ports[i].vlans;
        while ( vlan != NULL ) {
            if ( vlan->oid == oid ) {
                return &ports[i];
            }
            vlan = vlan->next;
        }
    }
    return NULL;
}

static ofdpa_sai_vlan_member_t* get_ofdpa_sai_vlan_member_by_vlan_member_id(sai_object_id_t oid) {
    int i;
    ofdpa_sai_vlan_member_t *vlan = NULL;
    for ( i = 0; i < port_num; i++ ) {
        vlan = ports[i].vlans;
        while ( vlan != NULL ) {
            if ( vlan->oid == oid ) {
                return vlan;
            }
        }
    }
    return NULL;
}

static sai_status_t ofdpa_sai_add_vlan_member(ofdpa_sai_port_t *port, sai_object_id_t vlan_member_id, int vid, bool tagged) {
    ofdpa_sai_vlan_member_t *vlan, *prev = NULL;
    vlan = malloc(sizeof(ofdpa_sai_vlan_member_t));
    memset(vlan, 0, sizeof(ofdpa_sai_vlan_member_t));
    vlan->oid = vlan_member_id;
    vlan->vid = vid;
    vlan->tagged = tagged;
    vlan->port = port;
    if ( port->vlans == NULL ){
        port->vlans = vlan;
    } else {
        vlan = port->vlans;
        while ( vlan != NULL ) {
            prev = vlan;
            vlan = vlan->next;
        }
        prev->next = vlan;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t ofdpa_sai_delete_vlan_member(ofdpa_sai_port_t *port, sai_object_id_t vlan_member_id) {
    ofdpa_sai_vlan_member_t *vlan = port->vlans, *prev = NULL;
    while ( vlan != NULL ) {
        if ( vlan->oid == vlan_member_id ) {
            prev->next = vlan->next;
            free(vlan);
            return SAI_STATUS_SUCCESS;
        }
        prev = vlan;
        vlan = vlan->next;
    }
    return SAI_STATUS_FAILURE;
}

struct ofdpa_sai_vlan_s;

typedef struct ofdpa_sai_vlan_s {
    sai_object_id_t oid; // SAI VLAN OID
    int vid;
    int num_ports;
    ofdpa_sai_port_t ports[MAX_PORTS];
    int num_neighbor;
    ofdpa_sai_neighbor_t neighbors[MAX_NEIGHBORS];
    sai_object_id_t router_if_oid;
    struct ofdpa_sai_vlan_s *next;
    ofdpaMacAddr_t mac;
} ofdpa_sai_vlan_t;

static ofdpa_sai_vlan_t *vlans;

static ofdpa_sai_vlan_t* append_new_vlan() {
    ofdpa_sai_vlan_t *v = vlans, *prev = NULL;
    while ( v != NULL ) {
        prev = v;
        v = v->next;
    }
    v = malloc(sizeof(ofdpa_sai_vlan_t));
    memset(v, 0, sizeof(ofdpa_sai_vlan_t));

    prev->next = v;

    return v;
}

static ofdpa_sai_vlan_t* get_ofdpa_sai_vlan_by_vlan_oid(sai_object_id_t oid) {
    ofdpa_sai_vlan_t *v = vlans;
    while (v != NULL ) {
        if ( v->oid == oid ) {
            return v;
        }
        v = v->next;
    }
    return NULL;
}

static sai_status_t get_mac_address(const char *name, ofdpaMacAddr_t *mac) {
    struct nl_sock *sock;
    struct rtnl_link *link;
    struct nl_cache *cache;
    struct nl_addr *addr;
    int i;
    char buf[64];
    char *p;

    if ( name == NULL || mac == NULL ) {
        return SAI_STATUS_FAILURE;
    }
    sock = nl_socket_alloc();
    if (!sock) {
        return SAI_STATUS_FAILURE;
    }
    if (nl_connect(sock, NETLINK_ROUTE) < 0) {
        nl_socket_free(sock);
        return SAI_STATUS_FAILURE;
    }
    if (rtnl_link_alloc_cache(sock, AF_UNSPEC, &cache) < 0) {
        nl_socket_free(sock);
        return SAI_STATUS_FAILURE;
    }
    link = rtnl_link_get_by_name(cache, name);
    if ( link == NULL ) {
        nl_socket_free(sock);
        return SAI_STATUS_FAILURE;
    }

    addr = rtnl_link_get_addr(link);

    nl_addr2str(addr, buf, 64);

    p = (char *)nl_addr_get_binary_addr(addr);

    for ( i = 0; i < OFDPA_MAC_ADDR_LEN; i++ ) {
        mac->addr[i] = *(p + i);
    }

    rtnl_link_put(link);
    rtnl_addr_put(addr);
    nl_socket_free(sock);

    return SAI_STATUS_SUCCESS;
}

static bool is_broadcast(char *pkt) {
    int i;
    for ( i = 0; i < OFDPA_MAC_ADDR_LEN; i++ ) {
        if ( (pkt[i] & 0xff) != 0xff ) {
            return false;
        }
    }
    return true;
}

static ofdpaMacAddr_t mask_mac() {
    ofdpaMacAddr_t mask;
    int i;
    for( i = 0; i < OFDPA_MAC_ADDR_LEN; i++ ) {
        mask.addr[i] = 0xff;
    }
    return mask;
}

static print_mac(ofdpaMacAddr_t mac) {
    int i;
    for( i = 0; i < OFDPA_MAC_ADDR_LEN; i++ ) {
        printf("%02x:", mac.addr[i] & 0xff);
    }
    printf("\n");
}

static sai_status_t ofdpa_err2sai_status(OFDPA_ERROR_t err) {
    switch ( err ) {
    case OFDPA_E_NONE:
        return SAI_STATUS_SUCCESS;
    }
    printf("OFDPA ERR: %d\n", err);
    return SAI_STATUS_FAILURE;
}

static uint32_t ofdpa_sai_group_id(OFDPA_GROUP_ENTRY_TYPE_t type, int vid, int port, int index) {
    uint32_t id;
    ofdpaGroupTypeSet(&id, (uint32_t)type);
    if ( vid > 0 ) {
        ofdpaGroupVlanSet(&id, (uint32_t)vid);
    }
    if ( port > 0 ) {
        ofdpaGroupPortIdSet(&id, (uint32_t)port);
    }
    if ( index > 0 ) {
        ofdpaGroupIndexSet(&id, (uint32_t)index);
    }
    return id;
}

sai_status_t sai_create_vlan(
        _Out_ sai_object_id_t *vlan_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list){
    ofdpa_sai_vlan_t *vlan;
    int i, vid = 0;
    for ( i = 0; i < attr_count; i++ ) {
        switch ( attr_list[i].id ) {
        case SAI_VLAN_ATTR_VLAN_ID:
            vid = (int)attr_list[i].value.u16;
            break;
        }
    }
    if ( vid == 0 ) {
        return SAI_STATUS_FAILURE;
    }

    vlan = append_new_vlan();
    vlan->oid = *vlan_id;
    vlan->vid = vid;

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_remove_vlan(
        _In_ sai_object_id_t vlan_id){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_set_vlan_attribute(
        _In_ sai_object_id_t vlan_id,
        _In_ const sai_attribute_t *attr){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_get_vlan_attribute(
        _In_ sai_object_id_t vlan_id,
        _In_ const uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list){
    int i;

    if ( vlan_id != g_default_vid ) {
        return SAI_STATUS_NOT_SUPPORTED;
    }
    for ( i = 0; i < attr_count; i++ ){
        switch (attr_list[i].id) {
        case SAI_VLAN_ATTR_MEMBER_LIST:
            attr_list[i].value.objlist.count = 1;
            attr_list[i].value.objlist.list[0] = g_vlan_member;
            break;
        case SAI_VLAN_ATTR_STP_INSTANCE:
        case SAI_VLAN_ATTR_UNKNOWN_NON_IP_MCAST_OUTPUT_GROUP_ID:
        case SAI_VLAN_ATTR_UNKNOWN_IPV4_MCAST_OUTPUT_GROUP_ID:
        case SAI_VLAN_ATTR_UNKNOWN_IPV6_MCAST_OUTPUT_GROUP_ID:
        case SAI_VLAN_ATTR_UNKNOWN_LINKLOCAL_MCAST_OUTPUT_GROUP_ID:
        case SAI_VLAN_ATTR_INGRESS_ACL:
        case SAI_VLAN_ATTR_EGRESS_ACL:
            return SAI_STATUS_NOT_SUPPORTED;
        }
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_create_vlan_member(
        _Out_ sai_object_id_t *vlan_member_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list){
    int i, mode, old_vid;
    ofdpa_sai_vlan_t *vlan = NULL;
    ofdpa_sai_port_t *port = NULL;
    sai_object_id_t oid;
    sai_status_t err;
    bool pop = true, old_tagged;
    for ( i = 0; i < attr_count; i++ ) {
        switch ( attr_list[i].id ) {
        case SAI_VLAN_MEMBER_ATTR_VLAN_ID:
            oid = attr_list[i].value.oid;
            vlan = get_ofdpa_sai_vlan_by_vlan_oid(oid);
            break;
        case SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID:
            oid = attr_list[i].value.oid;
            port = get_ofdpa_sai_port_by_bridge_port_oid(oid);
            break;
        case SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE:
            if ( attr_list[i].value.s32 == SAI_VLAN_TAGGING_MODE_TAGGED ) {
                pop = false;
            }
            break;
        }
    }

    if ( vlan == NULL || port == NULL ) {
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    old_vid = port->vid;
    if ( pop ) {
        port->vid = vlan->vid;
    }
    // if this is about adding another VLAN to trunk port
    // we don't change port's vid
    // port's vid is for access port

    ofdpa_sai_add_vlan_member(port, *vlan_member_id, vlan->vid, !pop);

    port->disabled = true;
    port->num_neighbor = 0;

    if ( pop ) {
        // if this vlan is for access, we need to remove existing flows for
        // vlan access
        // but we don't need to remove rules for tagged flow
        err = ofdpa_sai_delete_untagged_vlan_flow_entry(old_vid, port->index);
        if ( err != SAI_STATUS_SUCCESS ) {
            printf("failed to delete vlan flow: vid: %d, port: %d\n", old_vid, port->index);
            return err;
        }

        err = ofdpa_sai_flush_bridging_flows(old_vid, port->index);
        if ( err != SAI_STATUS_SUCCESS ) {
            printf("failed to flush briding table: vid: %d, port: %d\n", old_vid, port->index);
            return err;
        }

        err = ofdpa_sai_delete_l2_interface_group(old_vid, port->index);
        if ( err != SAI_STATUS_SUCCESS ) {
            printf("failed to delete l2 intefrace group: vid %d, port: %d\n", old_vid, port->index);
            return err;
        }

    }

    err = ofdpa_sai_add_mac_termination_flow(vlan->vid, 0, port->mac);
    if ( err != SAI_STATUS_SUCCESS ) {
        printf("failed to add mac termination flow: vid: %d, port: %d\n", vlan->vid, port->index);
        return err;
    }

    err = ofdpa_sai_add_l2_interface_group(vlan->vid, port->index, pop);
    if ( err != SAI_STATUS_SUCCESS ) {
        printf("failed to add l2 intefrace group\n");
        return err;
    }

    err = ofdpa_sai_add_vlan_flow_entry(vlan->vid, port->index, !pop);
    if ( err != SAI_STATUS_SUCCESS ) {
        printf("failed to add vlan flow entry: %d %d\n", vlan->vid, port->index);
        return err;
    }

    port->disabled = false;

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_remove_vlan_member(_In_ sai_object_id_t vlan_member_id){
    int old_vid;
    bool old_tagged;
    ofdpa_sai_port_t *port = NULL;
    sai_status_t err;
    ofdpa_sai_vlan_member_t *vlan = NULL;

    if ( vlan_member_id == g_vlan_member ) {
        return SAI_STATUS_SUCCESS;
    }

    vlan = get_ofdpa_sai_vlan_member_by_vlan_member_id(vlan_member_id);
    if ( vlan == NULL ) {
        return SAI_STATUS_FAILURE;
    }

    old_vid = vlan->vid;
    old_tagged = vlan->tagged;
    port = vlan->port;

    port->disabled = true;

    err = ofdpa_sai_flush_bridging_flows(old_vid, port->index);
    if ( err != SAI_STATUS_SUCCESS ) {
        printf("failed to flush briding table: vid: %d, port: %d\n", old_vid, port->index);
        return err;
    }

    err = ofdpa_sai_delete_vlan_flow_entry(old_vid, port->index, old_tagged);
    if ( err != SAI_STATUS_SUCCESS ) {
        printf("failed to delete vlan flow: vid: %d, port: %d\n", old_vid, port->index);
        return err;
    }

    err = ofdpa_sai_delete_l2_interface_group(old_vid, port->index);
    if ( err != SAI_STATUS_SUCCESS ) {
        printf("failed to delete l2 intefrace group\n");
        return err;
    }

    err = ofdpa_sai_delete_mac_termination_flow(old_vid, 0, port->mac);
    if ( err != SAI_STATUS_SUCCESS ) {
        printf("failed to delete mac termination flow: vid: %d, port: %d\n", port->vid, port->index);
        return err;
    }

    if ( !vlan->tagged ) {
        port->vid = port->i + VLAN_OFFSET;

        err = ofdpa_sai_add_l2_interface_group(port->vid, port->index, true);
        if ( err != SAI_STATUS_SUCCESS ) {
            printf("failed to add l2 intefrace group\n");
            return err;
        }

        err = ofdpa_sai_add_untagged_vlan_flow_entry(port->vid, port->index);
        if ( err != SAI_STATUS_SUCCESS ) {
            printf("failed to add vlan flow entry: %d %d\n", port->vid, port->index);
            return err;
        }
    }

    port->disabled = false;
    return ofdpa_sai_delete_vlan_member(port, vlan_member_id);
}

sai_status_t sai_set_vlan_member_attribute(
        _In_ sai_object_id_t vlan_member_id,
        _In_ const sai_attribute_t *attr){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_get_vlan_member_attribute(
        _In_ sai_object_id_t vlan_member_id,
        _In_ const uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list){
    int i;
    if ( vlan_member_id != g_vlan_member ) {
        return SAI_STATUS_NOT_SUPPORTED;
    }
    for ( i = 0; i < attr_count; i++ ){
        switch (attr_list[i].id) {
        case SAI_VLAN_MEMBER_ATTR_VLAN_ID:
            attr_list[i].value.oid = g_default_vid;
            break;
        }
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_get_vlan_stats(
        _In_ sai_object_id_t vlan_id,
        _In_ uint32_t number_of_counters,
        _In_ const sai_vlan_stat_t *counter_ids,
        _Out_ uint64_t *counters){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_clear_vlan_stats(
        _In_ sai_object_id_t vlan_id,
        _In_ uint32_t number_of_counters,
        _In_ const sai_vlan_stat_t *counter_ids){
    return SAI_STATUS_SUCCESS;
}

sai_vlan_api_t vlan_api = {
    .create_vlan = sai_create_vlan,
    .remove_vlan = sai_remove_vlan,
    .set_vlan_attribute = sai_set_vlan_attribute,
    .get_vlan_attribute = sai_get_vlan_attribute,
    .create_vlan_member = sai_create_vlan_member,
    .remove_vlan_member = sai_remove_vlan_member,
    .set_vlan_member_attribute = sai_set_vlan_member_attribute,
    .get_vlan_member_attribute = sai_get_vlan_member_attribute,
    .get_vlan_stats = sai_get_vlan_stats,
    .clear_vlan_stats = sai_clear_vlan_stats,
};

sai_status_t sai_create_router_interface(
        _Out_ sai_object_id_t *rif_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list){
    int i, j;
    ofdpa_sai_port_t *port;
    ofdpa_sai_vlan_t *vlan;
    ofdpaMacAddr_t mac;

    for ( i = 0; i < attr_count; i++ ) {
        switch(attr_list[i].id) {
        case SAI_ROUTER_INTERFACE_ATTR_TYPE:
            if ( attr_list[i].value.s32 == SAI_ROUTER_INTERFACE_TYPE_LOOPBACK ) {
                *rif_id = g_router_intf;
                return SAI_STATUS_SUCCESS;
            }
            break;
        case SAI_ROUTER_INTERFACE_ATTR_PORT_ID:
            port = get_ofdpa_sai_port_by_port_oid(attr_list[i].value.oid);
            break;
        case SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS:
            for ( j = 0; j < OFDPA_MAC_ADDR_LEN; j++ ) {
                mac.addr[j] = attr_list[i].value.mac[j];
            }
        case SAI_ROUTER_INTERFACE_ATTR_VLAN_ID:
            vlan = get_ofdpa_sai_vlan_by_vlan_oid(attr_list[i].value.oid);
            break;
        }
    }

    if ( port == NULL  && vlan == NULL ) {
        return SAI_STATUS_FAILURE;
    }

    if ( port != NULL ) {
        port->router_if_oid = *rif_id;
    }

    if ( vlan != NULL ) {
        vlan->router_if_oid = *rif_id;
        vlan->mac = mac;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_remove_router_interface(_In_ sai_object_id_t rif_id){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_set_router_interface_attribute(
        _In_ sai_object_id_t rif_id,
        _In_ const sai_attribute_t *attr){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_get_router_interface_attribute(
        _In_ sai_object_id_t rif_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list){
    return SAI_STATUS_SUCCESS;
}

sai_router_interface_api_t router_interface_api = {
    .create_router_interface = sai_create_router_interface,
    .remove_router_interface = sai_remove_router_interface,
    .set_router_interface_attribute = sai_set_router_interface_attribute,
    .get_router_interface_attribute = sai_get_router_interface_attribute,
};


sai_status_t sai_create_port(_Out_ sai_object_id_t *port_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list){
    int i;
    uint32_t count, *list, index;
    bool found = false;
    for( i = 0; i < attr_count; i++ ) {
        switch (attr_list[i].id) {
        case SAI_PORT_ATTR_HW_LANE_LIST:
            count = attr_list[i].value.u32list.count;
            list = attr_list[i].value.u32list.list;
            if ( count != 1 ) {
                printf("specify ofdpa port num in lane configuration\n");
                return SAI_STATUS_NOT_SUPPORTED;
            }
            index = list[0];
            found = true;
            break;
        }
        printf("create port attr: %d\n", attr_list[i].id);
    }
    if ( found == false ) {
        printf("no lane found\n");
        return SAI_STATUS_NOT_SUPPORTED;
    }
    for ( i = 0; i < port_num; i++ ) {
        printf("ports[%d].index = %d, index: %d\n", i, ports[i].index, index);
        if ( ports[i].index == index ) {
            ports[i].port_oid = *port_id;
            return SAI_STATUS_SUCCESS;
        }
    }
    printf("not found index: %d\n", index);
    return SAI_STATUS_FAILURE;
}

sai_status_t sai_remove_port(_In_ sai_object_id_t port_id) {
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_set_port_attribute(_In_ sai_object_id_t port_id, _In_ const sai_attribute_t *attr) {
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_get_port_attribute(_In_ sai_object_id_t port_id, _In_ uint32_t attr_count, _Inout_ sai_attribute_t *attr_list) {
    int i, count;
    for( i = 0; i < attr_count; i++ ) {
        switch (attr_list[i].id) {
        case SAI_PORT_ATTR_QOS_QUEUE_LIST:
            return SAI_STATUS_NOT_SUPPORTED;
//            count = attr_list[i].value.objlist.count;
//            printf("count: %d\n", count);
//            if ( count < 1 ) {
//                return SAI_STATUS_BUFFER_OVERFLOW;
//            }
//            attr_list[i].value.objlist.list[0] = g_queue_id;
        case SAI_PORT_ATTR_QOS_SCHEDULER_GROUP_LIST:
        case SAI_PORT_ATTR_INGRESS_PRIORITY_GROUP_LIST:
        case SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID:
        case SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID:
        case SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID:
        case SAI_PORT_ATTR_INGRESS_ACL:
        case SAI_PORT_ATTR_EGRESS_ACL:
        case SAI_PORT_ATTR_INGRESS_MIRROR_SESSION:
        case SAI_PORT_ATTR_EGRESS_MIRROR_SESSION:
        case SAI_PORT_ATTR_INGRESS_SAMPLEPACKET_ENABLE:
        case SAI_PORT_ATTR_EGRESS_SAMPLEPACKET_ENABLE:
        case SAI_PORT_ATTR_POLICER_ID:
        case SAI_PORT_ATTR_QOS_DOT1P_TO_TC_MAP:
        case SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP:
        case SAI_PORT_ATTR_QOS_DSCP_TO_TC_MAP:
        case SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP:
        case SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP:
        case SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP:
        case SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP:
        case SAI_PORT_ATTR_QOS_TC_TO_PRIORITY_GROUP_MAP:
        case SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_PRIORITY_GROUP_MAP:
        case SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_QUEUE_MAP:
        case SAI_PORT_ATTR_QOS_WRED_PROFILE_ID:
        case SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID:
        case SAI_PORT_ATTR_QOS_INGRESS_BUFFER_PROFILE_LIST:
        case SAI_PORT_ATTR_QOS_EGRESS_BUFFER_PROFILE_LIST:
        case SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL:
        case SAI_PORT_ATTR_META_DATA:
        case SAI_PORT_ATTR_EGRESS_BLOCK_PORT_LIST:
        case SAI_PORT_ATTR_HW_PROFILE_ID:
        case SAI_PORT_ATTR_EEE_ENABLE:
        case SAI_PORT_ATTR_EEE_IDLE_TIME:
        case SAI_PORT_ATTR_BIND_MODE:
            return SAI_STATUS_NOT_SUPPORTED;
        case SAI_PORT_ATTR_NUMBER_OF_INGRESS_PRIORITY_GROUPS:
            attr_list[i].value.u32 = 0;
            break;
        }
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_get_port_stats(_In_ sai_object_id_t port_id, _In_ uint32_t number_of_counters, _In_ const sai_port_stat_t *counter_ids, _Out_ uint64_t *counters) {
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_clear_port_stats(
        _In_ sai_object_id_t port_id,
        _In_ uint32_t number_of_counters,
        _In_ const sai_port_stat_t *counter_ids){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_clear_port_all_stats(
        _In_ sai_object_id_t port_id){
    return SAI_STATUS_SUCCESS;
}

sai_port_api_t port_api = {
    .create_port = sai_create_port,
    .remove_port = sai_remove_port,
    .set_port_attribute = sai_set_port_attribute,
    .get_port_attribute = sai_get_port_attribute,
    .get_port_stats = sai_get_port_stats,
    .clear_port_stats = sai_clear_port_stats,
    .clear_port_all_stats = sai_clear_port_all_stats,
};

static sai_status_t enableSourceMacLearning() {
    ofdpaSrcMacLearnModeCfg_t cfg;
    OFDPA_ERROR_t err;
    cfg.destPortNum = OFDPA_PORT_CONTROLLER;
    err = ofdpaSourceMacLearningSet(OFDPA_ENABLE, &cfg);
    if ( err != OFDPA_E_NONE ) {
        return SAI_STATUS_FAILURE;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t ofdpa_sai_init_port() {
    uint32_t i = 0, j, next;
    port_num = 0;
    OFDPA_ERROR_t err;
    while(1){
        err = ofdpaPortNextGet(i, &next);
        if ( err != OFDPA_E_NONE ) {
            break;
        }
        i = next;
        port_num++;
    }
    printf("port_num: %d\n", port_num);
    ports = malloc(port_num * sizeof(ofdpa_sai_port_t));
    memset(ports, 0, port_num * sizeof(ofdpa_sai_port_t));

    j = 0;

    for(i = 0; i < port_num; i++) {
        err = ofdpaPortNextGet(j, &next);
        if ( err != OFDPA_E_NONE ) {
            printf("should not happen: i: %d, j: %d\n", i, j);
            return SAI_STATUS_FAILURE;
        }

        j = next;

        ofdpa_buffdesc buf;
        ports[i].index = next;
        buf.size = 32;
        buf.pstart = ports[i].name;
        ofdpaPortNameGet(next, &buf);
        ports[i].i = i;
        ports[i].vid = i + VLAN_OFFSET;
    }

    return SAI_STATUS_SUCCESS;
}

void *ofdpa_sai_pkt_recv_loop(void *arg){
    ofdpaPacket_t pkt;
    pkt.pktData.size = max_pkt_size;
    pkt.pktData.pstart = (char *)malloc(max_pkt_size);
    OFDPA_ERROR_t err;
    sai_status_t status;
    ofdpa_sai_port_t *port = NULL;
    int i;

    while (1) {
        int fd;
        err = ofdpaPktReceive(NULL, &pkt);
        if ( err != OFDPA_E_NONE ) {
            printf("Receive fail: %d\n", err);
            return NULL;
        }
        printf("RECV: in-port: %d, reason: %d, table-id: %d\n", pkt.inPortNum, pkt.reason, pkt.tableId);

//        for ( i = 0; i < pkt.pktData.size; i++ ) {
//            printf("0x%02x ", pkt.pktData.pstart[i] & 0xff);
//        }
//        printf("\n");

        port = get_ofdpa_sai_port_by_port_index(pkt.inPortNum);
        if ( port == NULL ) {
            printf("failed to find port: %d\n", pkt.inPortNum);
            return NULL;
        }

        if ( port->disabled ) {
            printf("port %d is disabled\n", port->index);
            continue;
        }

        if ( pkt.tableId == OFDPA_FLOW_TABLE_ID_SA_LOOKUP || ( pkt.tableId == OFDPA_FLOW_TABLE_ID_ACL_POLICY &&  is_broadcast(pkt.pktData.pstart)) ) {
            ofdpaMacAddr_t src;
            for ( i = 0; i < OFDPA_MAC_ADDR_LEN; i++ ) {
                src.addr[i] = pkt.pktData.pstart[i+6] & 0xff;
            }
            status = ofdpa_sai_add_bridging_flow(port->vid, pkt.inPortNum, src);
            if ( status != SAI_STATUS_SUCCESS ) {
                printf("failed to add bridging flow\n");
            }
        }

        if ( (fd = port->fd) > 0 ) {
            write(fd, pkt.pktData.pstart, pkt.pktData.size);
        }
    }
}

void *ofdpa_sai_pkt_send_loop(void *arg) {
    ofdpa_buffdesc buf;
    buf.size = max_pkt_size;
    buf.pstart = (char *)malloc(max_pkt_size);
    OFDPA_ERROR_t err;
    ofdpa_sai_port_t *port = (ofdpa_sai_port_t *)arg;
    int size, i;

    while (1) {
        size = read(port->fd, buf.pstart, buf.size);
        if ( size < 0 ) {
            return NULL;
        }
        printf("SEND: in-port: %d, size: %d\n", port->index, size);
        buf.size = size;
//        for ( i = 0; i < size; i++ ) {
//            printf("0x%02x ", buf.pstart[i] & 0xff);
//        }
//        printf("\n");
        err = ofdpaPktSend(&buf, 0, port->index, 0);
        buf.size = max_pkt_size;
        if ( err != OFDPA_E_NONE) {
            printf("Send fail: %d\n", err);
            continue;
        }
    }
}

sai_status_t sai_create_switch(_Out_ sai_object_id_t* switch_id, _In_ uint32_t attr_count, _In_ const sai_attribute_t *attr_list) {
    int i;
    *switch_id = g_switch_id;
    OFDPA_ERROR_t err;
    ofdpaMacAddr_t mac;

    printf("switch_id: %lx\n", g_switch_id);

    for(i = 0; i < attr_count; i++){
        printf("attr: %d\n", attr_list[i].id);
    }

    err = ofdpaClientInitialize("sai");
    if ( err != OFDPA_E_NONE ) {
        return SAI_STATUS_FAILURE;
    }

    enableSourceMacLearning();

    ofdpaClientPktSockBind();

    ofdpaMaxPktSizeGet(&max_pkt_size);

    ofdpa_sai_init_port();

    mac = mask_mac();

    err = ofdpa_sai_add_acl_policy_flow(0, 0x0806, NULL, &mac, 0);
    if ( err != SAI_STATUS_SUCCESS ) {
        printf("failed to add acl policy flow for arp request\n");
        return err;
    }

    pthread_create(&pt, NULL, &ofdpa_sai_pkt_recv_loop, NULL);

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_remove_switch(_In_ sai_object_id_t switch_id) {
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_set_switch_attribute(_In_ sai_object_id_t switch_id, _In_ const sai_attribute_t *attr) {
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_get_switch_attribute(_In_ sai_object_id_t switch_id, _In_ sai_uint32_t attr_count, _Inout_ sai_attribute_t *attr_list) {
    int i, count;
    for(i = 0; i < attr_count; i++){
        switch (attr_list[i].id) {
        case SAI_SWITCH_ATTR_PORT_NUMBER:
            attr_list[i].value.u32 = 1;
            break;
        case SAI_SWITCH_ATTR_PORT_LIST:
            count = attr_list[i].value.objlist.count;
            if ( count < 1 ) {
                return SAI_STATUS_BUFFER_OVERFLOW;
            }
            attr_list[i].value.objlist.list[0] = g_port_id;
            attr_list[i].value.objlist.count = 1;
            break;
        case SAI_SWITCH_ATTR_CPU_PORT:
            attr_list[i].value.oid = g_cpu_port_id;
            break;
        case SAI_SWITCH_ATTR_DEFAULT_VIRTUAL_ROUTER_ID:
            attr_list[i].value.oid = g_virtual_rid;
            break;
        case SAI_SWITCH_ATTR_DEFAULT_1Q_BRIDGE_ID:
            attr_list[i].value.oid = g_default_1q_bridge_id;
            break;
        case SAI_SWITCH_ATTR_DEFAULT_VLAN_ID:
            attr_list[i].value.oid = g_default_vid;
            break;
        case SAI_SWITCH_ATTR_DEFAULT_TRAP_GROUP:
            attr_list[i].value.oid = g_default_trap_group;
            break;
        default:
            break;
        }
    }
    return SAI_STATUS_SUCCESS;
}

sai_switch_api_t switch_api = {
    .create_switch = sai_create_switch,
    .remove_switch = sai_remove_switch,
    .set_switch_attribute = sai_set_switch_attribute,
    .get_switch_attribute = sai_get_switch_attribute,
};

sai_status_t sai_create_bridge(
        _Out_ sai_object_id_t* bridge_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_remove_bridge(_In_ sai_object_id_t bridge_id){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_set_bridge_attribute(
        _In_ sai_object_id_t bridge_id,
        _In_ const sai_attribute_t *attr){


    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_get_bridge_attribute(
        _In_ sai_object_id_t bridge_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list){

    int i;
    if ( bridge_id != g_default_1q_bridge_id ) {
        return SAI_STATUS_NOT_SUPPORTED;
    }
    for(i = 0; i < attr_count; i++){
        switch (attr_list[i].id) {
        case SAI_BRIDGE_ATTR_PORT_LIST:
            attr_list[i].value.objlist.list[0] = g_bridge_port;
            attr_list[i].value.objlist.count = 1;
            break;
        }
    }
 
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_create_bridge_port(
        _Out_ sai_object_id_t* bridge_port_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list){
    int i;
    sai_object_id_t port_oid;
    ofdpa_sai_port_t *port = NULL;

    for ( i = 0; i < attr_count; i++ ) {
        switch ( attr_list[i].id ) {
        case SAI_BRIDGE_PORT_ATTR_TYPE:
            if ( attr_list[i].value.s32 != SAI_BRIDGE_PORT_TYPE_PORT ) {
                return SAI_STATUS_NOT_SUPPORTED;
            }
            break;
        case SAI_BRIDGE_PORT_ATTR_PORT_ID:
            port_oid = attr_list[i].value.oid;
            port = get_ofdpa_sai_port_by_port_oid(port_oid);
            break;
        case SAI_BRIDGE_PORT_ATTR_ADMIN_STATE:
            break;
        }
    }

    if ( port == NULL ) {
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    port->bridge_port_oid = *bridge_port_id;

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_remove_bridge_port(
        _In_ sai_object_id_t bridge_port_id){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_set_bridge_port_attribute(
        _In_ sai_object_id_t bridge_port_id,
        _In_ const sai_attribute_t *attr){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_get_bridge_port_attribute(
        _In_ sai_object_id_t bridge_port_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list){

    int i;
    if ( bridge_port_id != g_bridge_port ) {
        return SAI_STATUS_NOT_SUPPORTED;
    }
    for(i = 0; i < attr_count; i++){
        switch (attr_list[i].id) {
        case SAI_BRIDGE_PORT_ATTR_PORT_ID:
            attr_list[i].value.oid = g_port_id;
            break;
        }
    }
    return SAI_STATUS_SUCCESS;
}

sai_bridge_api_t bridge_api = {
    .create_bridge = sai_create_bridge,
    .remove_bridge = sai_remove_bridge,
    .set_bridge_attribute = sai_set_bridge_attribute,
    .get_bridge_attribute = sai_get_bridge_attribute,
    .create_bridge_port = sai_create_bridge_port,
    .remove_bridge_port = sai_remove_bridge_port,
    .set_bridge_port_attribute = sai_set_bridge_port_attribute,
    .get_bridge_port_attribute = sai_get_bridge_port_attribute,
};

sai_status_t sai_create_route_entry(
        _In_ const sai_route_entry_t *route_entry,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list){
    sai_ip4_t prefix, mask;
    int i, j, gid = -1;
    bool packet_in = false, forward = false;
    sai_object_id_t oid;

    if( route_entry->destination.addr_family != SAI_IP_ADDR_FAMILY_IPV4 ) {
        return SAI_STATUS_SUCCESS;
    }

    for (  i = 0; i < attr_count; i++ ) {
        switch (attr_list[i].id) {
        case SAI_ROUTE_ENTRY_ATTR_PACKET_ACTION:
            if ( attr_list[i].value.s32 == SAI_PACKET_ACTION_FORWARD ) {
                forward = true;
            }
            printf("packet action: %d, %d\n", attr_list[i].value.s32, forward);
        case SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID:
            printf("nexthop oid: %lx\n", attr_list[i].value.oid);
            if ( sai_object_type_query(attr_list[i].value.oid) == SAI_OBJECT_TYPE_PORT ) {
                packet_in = true;
            } else {
                oid = attr_list[i].value.oid;
            }
        }
    }
    prefix = route_entry->destination.addr.ip4;
    mask = route_entry->destination.mask.ip4;

    if ( packet_in == true && forward == true ) {
        return ofdpa_sai_add_unicast_routing_flow(0, 0, packet_in, (int)prefix, (int)mask);
    }

    for ( i = 0; i < port_num; i++ ) {
        for ( j = 0; j < ports[i].num_neighbor; j++ ) {
            if ( ports[i].neighbors[j].oid == oid ) {
                printf("oid: %lx\n", oid);
                gid = ports[i].neighbors[j].gid;
                break;
            }
        }
        if ( gid > 0 ) {
            break;
        }
    }

    printf("nexthop oid: %lx, gid: %0x\n", oid, gid);

    if ( gid > 0 ) {
        return ofdpa_sai_add_unicast_routing_flow(gid, 0, packet_in, (int)prefix, (int)mask);
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_remove_route_entry(
        _In_ const sai_route_entry_t *route_entry){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_set_route_entry_attribute(
        _In_ const sai_route_entry_t *route_entry,
        _In_ const sai_attribute_t *attr){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_get_route_entry_attribute(
        _In_ const sai_route_entry_t *route_entry,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list){
    return SAI_STATUS_SUCCESS;
}

sai_route_api_t route_api = {
    .create_route_entry = sai_create_route_entry,
    .remove_route_entry = sai_remove_route_entry,
    .set_route_entry_attribute = sai_set_route_entry_attribute,
    .get_route_entry_attribute = sai_get_route_entry_attribute,
};

sai_status_t sai_create_acl_table(
        _Out_ sai_object_id_t *acl_table_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_remove_acl_table(_In_ sai_object_id_t acl_table_id){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_set_acl_table_attribute(
        _In_ sai_object_id_t acl_table_id,
        _In_ const sai_attribute_t *attr){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_get_acl_table_attribute(
        _In_ sai_object_id_t acl_table_id,
        _In_ uint32_t attr_count,
        _Out_ sai_attribute_t *attr_list){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_create_acl_entry(
        _Out_ sai_object_id_t *acl_entry_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_remove_acl_entry(_In_ sai_object_id_t acl_entry_id){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_set_acl_entry_attribute(
        _In_ sai_object_id_t acl_entry_id,
        _In_ const sai_attribute_t *attr){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_get_acl_entry_attribute(
        _In_ sai_object_id_t acl_entry_id,
        _In_ uint32_t attr_count,
        _Out_ sai_attribute_t *attr_list){
    return SAI_STATUS_SUCCESS;
}

sai_acl_api_t acl_api = {
    .create_acl_table                      =  sai_create_acl_table,
    .remove_acl_table                      =  sai_remove_acl_table,
    .set_acl_table_attribute               =  sai_set_acl_table_attribute,
    .get_acl_table_attribute               =  sai_get_acl_table_attribute,
    .create_acl_entry                      =  sai_create_acl_entry,
    .remove_acl_entry                      =  sai_remove_acl_entry,
    .set_acl_entry_attribute               =  sai_set_acl_entry_attribute,
    .get_acl_entry_attribute               =  sai_get_acl_entry_attribute,
//    .create_acl_counter                    =  sai_create_acl_counter,
//    .remove_acl_counter                    =  sai_remove_acl_counter,
//    .set_acl_counter_attribute             =  sai_set_acl_counter_attribute,
//    .get_acl_counter_attribute             =  sai_get_acl_counter_attribute,
//    .create_acl_range                      =  sai_create_acl_range,
//    .remove_acl_range                      =  sai_remove_acl_range,
//    .set_acl_range_attribute               =  sai_set_acl_range_attribute,
//    .get_acl_range_attribute               =  sai_get_acl_range_attribute,
//    .create_acl_table_group                =  sai_create_acl_table_group,
//    .remove_acl_table_group                =  sai_remove_acl_table_group,
//    .set_acl_table_group_attribute         =  sai_set_acl_table_group_attribute,
//    .get_acl_table_group_attribute         =  sai_get_acl_table_group_attribute,
//    .create_acl_table_group_member         =  sai_create_acl_table_group_member,
//    .remove_acl_table_group_member         =  sai_remove_acl_table_group_member,
//    .set_acl_table_group_member_attribute  =  sai_set_acl_table_group_member_attribute,
//    .get_acl_table_group_member_attribute  =  sai_get_acl_table_group_member_attribute,
};

static int tap_alloc(char *dev)
{
  struct ifreq ifr;
  int fd, err;

  if( (fd = open("/dev/net/tun", O_RDWR)) < 0 ) {
    printf("[TUN] failed to open /dev/net/tun\n");
    return -1;
  }

  memset(&ifr, 0, sizeof(ifr));

  /* Flags: IFF_TUN   - TUN device (no Ethernet headers)
   *        IFF_TAP   - TAP device
   *
   *        IFF_NO_PI - Do not provide packet information
   */
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
  if( *dev )
     strncpy(ifr.ifr_name, dev, IFNAMSIZ);

  if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ){
     close(fd);
     return err;
  }
  strcpy(dev, ifr.ifr_name);
  return fd;
}

static sai_status_t ofdpa_sai_add_bridging_flow(int vid, int port, ofdpaMacAddr_t dst) {
    ofdpaFlowEntry_t entry;
    ofdpaFlowEntryInit(OFDPA_FLOW_TABLE_ID_BRIDGING, &entry);
    ofdpaBridgingFlowEntry_t br;

    uint32_t gid = ofdpa_sai_group_id(OFDPA_GROUP_ENTRY_TYPE_L2_INTERFACE, vid, port, 0);
    memset(&br, 0, sizeof(ofdpaBridgingFlowEntry_t));

    printf("add_bridging_flow: vid: %d, port: %d, gid: %d\n", vid, port, gid);

    br.gotoTableId = OFDPA_FLOW_TABLE_ID_ACL_POLICY;
    br.groupID = gid;
    br.match_criteria.vlanId = OFDPA_VID_PRESENT | (uint16_t)vid;
    br.match_criteria.vlanIdMask = OFDPA_VID_PRESENT | OFDPA_VID_EXACT_MASK;
    br.match_criteria.destMac = dst;
    br.match_criteria.destMacMask = mask_mac();

    entry.flowData.bridgingFlowEntry = br;
    return ofdpa_err2sai_status(ofdpaFlowAdd(&entry));
}

static sai_status_t ofdpa_sai_flush_bridging_flows(int vid, int port) {
    ofdpaFlowEntry_t flow;
    sai_status_t err;
    ofdpaFlowEntryInit(OFDPA_FLOW_TABLE_ID_BRIDGING, &flow);
    uint32_t gid = ofdpa_sai_group_id(OFDPA_GROUP_ENTRY_TYPE_L2_INTERFACE, vid, port, 0);

    while ( true ) {
        err = ofdpa_err2sai_status(ofdpaFlowNextGet(&flow, &flow));
        if ( err != SAI_STATUS_SUCCESS ) {
            break;
        }

        if ( flow.flowData.bridgingFlowEntry.groupID != gid ) {
            continue;
        }

        err = ofdpa_err2sai_status(ofdpaFlowDelete(&flow));
        if ( err != SAI_STATUS_SUCCESS ) {
            return err;
        }
    }
    return SAI_STATUS_SUCCESS;
}

static ofdpaFlowEntry_t _ofdpa_sai_create_mac_termination_flow(int vid, int port, ofdpaMacAddr_t dst) {
    ofdpaFlowEntry_t entry;
    ofdpaTerminationMacFlowEntry_t mac;

    memset(&mac, 0, sizeof(ofdpaTerminationMacFlowEntry_t));

    ofdpaFlowEntryInit(OFDPA_FLOW_TABLE_ID_TERMINATION_MAC, &entry);
    if ( port > 0 ) {
        mac.match_criteria.inPort = (uint32_t)port;
        mac.match_criteria.inPortMask = OFDPA_INPORT_EXACT_MASK;
    }
    mac.match_criteria.vlanId = OFDPA_VID_PRESENT | (uint16_t)vid;
    mac.match_criteria.vlanIdMask = OFDPA_VID_PRESENT | OFDPA_VID_EXACT_MASK;
    mac.match_criteria.etherType = 0x0800;
    mac.match_criteria.destMac = dst;
    mac.match_criteria.destMacMask = mask_mac();
    mac.gotoTableId = OFDPA_FLOW_TABLE_ID_UNICAST_ROUTING;
    entry.flowData.terminationMacFlowEntry = mac;

    return entry;
}

static sai_status_t ofdpa_sai_add_mac_termination_flow(int vid, int port, ofdpaMacAddr_t dst) {
    ofdpaFlowEntry_t entry = _ofdpa_sai_create_mac_termination_flow(vid, port, dst);
    return ofdpa_err2sai_status(ofdpaFlowAdd(&entry));
}

static sai_status_t ofdpa_sai_modify_mac_termination_flow(int vid, int port, ofdpaMacAddr_t dst) {
    ofdpaFlowEntry_t entry = _ofdpa_sai_create_mac_termination_flow(vid, port, dst);
    return ofdpa_err2sai_status(ofdpaFlowModify(&entry));
}

static sai_status_t ofdpa_sai_delete_mac_termination_flow(int vid, int port, ofdpaMacAddr_t dst) {
    ofdpaFlowEntry_t entry = _ofdpa_sai_create_mac_termination_flow(vid, port, dst);
    return ofdpa_err2sai_status(ofdpaFlowDelete(&entry));
}

static sai_status_t ofdpa_sai_add_acl_policy_flow(int port, int ether_type, ofdpaMacAddr_t *src, ofdpaMacAddr_t *dst, int priority) {
    ofdpaFlowEntry_t entry;
    ofdpaPolicyAclFlowEntry_t acl;
    memset(&acl, 0, sizeof(ofdpaPolicyAclFlowEntry_t));
    ofdpaFlowEntryInit(OFDPA_FLOW_TABLE_ID_ACL_POLICY, &entry);

    if ( port > 0 ) {
        acl.match_criteria.inPort = (uint32_t)port;
        acl.match_criteria.inPortMask = OFDPA_INPORT_FIELD_MASK;
    }
    if ( src != NULL ) {
        acl.match_criteria.srcMac = *src;
        acl.match_criteria.srcMacMask = mask_mac();
    }
    if ( dst != NULL ) {
        acl.match_criteria.destMac = *dst;
        acl.match_criteria.destMacMask = mask_mac();
    }
    if ( ether_type > 0 ) {
        acl.match_criteria.etherType = (uint16_t)ether_type;
        acl.match_criteria.etherTypeMask = OFDPA_ETHERTYPE_EXACT_MASK;
    } else {
        acl.match_criteria.etherTypeMask = OFDPA_ETHERTYPE_ALL_MASK;
    }
    acl.outputPort = OFDPA_PORT_CONTROLLER;
    entry.flowData.policyAclFlowEntry = acl;
    entry.priority = (uint32_t)priority;
    return ofdpa_err2sai_status(ofdpaFlowAdd(&entry));
}

static sai_status_t ofdpa_sai_add_l2_interface_group(int vid, int port, bool pop) {
    ofdpaGroupEntry_t group;
    uint32_t gid = ofdpa_sai_group_id(OFDPA_GROUP_ENTRY_TYPE_L2_INTERFACE, vid, port, 0);
    sai_status_t err;
    ofdpaGroupBucketEntry_t bucket;
    ofdpaL2InterfaceGroupBucketData_t l2;


    ofdpaGroupEntryInit(OFDPA_GROUP_ENTRY_TYPE_L2_INTERFACE, &group);
    group.groupId = gid;

    err = ofdpa_err2sai_status(ofdpaGroupAdd(&group));
    if ( err != SAI_STATUS_SUCCESS ) {
        return err;
    }

    ofdpaGroupBucketEntryInit(OFDPA_GROUP_ENTRY_TYPE_L2_INTERFACE, &bucket);

    bucket.groupId = gid;
    l2.outputPort = (uint32_t)port;
    if ( pop ) {
        l2.popVlanTag = 1;
    }
    bucket.bucketData.l2Interface = l2;
    return ofdpa_err2sai_status(ofdpaGroupBucketEntryAdd(&bucket));
}

static sai_status_t ofdpa_sai_delete_l2_interface_group(int vid, int port) {
    uint32_t gid = ofdpa_sai_group_id(OFDPA_GROUP_ENTRY_TYPE_L2_INTERFACE, vid, port, 0);
    return ofdpa_err2sai_status(ofdpaGroupDelete(gid));
}

static sai_status_t ofdpa_sai_add_l3_unicast_group(int vid, ofdpaMacAddr_t src, ofdpaMacAddr_t dst, int ref_gid, int index, int *gid) {
    ofdpaGroupEntry_t group;
    *gid = ofdpa_sai_group_id(OFDPA_GROUP_ENTRY_TYPE_L3_UNICAST, 0, 0, index);
    group.groupId = *gid;
    sai_status_t err;
    ofdpaGroupBucketEntry_t bucket;
    ofdpaL3UnicastGroupBucketData_t l3;

    err = ofdpa_err2sai_status(ofdpaGroupAdd(&group));
    if ( err != SAI_STATUS_SUCCESS ) {
        return err;
    }

    ofdpaGroupBucketEntryInit(OFDPA_GROUP_ENTRY_TYPE_L3_UNICAST, &bucket);

    bucket.groupId = *gid;
    bucket.referenceGroupId = (uint32_t)ref_gid;
    l3.srcMac = src;
    l3.dstMac = dst;
    l3.vlanId = OFDPA_VID_PRESENT | (uint32_t)vid;

    bucket.bucketData.l3Unicast = l3;
    return ofdpa_err2sai_status(ofdpaGroupBucketEntryAdd(&bucket));
}

static ofdpaFlowEntry_t _ofdpa_sai_create_vlan_flow_entry(int vid, int port, bool tagged) {
    ofdpaFlowEntry_t entry;
    ofdpaVlanFlowEntry_t vlan;
    ofdpaFlowEntryInit(OFDPA_FLOW_TABLE_ID_VLAN, &entry);
    memset(&vlan, 0, sizeof(ofdpaVlanFlowEntry_t));

    vlan.gotoTableId = OFDPA_FLOW_TABLE_ID_TERMINATION_MAC;
    vlan.match_criteria.inPort = (uint32_t)port;
    vlan.match_criteria.vlanId = OFDPA_VID_PRESENT | (uint16_t)vid;
    vlan.match_criteria.vlanIdMask  = OFDPA_VID_PRESENT | OFDPA_VID_EXACT_MASK;
    entry.flowData.vlanFlowEntry = vlan;
    if ( tagged ) {
        return entry;
    }
    vlan.match_criteria.vlanId = 0;
    vlan.setVlanIdAction = 1;
    vlan.newVlanId = OFDPA_VID_PRESENT | (uint16_t)vid;
    entry.flowData.vlanFlowEntry = vlan;
    return entry;
}

static sai_status_t ofdpa_sai_add_untagged_vlan_flow_entry(int vid, int port) {
    ofdpaFlowEntry_t entry = _ofdpa_sai_create_vlan_flow_entry(vid, port, false);
    return ofdpa_err2sai_status(ofdpaFlowAdd(&entry));
}

static sai_status_t ofdpa_sai_add_tagged_vlan_flow_entry(int vid, int port) {
    ofdpaFlowEntry_t entry = _ofdpa_sai_create_vlan_flow_entry(vid, port, true);
    return ofdpa_err2sai_status(ofdpaFlowAdd(&entry));
}

static sai_status_t ofdpa_sai_add_vlan_flow_entry(int vid, int port, bool tagged) {
    ofdpaFlowEntry_t entry;
    sai_status_t err;
    entry = _ofdpa_sai_create_vlan_flow_entry(vid, port, true);
    err = ofdpa_err2sai_status(ofdpaFlowAdd(&entry));
    if ( tagged || err != SAI_STATUS_SUCCESS ) {
        return err;
    }
    entry = _ofdpa_sai_create_vlan_flow_entry(vid, port, false);
    return ofdpa_err2sai_status(ofdpaFlowAdd(&entry));
}

static sai_status_t ofdpa_sai_delete_untagged_vlan_flow_entry(int vid, int port) {
    ofdpaFlowEntry_t entry = _ofdpa_sai_create_vlan_flow_entry(vid, port, false);
    return ofdpa_err2sai_status(ofdpaFlowDelete(&entry));
}

static sai_status_t ofdpa_sai_delete_tagged_vlan_flow_entry(int vid, int port) {
    ofdpaFlowEntry_t entry = _ofdpa_sai_create_vlan_flow_entry(vid, port, true);
    return ofdpa_err2sai_status(ofdpaFlowDelete(&entry));
}

static sai_status_t ofdpa_sai_delete_vlan_flow_entry(int vid, int port, bool tagged) {
    ofdpaFlowEntry_t entry;
    sai_status_t err;
    if ( !tagged ) {
        entry = _ofdpa_sai_create_vlan_flow_entry(vid, port, false);
        err = ofdpa_err2sai_status(ofdpaFlowDelete(&entry));
        if ( err != SAI_STATUS_SUCCESS ) {
            return err;
        }
    }
    entry = _ofdpa_sai_create_vlan_flow_entry(vid, port, true);
    return ofdpa_err2sai_status(ofdpaFlowDelete(&entry));
}

static sai_status_t ofdpa_sai_add_unicast_routing_flow(int gid, int vrf, bool packet_in, int dst_v4, int mask_v4) {
    ofdpaFlowEntry_t entry;
    ofdpaUnicastRoutingFlowEntry_t unicast;
    ofdpaFlowEntryInit(OFDPA_FLOW_TABLE_ID_UNICAST_ROUTING, &entry);
    memset(&unicast, 0, sizeof(ofdpaUnicastRoutingFlowEntry_t));
    if ( vrf > 0 ) {
        unicast.match_criteria.vrf = (uint16_t)vrf;
        unicast.match_criteria.vrfMask = OFDPA_VRF_VALUE_MASK;
    }
    unicast.match_criteria.etherType = 0x0800;
    unicast.match_criteria.dstIp4 = (in_addr_t)htonl((uint32_t)dst_v4);
    unicast.match_criteria.dstIp4Mask = (in_addr_t)htonl((uint32_t)mask_v4);
    unicast.gotoTableId = OFDPA_FLOW_TABLE_ID_ACL_POLICY;
    if ( packet_in == true ) {
        unicast.outputPort = OFDPA_PORT_CONTROLLER;
    } else {
        unicast.groupID = (uint32_t)gid;
    }
    entry.flowData.unicastRoutingFlowEntry = unicast;
    return ofdpa_err2sai_status(ofdpaFlowAdd(&entry));
}

sai_status_t sai_create_hostif(
        _Out_ sai_object_id_t *hif_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list){
    int i, fd, vid, ofdpa_idx;
    sai_object_type_t obj_type;
    sai_object_id_t port_oid;
    char *name = NULL;
    ofdpaMacAddr_t mac;
    sai_status_t err;
    ofdpa_sai_port_t *port = NULL;

    for( i = 0; i < attr_count; i++ ) {
        switch (attr_list[i].id) {
        case SAI_HOSTIF_ATTR_TYPE:
            if ( attr_list[i].value.s32 != SAI_HOSTIF_TYPE_NETDEV ) {
                return SAI_STATUS_NOT_SUPPORTED;
            }
            break;
        case SAI_HOSTIF_ATTR_OBJ_ID:
            port_oid = attr_list[i].value.oid;
            obj_type = sai_object_type_query(port_oid);
            if ( obj_type != SAI_OBJECT_TYPE_PORT ) {
                return SAI_STATUS_NOT_SUPPORTED;
            }
            port = get_ofdpa_sai_port_by_port_oid(port_oid);
            if ( port == NULL ) {
                return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
            }
            break;
        case SAI_HOSTIF_ATTR_NAME:
            name = (char *)attr_list[i].value.chardata;
            break;
        }
    }

    if ( name == NULL ) {
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    fd = tap_alloc(name);
    if ( fd < 0 ) {
        return SAI_STATUS_FAILURE;
    }

    port->hostif_oid = *hif_id;
    port->fd = fd;

    get_mac_address(name, &mac);

    port->mac = mac;

    vid = port->vid;
    ofdpa_idx = port->index;

    err = ofdpa_sai_add_mac_termination_flow(vid, 0, mac);
    if ( err != SAI_STATUS_SUCCESS ) {
        printf("failed to add mac termination flow: vid: %d, port_idx: %d\n", vid, port->i);
        return err;
    }

    err = ofdpa_sai_add_acl_policy_flow(ofdpa_idx, 0x0806, NULL, &mac, 10);
    if ( err != SAI_STATUS_SUCCESS ) {
        printf("failed to add acl policy flow: port: %d\n", port->i);
    }

    err = ofdpa_sai_add_l2_interface_group(vid, ofdpa_idx, true);
    if ( err != SAI_STATUS_SUCCESS ) {
        printf("failed to add l2 interface group: %d %d\n", vid, port->i);
    }

    err = ofdpa_sai_add_vlan_flow_entry(vid, ofdpa_idx, false);
    if ( err != SAI_STATUS_SUCCESS ) {
        printf("failed to add vlan flow entry: %d %d\n", vid, port->i);
    }

    pthread_create(&port->pt, NULL, ofdpa_sai_pkt_send_loop, (void *)port);

    printf("send loop thread created for %s\n", name);

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_remove_hostif(
        _In_ sai_object_id_t hif_id){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_set_hostif_attribute(
        _In_ sai_object_id_t hif_id,
        _In_ const sai_attribute_t *attr){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_get_hostif_attribute(
        _In_ sai_object_id_t hif_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_create_hostif_table_entry(
        _Out_ sai_object_id_t *hif_table_entry,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_remove_hostif_table_entry(
        _In_ sai_object_id_t hif_table_entry){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_set_hostif_table_entry_attribute(
        _In_ sai_object_id_t hif_table_entry,
        _In_ const sai_attribute_t *attr){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_get_hostif_table_entry_attribute(
        _In_ sai_object_id_t hif_table_entry,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_create_hostif_trap_group(
        _Out_ sai_object_id_t *hostif_trap_group_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_remove_hostif_trap_group(
        _In_ sai_object_id_t hostif_trap_group_id){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_set_hostif_trap_group_attribute(
        _In_ sai_object_id_t hostif_trap_group_id,
        _In_ const sai_attribute_t *attr){
}

sai_status_t sai_get_hostif_trap_group_attribute(
        _In_ sai_object_id_t hostif_trap_group_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list){
    int i;
    printf("trap: %lx, %lx\n", hostif_trap_group_id, g_default_trap_group);
    if ( hostif_trap_group_id != g_default_trap_group ) {
        return SAI_STATUS_NOT_SUPPORTED;
    }
    for ( i = 0; i < attr_count; i++ ){
        switch (attr_list[i].id) {
        case SAI_HOSTIF_TRAP_GROUP_ATTR_POLICER:
            attr_list[i].value.oid = g_policer;
            break;
        }
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_create_hostif_trap(
        _Out_ sai_object_id_t *hostif_trap_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_remove_hostif_trap(
        _In_ sai_object_id_t hostif_trap_id){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_set_hostif_trap_attribute(
        _In_ sai_object_id_t hostif_trap_id,
        _In_ const sai_attribute_t *attr){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_get_hostif_trap_attribute(
        _In_ sai_object_id_t hostif_trap_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_create_hostif_user_defined_trap(
        _Out_ sai_object_id_t *hostif_user_defined_trap_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_remove_hostif_user_defined_trap(
        _In_ sai_object_id_t hostif_user_defined_trap_id){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_set_hostif_user_defined_trap_attribute(
        _In_ sai_object_id_t hostif_user_defined_trap_id,
        _In_ const sai_attribute_t *attr){
    return SAI_STATUS_SUCCESS;
}


sai_status_t sai_get_hostif_user_defined_trap_attribute(
        _In_ sai_object_id_t hostif_user_defined_trap_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_recv_hostif_packet(
        _In_ sai_object_id_t hif_id,
        _Out_ void *buffer,
        _Inout_ sai_size_t *buffer_size,
        _Inout_ uint32_t *attr_count,
        _Out_ sai_attribute_t *attr_list){
    return SAI_STATUS_SUCCESS;
}


sai_status_t sai_send_hostif_packet(
        _In_ sai_object_id_t hif_id,
        _In_ void *buffer,
        _In_ sai_size_t buffer_size,
        _In_ uint32_t attr_count,
        _In_ sai_attribute_t *attr_list){
    return SAI_STATUS_SUCCESS;
}


sai_hostif_api_t hostif_api = {
    .create_hostif = sai_create_hostif,
    .remove_hostif = sai_remove_hostif,
    .set_hostif_attribute = sai_set_hostif_attribute,
    .get_hostif_attribute = sai_get_hostif_attribute,
    .create_hostif_table_entry = sai_create_hostif_table_entry,
    .remove_hostif_table_entry = sai_remove_hostif_table_entry,
    .set_hostif_table_entry_attribute = sai_set_hostif_table_entry_attribute,
    .get_hostif_table_entry_attribute = sai_get_hostif_table_entry_attribute,
    .create_hostif_trap_group = sai_create_hostif_trap_group,
    .remove_hostif_trap_group = sai_remove_hostif_trap_group,
    .set_hostif_trap_group_attribute = sai_set_hostif_trap_group_attribute,
    .get_hostif_trap_group_attribute = sai_get_hostif_trap_group_attribute,
    .create_hostif_trap = sai_create_hostif_trap,
    .remove_hostif_trap = sai_remove_hostif_trap,
    .set_hostif_trap_attribute = sai_set_hostif_trap_attribute,
    .get_hostif_trap_attribute = sai_get_hostif_trap_attribute,
    .create_hostif_user_defined_trap = sai_create_hostif_user_defined_trap,
    .remove_hostif_user_defined_trap = sai_remove_hostif_user_defined_trap,
    .set_hostif_user_defined_trap_attribute = sai_set_hostif_user_defined_trap_attribute,
    .get_hostif_user_defined_trap_attribute = sai_get_hostif_user_defined_trap_attribute,
    .recv_hostif_packet = sai_recv_hostif_packet,
    .send_hostif_packet = sai_send_hostif_packet,
};

sai_status_t sai_create_next_hop(
        _Out_ sai_object_id_t *next_hop_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list){
    int i, idx = 0, j, jdx = 0;
    sai_object_id_t oid;
    bool found = false;
    uint32_t ip4 = 0;
    int gid = 0, ref_gid = 0, vid, port, ip;
    ofdpaMacAddr_t src, dst;
    sai_status_t err;

    pthread_mutex_lock(&m);

    for ( i = 0; i < attr_count; i++ ) {
        switch ( attr_list[i].id ) {
        case SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID:
            oid = attr_list[i].value.oid;
            for ( j = 0; j < port_num; j++ ){
                if ( ports[j].router_if_oid == oid ) {
                    found = true;
                    idx = j;
                }
            }
            break;
        case SAI_NEXT_HOP_ATTR_IP:
            ip4 = (uint32_t)attr_list[i].value.ipaddr.addr.ip4;
            break;
        }
    }

    printf("create next hop: %d %d\n", found, ip4);

    if ( found == false || ip4 == 0 ) {
        pthread_mutex_unlock(&m);
        return SAI_STATUS_FAILURE;
    }
    found = false;

    for ( i = 0; i < ports[idx].num_neighbor; i++ ) {
        printf("idx: %d, i: %d, ip: %d, ip4: %d\n", idx, i, ports[idx].neighbors[i].ip, ip4);
        if ( ports[idx].neighbors[i].ip == ip4 ) {
            jdx = i;
            found = true;
        }
    }

    if ( found == false ) {
        pthread_mutex_unlock(&m);
        return SAI_STATUS_FAILURE;
    }

    printf("setting nexthop oid: %lx\n", *next_hop_id);
    ports[idx].neighbors[jdx].oid = *next_hop_id;

    vid = ports[idx].vid;
    port = ports[idx].index;
    src = ports[idx].mac;
    dst = ports[idx].neighbors[jdx].mac;
    ip = ports[idx].neighbors[jdx].ip;
    l3_unicast_idx++;

    ref_gid = (int)ofdpa_sai_group_id(OFDPA_GROUP_ENTRY_TYPE_L2_INTERFACE, vid, port, 0);

    printf("ref-gid: %d\n", ref_gid);
    print_mac(src);
    print_mac(dst);

    err = ofdpa_sai_add_l3_unicast_group(vid, src, dst, ref_gid, l3_unicast_idx, &gid);
    if ( err != SAI_STATUS_SUCCESS ) {
        printf("failed to add l3 unicast group: %d\n", vid);
        return SAI_STATUS_FAILURE;
    }

    printf("setting gid: %x\n", gid);
    ports[idx].neighbors[jdx].gid = gid;

    printf("added l3 unciast group\n");

    err = ofdpa_sai_add_unicast_routing_flow(gid, 0, false, (int)ip, (int)0xffffffff);
    if ( err != SAI_STATUS_SUCCESS ) {
        printf("failed to add unicast routing flow: %d\n", gid);
        pthread_mutex_unlock(&m);
        return err;
    }

    printf("added unicast routing flow\n");

    pthread_mutex_unlock(&m);
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_remove_next_hop(
        _In_ sai_object_id_t next_hop_id){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_set_next_hop_attribute(
        _In_ sai_object_id_t next_hop_id,
        _In_ const sai_attribute_t *attr){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_get_next_hop_attribute(
        _In_ sai_object_id_t next_hop_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list){
    return SAI_STATUS_SUCCESS;
}

sai_next_hop_api_t next_hop_api = {
    .create_next_hop = sai_create_next_hop,
    .remove_next_hop = sai_remove_next_hop,
    .set_next_hop_attribute = sai_set_next_hop_attribute,
    .get_next_hop_attribute = sai_get_next_hop_attribute,
};

sai_status_t sai_create_neighbor_entry(
        _In_ const sai_neighbor_entry_t *neighbor_entry,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list){
    int i, j, idx, jdx;
    bool found = false;
    ofdpaMacAddr_t mac;

    for ( i = 0; i < port_num; i++ ) {
        if ( ports[i].router_if_oid == neighbor_entry->rif_id ) {
            idx = i;
            found = true;
        }
    }

    if ( found == false ) {
        return SAI_STATUS_FAILURE;
    }


    for ( i = 0; i < attr_count; i++ ) {
        switch (attr_list[i].id) {
        case SAI_NEIGHBOR_ENTRY_ATTR_DST_MAC_ADDRESS:
            for ( j = 0; j < OFDPA_MAC_ADDR_LEN; j++ ) {
                mac.addr[j] = attr_list[i].value.mac[j];
            }
        }
    }

    printf("create neighbor entry: %d %d %d\n", idx, jdx, neighbor_entry->ip_address.addr.ip4);

    ports[idx].neighbors[jdx].mac = mac;
    ports[idx].neighbors[jdx].ip = (uint32_t)neighbor_entry->ip_address.addr.ip4;

    jdx = ports[idx].num_neighbor++;

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_remove_neighbor_entry(_In_ const sai_neighbor_entry_t *neighbor_entry){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_set_neighbor_entry_attribute(
        _In_ const sai_neighbor_entry_t *neighbor_entry,
        _In_ const sai_attribute_t *attr){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_get_neighbor_entry_attribute(
        _In_ const sai_neighbor_entry_t *neighbor_entry,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list){
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_remove_all_neighbor_entries(_In_ sai_object_id_t switch_id){
    return SAI_STATUS_SUCCESS;
}

sai_neighbor_api_t neighbor_api = {
    .create_neighbor_entry = sai_create_neighbor_entry,
    .remove_neighbor_entry = sai_remove_neighbor_entry,
    .set_neighbor_entry_attribute = sai_set_neighbor_entry_attribute,
    .get_neighbor_entry_attribute = sai_get_neighbor_entry_attribute,
    .remove_all_neighbor_entries = sai_remove_all_neighbor_entries,
};

sai_object_type_t sai_object_type_query(_In_ sai_object_id_t sai_object_id) {
    return (sai_object_type_t)(sai_object_id >> OBJECT_TYPE_SHIFT);
}

sai_status_t sai_api_initialize(_In_ uint64_t flags, _In_ const service_method_table_t *services) {
    ofdpa_sai_vlan_t *v;
    pthread_mutex_init(&m, NULL);
    v = malloc(sizeof(ofdpa_sai_vlan_t));
    memset(v, 0, sizeof(ofdpa_sai_vlan_t));
    vlans = v;

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_api_query(_In_ sai_api_t sai_api_id, _Out_ void **api_method_table) {
    switch (sai_api_id) {
    case SAI_API_SWITCH:
        *api_method_table = &switch_api;
        break;
    case SAI_API_PORT:
        *api_method_table = &port_api;
        break;
    case SAI_API_VLAN:
        *api_method_table = &vlan_api;
        break;
    case SAI_API_ROUTE:
        *api_method_table = &route_api;
        break;
    case SAI_API_ROUTER_INTERFACE:
        *api_method_table = &router_interface_api;
        break;
    case SAI_API_ACL:
        *api_method_table = &acl_api;
        break;
    case SAI_API_HOSTIF:
        *api_method_table = &hostif_api;
        break;
    case SAI_API_BRIDGE:
        *api_method_table = &bridge_api;
        break;
    case SAI_API_NEXT_HOP:
        *api_method_table = &next_hop_api;
        break;
    case SAI_API_NEIGHBOR:
        *api_method_table = &neighbor_api;
        break;
    default:
        printf("no api: %d\n", sai_api_id);
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_api_uninitialize(void) {
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_log_set(_In_ sai_api_t sai_api_id, _In_ sai_log_level_t log_level) {
    return SAI_STATUS_SUCCESS;
}

sai_object_id_t sai_switch_id_query(_In_ sai_object_id_t sai_object_id) {
    return g_switch_id;
}

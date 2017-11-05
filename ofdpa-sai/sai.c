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

#include <pthread.h>

#define OBJECT_TYPE_SHIFT 48

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


// FIXME use lane info to get matching with actual hardware
static uint32_t port_cnt = 0;

static pthread_t pt;

typedef struct {
    int index;
    char name[32];
    int fd;
    sai_object_id_t port_oid;
    sai_object_id_t hostif_oid;
    pthread_t pt;
} ofdpa_sai_port_t;

static ofdpa_sai_port_t *ports;

sai_status_t sai_create_vlan(
        _Out_ sai_object_id_t *vlan_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list){
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
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_remove_vlan_member(_In_ sai_object_id_t vlan_member_id){
    return SAI_STATUS_SUCCESS;
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
    *rif_id = g_router_intf;
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
    for( i = 0; i < attr_count; i++ ) {
        printf("create port attr: %d\n", attr_list[i].id);
    }
    ports[port_cnt++].port_oid = *port_id;
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_remove_port(_In_ sai_object_id_t port_id) {
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_set_port_attribute(_In_ sai_object_id_t port_id, _In_ const sai_attribute_t *attr) {
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_get_port_attribute(_In_ sai_object_id_t port_id, _In_ uint32_t attr_count, _Inout_ sai_attribute_t *attr_list) {
    int i, count;
    printf("port get attr count: %d\n", attr_count);
    for( i = 0; i < attr_count; i++ ) {
        printf("port get attr: %d\n", attr_list[i].id);
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
    uint32_t i, next;
    OFDPA_ERROR_t err;
    while(1){
        err = ofdpaPortNextGet(i, &next);
        if ( err != OFDPA_E_NONE ) {
            break;
        }
        i = next;
    }
    port_num = i;
    ports = malloc(port_num * sizeof(ofdpa_sai_port_t));

    for(i = 0; i < port_num; i++) {
        ofdpa_buffdesc buf;
        ports[i].index = i;
        buf.size = 32;
        buf.pstart = ports[i].name;
        ofdpaPortNameGet(i, &buf);
    }

    return SAI_STATUS_SUCCESS;
}

void *ofdpa_sai_pkt_recv_loop(void *arg){
    ofdpaPacket_t pkt;
    pkt.pktData.size = max_pkt_size;
    pkt.pktData.pstart = (char *)malloc(max_pkt_size);
    OFDPA_ERROR_t err;

    while (1) {
        int fd;
        err = ofdpaPktReceive(NULL, &pkt);
        if ( err != OFDPA_E_NONE ) {
            printf("Receive fail: %d\n", err);
            return NULL;
        }
        printf("PKT: in-port: %d, reason: %d, table-id: %d\n", pkt.inPortNum, pkt.reason, pkt.tableId);

        if ( (fd = ports[pkt.inPortNum].fd) > 0 ) {
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
    int size;

    while (1) {
        size = read(port->fd, buf.pstart, buf.size);
        if ( size < 0 ) {
            return NULL;
        }
        err = ofdpaPktSend(&buf, 0, port->index, 0);
        if ( err != OFDPA_E_NONE) {
            printf("Send fail: %d\n", err);
            return NULL;
        }
    }
}

sai_status_t sai_create_switch(_Out_ sai_object_id_t* switch_id, _In_ uint32_t attr_count, _In_ const sai_attribute_t *attr_list) {
    int i;
    *switch_id = g_switch_id;
    OFDPA_ERROR_t err;

    printf("switch_id: %lx\n", g_switch_id);

    for(i = 0; i < attr_count; i++){
        printf("attr: %d\n", attr_list[i].id);
    }

    err = ofdpaClientInitialize("sai");
    if ( err != OFDPA_E_NONE ) {
        return SAI_STATUS_FAILURE;
    }

    ofdpa_sai_add_acl_policy_flow(0, 0x0806, "", "ff:ff:ff:ff:ff:ff", "", "")

    enableSourceMacLearning();

    ofdpaClientPktSockBind();

    ofdpaMaxPktSizeGet(&max_pkt_size);

    ofdpa_sai_init_port();

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
        printf("get attr: %d\n", attr_list[i].id);
        switch (attr_list[i].id) {
        case SAI_SWITCH_ATTR_PORT_NUMBER:
            attr_list[i].value.u32 = 1;
            break;
        case SAI_SWITCH_ATTR_PORT_LIST:
            count = attr_list[i].value.objlist.count;
            printf("count: %d\n", count);
            if ( count < 1 ) {
                return SAI_STATUS_BUFFER_OVERFLOW;
            }
            printf("gportid: %lx\n", g_port_id);
            attr_list[i].value.objlist.list[0] = g_port_id;
            attr_list[i].value.objlist.count = 1;
            break;
        case SAI_SWITCH_ATTR_CPU_PORT:
            attr_list[i].value.oid = g_cpu_port_id;
            break;
        case SAI_SWITCH_ATTR_DEFAULT_VIRTUAL_ROUTER_ID:
            printf("gvrid: %lx\n", g_virtual_rid);
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

sai_status_t sai_create_hostif(
        _Out_ sai_object_id_t *hif_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list){
    int i, j, fd, port_idx;
    sai_object_type_t obj_type;
    sai_object_id_t port_oid;
    char *name = NULL;
    bool port_exists = false;
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
            for( j = 0; j < port_num; j++ ) {
                if ( ports[j].port_oid == port_oid ) {
                    port_exists = true;
                    port_idx = j;
                    break;
                }
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

    if ( port_exists == false ) {
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    fd = tap_alloc(name);
    if ( fd < 0 ) {
        return SAI_STATUS_FAILURE;
    }

    ports[port_idx].hostif_oid = *hif_id;
    ports[port_idx].fd = fd;

    pthread_create(&ports[port_idx].pt, NULL, ofdpa_sai_pkt_send_loop, (void *)&ports[port_idx]);

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

sai_object_type_t sai_object_type_query(_In_ sai_object_id_t sai_object_id) {
    printf("object-type-query: %lx\n", sai_object_id);
    return (sai_object_type_t)(sai_object_id >> OBJECT_TYPE_SHIFT);
}

sai_status_t sai_api_initialize(_In_ uint64_t flags, _In_ const service_method_table_t *services) {
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

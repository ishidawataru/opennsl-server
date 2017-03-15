#include <iostream>
#include <cstring>
#include <string>

uint8_t* mac_convert_str_to_bytes(const std::string mac) {
    unsigned char* bytes = new unsigned char[6];
    int values[6];
    int i;
    char * x;
    if(6 == sscanf( mac.c_str(), "%x:%x:%x:%x:%x:%x%c", &values[0], &values[1], &values[2], &values[3], &values[4], &values[5], x))
    {
        /* convert to uint8_t */
        for( i = 0; i < 6; ++i ) {
            bytes[i] = (unsigned char) values[i];
        }
    } else if (6 == sscanf( mac.c_str(), "%x-%x-%x-%x-%x-%x%c", &values[0], &values[1], &values[2], &values[3], &values[4], &values[5], x))
    {
        /* convert to uint8_t */
        for( i = 0; i < 6; ++i ) {
            bytes[i] = (unsigned char) values[i];
        }
    } else {
        for( i = 0; i < 6; ++i ){
            bytes[i] = (unsigned char) 0x00;
        }
    }
    return bytes;
}

std::string mac_convert_bytes_to_sting(const uint8_t baseMac[6]) {
    char tmp_mac[18];
    sprintf( tmp_mac, "%x:%x:%x:%x:%x:%x", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
    std::string mac = tmp_mac;
    return mac;
}
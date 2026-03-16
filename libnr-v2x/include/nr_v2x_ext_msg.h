/*
 * Copyright (c) 2025 Korea Intelligent Automotive Parts Promotion Institute (KIAPI). All rights reserved.
 * Project : KIAPI 5G-NR-V2X
 * Author: WooChang Seo (wcseo@kiapi.or.kr) 
 * Date: 2025-08-29
 */ 


#ifndef _NR_V2X_KIAPI_MSG_H_ 
#define _NR_V2X_KIAPI_MSG_H_
 
#include <string> 
#include <cpp-framework/common/endian.h>
#include <stdint.h> 

// Little Endian base format ! 


#define KIAPI_MAGIC_NUMBER 0x4B494150 // KIAP
  
typedef enum{
    KIAPI_RTT_NUMBER = 0x4B525454, // KRTT 
    KIAPI_RTT_ACK_NUMBER = 0x4B525455, // KRTT (ACK) 
    KIAPI_DEV_POS_DATA = 0x4B525450, // KRTP (POSITION) 
    // KIAPI_OBU_RTT_NUMBER = 0x4B525456
}nr_v2x_kiapi_msg_tag_t;
     

struct nr_v2x_dev_pos_data_t{

    uint64_t seq;      
    uint64_t tx_tick; 
    uint32_t dev_id;    
    int lat;            
    int lon;           
    int elev;         
    uint16_t heading;  
    int speed;        

} __attribute__((packed));




#endif
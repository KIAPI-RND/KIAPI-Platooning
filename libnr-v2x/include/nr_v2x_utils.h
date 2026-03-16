/*
 * Copyright (c) 2025 Korea Intelligent Automotive Parts Promotion Institute (KIAPI). All rights reserved.
 * Project : KIAPI 5G-NR-V2X
 * Author: WooChang Seo (wcseo@kiapi.or.kr) 
 * Date: 2025-08-29
 */ 


#ifndef _NR_V2X_UTILS_H_
#define _NR_V2X_UTILS_H_

#include <nr-v2x/nr_v2x_msg.h>
#include <vector>
#include <stdint.h>
  
#include <stdint.h>
#include <stddef.h>

uint32_t CLI_UTIL_GetCrc32(const uint8_t* pBuf, size_t unSize);
 
uint64_t convert_nr_status_us(uint64_t value);

#endif
/*
 * Copyright (c) 2025 Korea Intelligent Automotive Parts Promotion Institute (KIAPI). All rights reserved.
 * Project :  KIAPI PQ-UTILS 
 * Author: WooChang Seo (wcseo@kiapi.or.kr) 
 * Date: 2025-08-29
 */ 
#ifndef _PQ_CONFIG_H_
#define _PQ_CONFIG_H_

#include <string>
#include <stdint.h>

struct pq_db_config_t
{
    std::string ip = "127.0.0.1";
    uint32_t port = 5432;
    std::string id = "posgtres";
    std::string pw = "posgtres";
    std::string name = "postgre";
};

#include <vector> 
std::vector<pq_db_config_t> pq_db_config_parse(const std::string &json_path); 

#endif
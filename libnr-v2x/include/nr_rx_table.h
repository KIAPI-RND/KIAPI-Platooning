/*
 * Copyright (c) 2025 Korea Intelligent Automotive Parts Promotion Institute (KIAPI). All rights reserved.
 * Project : KIAPI 5G-NR-V2X
 * Author: WooChang Seo (wcseo@kiapi.or.kr) 
 * Date: 2025-08-29
 */ 

#ifndef _NR_V2X_RX_TABLE_H_
#define _NR_V2X_RX_TABLE_H_

#include <map>
#include <vector>
#include <mutex>

struct nr_rx_info_t
{
    uint64_t tick;
    uint32_t id;
    uint32_t latency;
    uint32_t per;
    uint32_t prr;
    uint32_t rcpi;
    uint32_t rssi;
    uint32_t cnt;
};

class nr_rx_table{ 
public: 

    nr_rx_table();
    ~nr_rx_table(); 
    
    void push(int dev_id, int src_id, int rssi = -1, int rcpi= -1, int latency= -1, int prr = -1); 
    int get(int dev_id, std::map<int, nr_rx_info_t> *out); // get 사용 시, 전역 lock, unlock 처리 필요
    void reset();
 
    void lock();
    void unlock();

private:
    std::mutex mutex;
    std::map<int, std::map<int, nr_rx_info_t>> table;
};

#endif
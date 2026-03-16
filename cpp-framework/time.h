/*
 * Project : KIAPI cpp-framework
 * Author: WooChang Seo (wcseo@kiapi.or.kr) 
 * Date: 2025-10-04
 */  

#ifndef _CPP_COMMON_TIME_H_
#define _CPP_COMMON_TIME_H_

#include <stdint.h>
#include "cpp-framework/thread/local_lock.h"

uint64_t get_epoch_time_msec(); 
uint64_t get_epoch_time_usec();

uint64_t set_epoch_time_msec(uint64_t epoch_time);
uint64_t sleep_for(uint32_t msec);

std::tm get_tm();
std::string get_time_string(std::string format = "%Y%m%d_%H%M%S");
std::string get_time_string(uint64_t tick, std::string format = "%Y%m%d_%H%M%S");
  
std::time_t timegm_custom(std::tm *tm, bool offset);
uint64_t timestamp_to_uint64(const std::string &timestamp_str, bool offset);

//  std::tm localTime = *std::localtime(&now_c);

int64_t toEpochUTC(int year, int month, int day,
                   int hour, int min, int sec, int msec);

class tick_timer{

public: 
    tick_timer(uint64_t duration = 0);
    ~tick_timer();

    void set(uint64_t duration,bool immediately = false);
    bool is_enable();
    bool timeout(bool update = true);
    uint64_t left();
    void set_margin(int64_t margin); 
    void set_start_time(int64_t tick);
    int64_t elapsed();

private:
    std::mutex lock;
    uint64_t start = 0;
    uint64_t now = 0;
    uint64_t next = 0;
    uint64_t duration = 0; 
    bool update(); 
    bool over();
};






#endif
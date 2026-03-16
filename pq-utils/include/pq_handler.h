/*
 * Copyright (c) 2025 Korea Intelligent Automotive Parts Promotion Institute (KIAPI). All rights reserved.
 * Project : KIAPI 5G-NR-V2X
 * Author: WooChang Seo (wcseo@kiapi.or.kr) 
 * Date: 2025-08-29
 */ 

#ifndef _PG_HANDLER_H_
#define _PG_HANDLER_H_
 
//https://libpqxx.readthedocs.io/6.4/

#include <string> 
#include <pqxx/pqxx>
#include <cpp-framework/thread/thread_handler.h>
#include <cpp-framework/common/time.h>
// #include <queue>
#include <vector>
#include <pq-utils/pq_config.h>

#define MAX_QUERY_QUE_SIZE 1000 

struct pq_result_t{
    bool done = false; 
    pqxx::result res;
    long tick = 0;
};

typedef std::function<void(int,pq_result_t)> on_pq_progress; 
typedef std::function<void(pqxx::work &worker, const std::string &query, std::vector<std::string> hex)> on_pq_stream_progress;

struct pq_query_event_t{
    std::string query = "";
    tick_timer *tick;
    int index = 0;
    on_pq_progress handler = nullptr;
    on_pq_stream_progress stream_requester = nullptr;
    std::vector<std::string> stream_data;
};    
 
class pq_handler{

public:
    pq_handler(uint thread_cnt = 1);
    ~pq_handler();

    void set_debug(bool set);

    bool connect(const pq_db_config_t &conf);
  
    bool close();
    bool add_polling(int index, const std::string query, uint32_t msec,
                     on_pq_progress func = nullptr);

    template <typename T>
    bool add_polling(int index, const std::string query, uint32_t msec, 
    T *class_type, void (T::*on_pq_progress)(int,pq_result_t))
    { 
        return add_polling(index, query, msec, std::bind(on_pq_progress, class_type, std::placeholders::_1,std::placeholders::_2));
    }

    int push_query(std::string query, on_pq_progress func = nullptr);
    int push_stream_query(std::string query, std::string dat, on_pq_progress func = nullptr);

     
    pqxx::result exec(std::string query); 
    pq_result_t exec(pq_query_event_t* query); 
    pq_result_t exec_stream(pq_query_event_t * query);
   
    template <typename T>
    bool push_query(std::string query,
                     T *class_type, void (T::*on_pq_progress)(int, pq_result_t))
    {
        return push_query(query, std::bind(on_pq_progress, class_type, std::placeholders::_1, std::placeholders::_2));
    }
 

    int queueing_size();
    bool connected();

private:
    bool connect(const std::string &db_name = "postgres",
                 const std::string &user = "postgres",
                 const std::string &password = "postgres",
                 const std::string &host = "127.0.0.1",
                 const int32_t &port = 5432,
                 const std::string &hostaddr = "",
                 const int32_t &connect_timeout = 10,
                 const std::string &options = "",
                 const std::string &sslmode = "",
                 const std::string &application_name = "");
  
    bool enable = false; 
    bool debug = true;

    bool running = false;
    pqxx::connection *conn = nullptr;
    pqxx::work *worker = nullptr;

    pq_db_config_t conn_info;
    
    std::vector<thread_handler *>threads; 

    std::mutex lock;
    std::vector<pq_query_event_t> polling;

    void progress_routine(void *arg);  
    void exec(const std::vector<pq_query_event_t> &query); 

    bool done = false;

    std::vector<pq_query_event_t> query_que;
    std::vector<pq_query_event_t> stream_que;

    thread_handler main_srv; 
    void progress_main_routine(void *arg); // connection 관리 처리함

};
 


#endif
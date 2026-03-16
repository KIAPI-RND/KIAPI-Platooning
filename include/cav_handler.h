#ifndef _CAV_HANDLER_H_
#define _CAV_HANDLER_H_

// 자율주행차량 정보 연계 Class 정의 
#include <cpp-framework/sock/sock_handler.hpp>
#include <nr-v2x/keti/db_v2x_platooning.h>
#include <nr-v2x/nr_v2x.h>

class cav_handler : public sock_handler_event
{

public:
    cav_handler(int port);
    ~cav_handler();
  
    // 수신된 LV 메시지 자율주행차량으로 전송 
    bool forward_lv_msg(uint64_t timestamp,uint32_t dev_id, uint32_t src, uint32_t msg_seq, const DB_V2X_PLATOONING_LV_T &msg, const v2x_parameter_field_t &param);
    // 수신된 FV 메시지 자율주행차량으로 전송
    bool forward_fv_msg(uint64_t timestamp,uint32_t dev_id, uint32_t src, uint32_t msg_seq, const DB_V2X_PLATOONING_FV_T &msg, const v2x_parameter_field_t &param);
    
protected:
    int on_sock_receive(int index, io_struct &sock);
    int on_sock_connection(int index, io_struct &sock, bool connected);
 
    sock_handler sock;

    int seq = 0;
  
};


#endif
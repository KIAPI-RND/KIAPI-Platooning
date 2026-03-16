#ifndef _SERVER_SERVICE_H_
#define _SERVER_SERVICE_H_
 
#include <nr-v2x/keti/db_v2x_platooning.h>
#include <nr-v2x/nr_v2x_msg.h>
 
typedef enum{
    PLATOONING_SERVER_LV_MSG = 1,
    PLATOONING_SERVER_FV_MSG = 2
}PLATOONING_SERVER_MSG_LIST;
 
struct platooning_rx_info_t
{
    uint32_t rcpi;
    uint32_t rssi;
    uint32_t per;
    uint32_t prr;
    uint32_t latency;
}__attribute__((__packed__)) ;

struct platooning_server_lv_msg_t
{
    nr_v2x_msg_header_t header;
    uint64_t timestamp;
    uint32_t src_dev_id;
    uint32_t rx_dev_id;
    DB_V2X_PLATOONING_LV_T msg;
    platooning_rx_info_t rx;
}__attribute__((__packed__));

struct platooning_server_fv_msg_t{
    nr_v2x_msg_header_t header;
    uint64_t timestamp;
    uint32_t src_dev_id;
    uint32_t rx_dev_id;
    DB_V2X_PLATOONING_FV_T msg;
    platooning_rx_info_t rx;
}__attribute__((__packed__)) ; 



class server_handler : public sock_handler_event {

public:
    server_handler(uint32_t port);
    ~server_handler();
    void set_db(pq_db_config_t config);

protected:
    int on_sock_receive(int index, io_struct &sock);
    int on_sock_connection(int index, io_struct &sock, bool connected);
}


#endif
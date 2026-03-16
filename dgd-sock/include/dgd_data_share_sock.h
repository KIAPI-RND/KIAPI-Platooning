#ifndef _DGD_DATA_SHARE_SOCK_H_
#define _DGD_DATA_SHARE_SOCK_H_

#include <dgd-sock/dgd_broker_sock.h> 
#include <nmeaparse/GPSFix.h> 
#include <cpp-framework/shape/hdmap_builder.h>
#include <dgd-sock/data/data.h>

// DATA SHARE SOCK 
// USE CASE, 
// 1. CAN으로부터 수집된 데이터 공유
// 2. GPS 정보 공유
// 3. CAN으로 전송 명령 처리 
// 4. TODO..........
// Request <-----> Ack 메시지로 구현되어야함

class dgd_data_share_sock_server_event{
public: 
    virtual bool on_request_tx_can(uint32_t psid, io_struct &sock, const dgd_sock_msg_req_can_tx_t &msg) = 0;
};

class dgd_data_share_sock_client_event{
public:
    virtual bool on_can_raw_data(uint32_t psid, io_struct &sock, const dgd_sock_msg_can_raw_data_t &msg, char *ptr) = 0;
    virtual bool on_dgd_gps_data(uint32_t psid, io_struct &sock, const dgd_gps_data_t &msg) = 0;
    virtual bool on_dgd_link_data(uint32_t psid, io_struct &sock, const geojson_link_data_msg_t &msg) = 0;
    virtual bool on_dgd_cvib_data(uint32_t psid, io_struct &sock, const dgd_sock_msg_cvib_data_t &msg) = 0;
    virtual bool on_dgd_vc_data(uint32_t psid, io_struct &sock, const data_vehicle_status_t &payload) = 0;
};  
 
class dgd_data_share_sock : public dgd_broker_sock , dgd_broker_sock_event_handler{

public:
    dgd_data_share_sock(bool server,const std::string &ip = "127.0.0.1",int psid =DGD_SOCK_PSID_DATA_SHARE,std::string name = "");
    ~dgd_data_share_sock();

    void set_container(dgd_data_share_sock_server_event *container);
    void set_container(dgd_data_share_sock_client_event *container);
 
    // Vehicle Data 정보 전송 (Parsed 된 정보)
    bool request_tx_vc_data(const data_vehicle_status_cache_t &data);
    // GPS 정보 전송 (Parsed 된 정보) 
    bool request_tx_gps_data(nmea::GPSFix &fix);  
    bool request_tx_gps_data(const dgd_gps_data_t &msg);
 
    // Link 정보 전송
    bool request_tx_link_data(hdmap_link_data_t *link , int fd = -1);
    // cvib 정보 전송
    bool request_tx_cvib_data(int id, int dist, long tick, int tick_error, int stop_line, std::vector<dgd_sock_msg_cvib_signal_data_t> signal,int ref_dir = -1, int fd = -1);
 
    int client_count();

    // CAN 전송 응답 메시지 정보 // ACK으로 대체함
    bool request_tx_can(uint8_t if_index, uint32_t can_id, uint8_t dlc, std::string payload);
    int update_can_data(uint32_t ifnum,uint64_t tick, can_frame can);  
    bool request_tx_can_raw_data();

 
protected:
    bool on_dgd_sign(uint32_t psid, io_struct &sock,const std::string &key);
    bool on_dgd_disconnect(uint32_t psid, io_struct &sock);
    int on_dgd_message_receive(uint32_t psid, io_struct &sock, const dgd_sock_msg_header_t &header, const std::string &payload);

private:
    bool on_request_tx_can(uint32_t psid, io_struct &sock, const dgd_sock_msg_header_t &header, const dgd_sock_msg_req_can_tx_t &payload);
    bool on_can_raw_data(uint32_t psid, io_struct &sock, const dgd_sock_msg_can_raw_data_t &payload, char *ptr);
    bool on_dgd_gps_data(uint32_t psid, io_struct &sock, const dgd_gps_data_t &payload);
    bool on_dgd_vc_data(uint32_t psid, io_struct &sock, const data_vehicle_status_t &payload);
    bool on_dgd_link_data(uint32_t psid, io_struct &sock, const geojson_link_data_msg_t &payload);
    bool on_dgd_cvib_data(uint32_t psid, io_struct &sock,const dgd_sock_msg_cvib_data_t &payload);

    dgd_data_share_sock_client_event *client_container = nullptr;
    dgd_data_share_sock_server_event *server_container = nullptr;
 
    dgd_broker_service *sock = nullptr;  
    uint16_t seq = 0;

    int psid = -1;

    std::mutex can_map_lock;
    
    std::map<uint32_t, std::map<uint32_t, can_data_frame>> can_map;
  
};


#define D2X_SHARED_MEM_VERSION 0   // 00.00.00 기준    
#define D2X_SHARED_STATUS_PATH "/d2x_status"
struct d2x_data_shared_mem
{
    uint32_t version = D2X_SHARED_MEM_VERSION;

    struct connection_state
    {
        bool center = false;
    } conn;  
  
    char reserved[1000]; // 차후 데이터 갱신 시 버전 매칭을 위해 충돌 방지
};

#endif
/*
 * Copyright (c) 2025 Korea Intelligent Automotive Parts Promotion Institute (KIAPI). All rights reserved.
 * Project : KIAPI 5G-NR-V2X
 * Author: WooChang Seo (wcseo@kiapi.or.kr) 
 * Date: 2025-08-29
 */ 


#ifndef _MCU_CONNECTION_H_
#define _MCU_CONNECTION_H_

#include <cpp-framework/sock/sock_handler.hpp> 
#include <cpp-framework/thread/thread_handler.h> 
#include <cpp-framework/common/time.h>
#include <nmeaparse/nmea.h>

#include <nr-v2x/nr_v2x_conf.h>
#include <nr-v2x/tlvc_msg.h>
#include <nr-v2x/nr_v2x_msg.h>
#include <nr-v2x/nr_pps_mng.h>
#include <nr-v2x/nr_v2x_ext_msg.h>

#include <map>
#include <vector>
 

// V2X 메시지 전송 Parameter 정의 
typedef struct v2x_message_field
{
    uint32_t psid;
    std::string data;
} v2x_message_field_t;
 
// V2X 송수신 메시지 Handler Parameter 정의
typedef struct v2x_parameter_field
{

    uint32_t psid = 0;              
    uint64_t timestamp = 0;
    uint8_t cast_mode = 0;
    int sender = -1; // 4byte hex 
    int res = -1; // 알수 없음
    int data_size = -1;
    int dest = -1; // -1 = broadcast 

    // V2X 송수신 RF 채널 정보
    struct v2x_params_t
    { 
        int8_t pwr = 20; // 20 dbm
        uint16_t freq = 5915;  // old 5885;
        uint8_t bw = 20;           // Band 10Mhz , 20Mhz
        uint8_t scs = 30;          // Subcarrier spacing [15, 30, 60] (kHz)
        uint8_t pssch_mcs_idx = 0; // MCS index [7..6] , [5..0]

    } rf;

    // V2X 수신 정보
    struct v2x_params_rx_t
    { 
        int pps = 0; 
        uint8_t rcpi = 0; 
        bool tx_info_set = false;
        bool rx_info_set = false;
        nr_v2x_ext_status_modem_tx_t tx_info;
        nr_v2x_ext_status_modem_rx_t rx_info; 
        uint64_t throughput = 0;
    } rx;
 
    v2x_parameter_field(uint32_t psid = 0, int sender = -1)
    {
        this->timestamp = get_epoch_time_msec();
        this->psid = psid;
        this->sender = sender;
    }

}v2x_parameter_field_t;


/// @brief V2X 메시지 핸들러 클래스 정의 
/// 해당 클래스는 연결되는 V2X 장치 1대에 대한 연결 커넥션 수 만큼 내부 Handler가 관리되어 동작함
/// 클래스 내부 각 장치로부터 메시지 수신 또는 전송에 대한 응답 및 결과가 자동으로 실행되도록 이벤트 핸들러 코드가 포함됨
/// nr_v2X_mnsg_handler을 상속받은 상위클래스에서 이벤트 호출을 재정의(Overwrite)하여 호출되는 이벤트를 구현 필요함
class nr_v2x_mng_handler{

public:
    // 장치 연결 / 해제 이벤트
    virtual bool on_dev_connection(nr_v2x_dev_info_t *dev, const std::string &ip, uint32_t port, bool connection) = 0;
    // V2X 메시지(J2735) 메시지 수신 처리 이벤트 호출 코드
    virtual void on_rx_msg(nr_v2x_dev_info_t *dev, const v2x_parameter_field_t &param, const std::string &msg) = 0;
    // V2X Extensions 메시지 수신 처리 이벤트 호출 코드
    virtual void on_rx_msg_ext(nr_v2x_dev_info_t *dev, const v2x_parameter_field_t &param, const std::vector<v2x_message_field_t> &msg, const std::vector<nr_v2x_ext_status_msg_field_t> &status) = 0;
    // V2X 메시지(J2735) 메시지 전송 응답 호출 코드 (전송 후, 전송 결과 처리 시 사용)
    virtual void on_tx_msg(nr_v2x_dev_info_t *dev, const v2x_parameter_field_t &param, const std::string &msg) = 0;
    // V2X Extensions 메시지 수신 처리 이벤트 호출 코드 (전송 후, 전송 결과 처리 시 사용)
    virtual void on_tx_msg_ext(nr_v2x_dev_info_t *dev, const v2x_parameter_field_t &param, const std::vector<v2x_message_field_t> &msg,
                               const std::vector<nr_v2x_ext_status_msg_field_t> &status) = 0;
    // FTP 업데이트 정보 요청 이벤트 호출
    virtual void on_ftp_conn_req(nr_v2x_dev_info_t *dev, uint32_t psid, uint8_t unit_id, uint32_t link_id) = 0;
};


/// @brief V2X 장치 Handler Class
/// V2X 장치에 대한 기능과 데이터, 인터페이스를 처리하기위한 Class 정의 
/// TCP 통신으로 연결된 5G-NR RSU 또는 OBU 장치를 비롯하여 각 장치마다 접속을 관리하고 이벤트 호출 등 관리
class nr_v2x_mng : public sock_handler_event{

public: 
    nr_v2x_mng(nr_v2x_dev_type_t type,  nr_v2x_mng_handler *container = nullptr , int id = 0); 
    ~nr_v2x_mng();

    void set_debug(bool set);                    // SET : 디버그 메시지 활성/비활성
    bool available(int index = 0);               // GET : 장치 활성 유무 상태 플래그
    void set_ettifos_direct_mode(int index = 0); // SET : 자체 IP Direct 통신 활성
    void set_crc_enable(int index, bool set);    // SET : CRC Check 활성 / 비활성

    bool add_dev_list(std::vector<nr_v2x_dev_info_t> dev);       // TCP 서버로 동작하는 경우, 하위 접속 Client 장치 정보 입력 
    bool set(bool server, const std::string &ip, uint32_t port); // 서비스 활성
    void set_position(int index, nmea::GPSFix fix);  // 연결 장치 위치 정보 갱신 
    bool get_position(int index, nmea::GPSFix &out); // 연결 장치 위치 정보 수신
    void set_dev_id(int index, uint32_t id);         // 연결 장치 Device ID 정보 설정
    void set_rtt_option(int index, bool set, uint32_t size = 100, uint32_t interval = 1000,
                        bool ext = true, uint32_t tansmit_per_period = 2, bool burst_mode = false); // RTT 전송 정보 설정 
    int get_cnt(); // 송수신 메시지 수 정보 조회

    nr_v2x_dev_info_t *get_dev_info(int index = 0, bool lock = false); // 장치 정보 저장 구조체 조회(Pointer)

    int wait_response(nr_v2x_dev_info_t *dev,const std::string &msg, uint32_t id, uint32_t timeout_msec = 1000); // 프로토콜 메시지 명령 실행부  
    int request_wsm_service(int index, v2x_action_type action, uint32_t psid); // WSM 서비스 요청 프로세스 실행

    // J2735 메시지 전송 요청  
    int request_tx_fixed_msg(int index, v2x_parameter_field_t param, const std::string &msg);
    // Extensions 메시지 전송 요청
    int request_tx_extensible_msg(int index, v2x_parameter_field_t param, uint32_t sub_msg_type, const std::string &msg,
                                  bool add_status = false, int target = -1);
    // RTT 메시지 전송 요청 
    int request_tx_rtt_msg(int index, uint32_t psid, uint32_t dev_id, uint32_t dev_type, uint32_t seq, uint32_t interval,
                           uint32_t size, bool bust_mode = false, double lat = 0.0, double lon = 0.0, double elev = 0.0, double speed = 0.0, double heading = 0.0, int dop = 0);
    // RTT Ack 메시지 전송 요청
    int request_tx_rtt_ack_msg(int index, uint32_t psid, const nr_v2x_rtt_base_msg_t &base);
    // Extensions 메시지 수신 결과 전송 요청 (일반적 사용 X) 
    int request_tx_extensible_rx_msg(int index, uint8_t ver, uint8_t rcpi, const v2x_parameter_field_t &param,
                                     const std::vector<v2x_message_field_t> &msg, const std::vector<nr_v2x_ext_status_msg_field_t> &status);

    // FTP 업데이트 접속 요청에 대한 응답 메시지 전송
    int request_tx_ftp_conn_res(int index, uint8_t unit_id, uint32_t link_id, const std::string &ip, uint16_t port,
                                const std::string &id, const std::string pw);

protected:
    int on_sock_receive(int index, io_struct &sock);
    int on_sock_connection(int index, io_struct &sock, bool connected); 
private:
   
    bool build_dev_info();
    bool add_remote_dev(int index);
    std::string parse_msg(const std::string &buffer);
    int reg_dev_connection(int index, io_struct &sock);
    int unreg_dev_connection(int index, io_struct &sock);
    bool progress_v2x_msg(io_struct &sock, const std::string &msg);
    bool crc_check(const std::string &msg);
    uint16_t crc_get(const std::string &msg);

    int parse_tlvc_data(const std::string &msg, tlvc_msg_t &out, bool crc_check_enable = true);
    // nr_v2x_ext_status_msg_field_t build_ext_status(std::string &out, uint8_t device_type, uint8_t tx_rx,uint32_t device_id,uint16_t hw_ver,uint16_t sw_ver,uint64_t timestamp);

    bool on_rx_wsc(uint64_t timestamp, nr_v2x_dev_info_t *user, const std::string &msg);
    bool on_rx_msg(uint64_t timestamp, nr_v2x_dev_info_t *user, const std::string &msg);
    bool on_rx_tx_msg(uint64_t timestamp,nr_v2x_dev_info_t *user, const std::string &msg);
    bool on_rx_ftp_conn_req(uint64_t timestamp,nr_v2x_dev_info_t *user, const std::string &msg);

    bool on_rx_v2x_msg_fixed(nr_v2x_dev_info_t *user,  v2x_parameter_field param , const std::string &payload);
    bool on_rx_v2x_msg_extensible(nr_v2x_dev_info_t *user, v2x_parameter_field param, const std::string &payload);
 
    // nr_v2x_ext_status_msg_field_t parse_status_field(int index, const std::string &status_msg);
 
    bool running = false;
    nr_v2x_mng_handler *container = nullptr;  
    nr_v2x_config_t config; 
    sock_handler sock; 
    std::map<int,std::shared_ptr<sock_handler>> remote;

    bool debug = false;

    struct dev_info_t
    {
        std::mutex lock;
        int index = -1;
        std::map<std::string, uint32_t> reg_ip_list;
        std::map<uint32_t, nr_v2x_dev_info_t> data;
    } dev;
   
    nr_pps_mng pps; 
    thread_handler thread;
    thread_handler scheduler; 

    uint32_t progress_transmit_rtt(); 
    uint32_t progress_scheduler(); 

    void progress(void *argv);
 
    tick_timer debug_tick;
    

};

 


#endif
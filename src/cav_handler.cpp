/*
 * Project : KIAPI-Platooning
 * Author: WooChang Seo (wcseo@kiapi.or.kr) 
 * Date: 2025-08-29
 */  

#include "cav_handler.h" 
#include "cav_msg.h"
#include <cpp-framework/common/time.h>
#include <string>
#include <cstring>
#include <cpp-framework/data/j2735.h>
#include <cpp-framework/text/text_tool.h>
#include <nr-v2x/nr_v2x_utils.h>
   
cav_handler::cav_handler(int port):
sock(this)
{   
    sock.tcp_bind(port);
}

cav_handler::~cav_handler()
{
    sock.tcp_close();
}

bool cav_handler::forward_lv_msg(uint64_t timestamp, uint32_t dev_id, uint32_t src, uint32_t msg_seq, const DB_V2X_PLATOONING_LV_T &msg, const v2x_parameter_field_t &param)
{ 
    std::string tmp;
    tmp.resize(sizeof(connected_vehicle_information_t));
    
    int index = 0;
    for(int i = 0; i < DB_V2X_PT_LV_PATH_PLAN_MAX_LEN ; i++){

        if (msg.stLvPathPlan.adLvLatitude[i] == 0.0 && msg.stLvPathPlan.adLvLongitude[i] == 0.0)
            break;

        cav_path_plane_t path;
        path.lat = (int)(msg.stLvPathPlan.adLvLatitude[i] / 100000.0);
        path.lon = (int)(msg.stLvPathPlan.adLvLongitude[i] / 100000.0);
        tmp.append((char *)&path, sizeof(cav_path_plane_t));
        index++; 
    } 
 
    cav_msg_header_t *header = (cav_msg_header_t *)tmp.c_str();

    header->msg_id = 0x64;
    header->msg_seq = seq++;
    header->time_stamp = get_epoch_time_msec();
    header->msg_length = sizeof(connected_vehicle_information_t) - sizeof(cav_msg_header_t) + ((index) * sizeof(cav_path_plane_t));
 
    connected_vehicle_information_t *ptr = (connected_vehicle_information_t*)tmp.c_str();

    ptr->tx_timestamp = convert_nr_status_us(param.rx.tx_info.base.us_timestamp) / 1000;
    ptr->rx_timestamp = param.timestamp / 1000; 
    ptr->device_id = src;
    ptr->msg_seq = msg_seq;
  
    memcpy(&ptr->vehicle_id, &msg.szLvVehicleId, 10);
    memcpy(&ptr->vehicle_number, &msg.szLvVehicleNum, 20);
  
    ptr->vehicle_type = 1;
    ptr->lat = (int)(msg.dLvLatitude / 100000.0);
    ptr->lon = (int)(msg.dLvLongitude / 100000.0); 
    ptr->elev = 0;
  
    ptr->speed = J2735_DATA::Velocity_MPS::convert(msg.usLvSpeed * J2735_VELOCITY_KMS_TO_MS);
    ptr->heading = J2735_DATA::Heading::convert(msg.usLvHeading);
    
    ptr->accel_lat = J2735_DATA::Acceleation::convert(msg.stLvCan.fLatAccel);
    ptr->accel_lon = J2735_DATA::Acceleation::convert(msg.stLvCan.fLongAccel);
    ptr->yaw = J2735_DATA::Acceleation::convert(msg.stLvCan.fYawRate);
     
    ptr->brake_cylinder = (int)(msg.stLvCan.fBrakeCylinder * 10);
    ptr->sig_left = msg.stLvCan.bTurnLeftEn;
    ptr->sig_right = msg.stLvCan.bTurnRightEn;
    ptr->sig_hazard = msg.stLvCan.bHazardEn;
 
    ptr->num_of_pathplane = index;
   
    return sock.send(tmp) > 0;
}
bool cav_handler::forward_fv_msg(uint64_t timestamp, uint32_t dev_id, uint32_t src, uint32_t msg_seq, const DB_V2X_PLATOONING_FV_T &msg,const v2x_parameter_field_t &param)
{
  
    std::string tmp;
    tmp.resize(sizeof(connected_vehicle_information_t));
    
    int index = 0;
    for(int i = 0; i < DB_V2X_PT_LV_PATH_PLAN_MAX_LEN ; i++){

        if (msg.stFvPathPlan.adFvLatitude[i] == 0.0 && msg.stFvPathPlan.adFvLongitude[i] == 0.0)
            break;

        cav_path_plane_t path;
        path.lat = (int)(msg.stFvPathPlan.adFvLatitude[i] / 100000.0);
        path.lon = (int)(msg.stFvPathPlan.adFvLongitude[i] / 100000.0);
        tmp.append((char *)&path, sizeof(cav_path_plane_t));
        index++;
    } 

    cav_msg_header_t *header = (cav_msg_header_t *)tmp.c_str();

    header->msg_id = 0x64;
    header->msg_seq = seq++;
    header->time_stamp = get_epoch_time_msec();
    header->msg_length = sizeof(connected_vehicle_information_t) - sizeof(cav_msg_header_t) + ((index) * sizeof(cav_path_plane_t));
  
    connected_vehicle_information_t *ptr = (connected_vehicle_information_t*)tmp.c_str();
 
    ptr->tx_timestamp = convert_nr_status_us(param.rx.tx_info.base.us_timestamp) / 1000;
    ptr->rx_timestamp = param.timestamp / 1000; 
    ptr->device_id = src;
    ptr->msg_seq = msg_seq;

    memcpy(&ptr->vehicle_id, &msg.szFvVehicleId, 10);
    memcpy(&ptr->vehicle_number, &msg.szFvVehicleNum, 20);

    ptr->vehicle_type = 2;
    ptr->lat = (int)(msg.dFvLatitude / 100000.0);
    ptr->lon = (int)(msg.dFvLongitude / 100000.0); 
    ptr->elev = 0;

    ptr->speed = J2735_DATA::Velocity_MPS::convert(msg.usFvSpeed * J2735_VELOCITY_KMS_TO_MS);
    ptr->heading = J2735_DATA::Heading::convert(msg.usFvHeading);

    ptr->accel_lat = J2735_DATA::Acceleation::convert(msg.stFvCan.fLatAccel);
    ptr->accel_lon = J2735_DATA::Acceleation::convert(msg.stFvCan.fLongAccel);
    ptr->yaw = J2735_DATA::Acceleation::convert(msg.stFvCan.fYawRate);

    ptr->brake_cylinder = (int)(msg.stFvCan.fBrakeCylinder * 10);
    ptr->sig_left = msg.stFvCan.bTurnLeftEn;
    ptr->sig_right = msg.stFvCan.bTurnRightEn;
    ptr->sig_hazard = msg.stFvCan.bHazardEn;
    
    ptr->num_of_pathplane = index;
 

    return sock.send(tmp) > 0;
}

int cav_handler::on_sock_receive(int index, io_struct &sock)
{ 
    return 0;
}

int cav_handler::on_sock_connection(int index, io_struct &sock, bool connected)
{  
    return 0;
}
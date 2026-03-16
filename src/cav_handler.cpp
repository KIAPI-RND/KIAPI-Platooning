#include "cav_handler.h" 
#include "cav_msg.h"
#include <cpp-framework/common/time.h>
#include <string>
#include <cstring>
#include <cpp-framework/data/j2735.h>
#include <cpp-framework/text/text_tool.h>
#include <nr-v2x/nr_v2x_utils.h>
 
#define DEBUG_OPTION 0

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

    if (DEBUG_OPTION)
    {
        printf("header->msg_id = %d\n", header->msg_id);
        printf("header->msg_seq = %u\n", header->msg_seq);
        printf("header->time_stamp = %lu\n", header->time_stamp);
        printf("header->msg_length = %u\n", header->msg_length);
        printf("payload->tx_timestamp = %lu\n", ptr->tx_timestamp);
        printf("payload->rx_timestamp = %lu\n", ptr->rx_timestamp);
        printf("payload->device_id = %u\n", ptr->device_id);
        printf("payload->msg_seq = %u\n", ptr->msg_seq);
        printf("payload->vehicle_id = %s\n", ptr->vehicle_id);
        printf("payload->vehicle_number = %s\n", ptr->vehicle_number);
        printf("payload->vehicle_type = %d\n", ptr->vehicle_type);
        printf("payload->lat = %d\n", ptr->lat);
        printf("payload->lon = %d\n", ptr->lon);
        printf("payload->elev = %d\n", ptr->elev);
        printf("payload->speed = %d\n", ptr->speed);
        printf("payload->heading = %d\n", ptr->heading);
        printf("payload->accel_lat = %d\n", ptr->accel_lat);
        printf("payload->accel_lon = %d\n", ptr->accel_lon);
        printf("payload->yaw = %d\n", ptr->yaw);
        printf("payload->brake_cylinder = %d\n", ptr->brake_cylinder);
        printf("payload->sig_left = %d\n", ptr->sig_left);
        printf("payload->sig_right = %d\n", ptr->sig_right);
        printf("payload->sig_hazard = %d\n", ptr->sig_hazard);
        printf("payload->num_of_pathplane = %d\n", ptr->num_of_pathplane);
        for (int i = 0; i < ptr->num_of_pathplane; i++)
        {
            printf("payload->pathplane[%d].lat = %d \n", i, ptr->path_plane[i].lat);
            printf("payload->pathplane[%d].lon = %d \n", i, ptr->path_plane[i].lon);
        }

        printf("hex = %s\n", string_to_hex(tmp).c_str());
    }

    return sock.send(tmp) > 0;
}
bool cav_handler::forward_fv_msg(uint64_t timestamp, uint32_t dev_id, uint32_t src, uint32_t msg_seq, const DB_V2X_PLATOONING_FV_T &msg, const v2x_parameter_field_t &param)
{
    return false;
}

int cav_handler::on_sock_receive(int index, io_struct &sock)
{
    return 0;
}

int cav_handler::on_sock_connection(int index, io_struct &sock, bool connected)
{
    return 0;
}
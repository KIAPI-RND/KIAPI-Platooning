

#include <cpp-framework/sock/sock_handler.hpp>
#include <cpp-framework/gps/gps_handler.h>
#include <pq-utils/pq_handler.h>
#include <nr-v2x/nr_v2x_conf.h>
#include "server.h"
#include <cpp-framework/data/j2735.h>

server_handler::server_handler(uint32_t port) : thread(), sock(this)
{
    db_upate_tick.set(100);
    sock.tcp_bind(port);
    thread.start(this, &server_handler::progress_thread, nullptr);
}

server_handler::~server_handler()
{
    sock.tcp_close();
}

void server_handler::set_db(pq_db_config_t config)
{
    pq.connect(config);
}

int server_handler::on_sock_receive(int index, io_struct &sock)
{
    int res = 0;

    while (true)
    {
        if (sock.buffer.size() < sizeof(nr_v2x_msg_header_t))
            break;

        nr_v2x_msg_header_t *header = (nr_v2x_msg_header_t *)sock.buffer.c_str();

        int size = header->length + sizeof(nr_v2x_msg_header_t);

        if (size > sock.buffer.size())
            break;

        std::string tmp;

        tmp.append(sock.buffer.c_str(), size);

        on_rx_msg(tmp);

        sock.buffer.erase(0, size);

        res += size;
    }

    return res;
}

int server_handler::on_sock_connection(int index, io_struct &sock, bool connected)
{
    printf("sock !! connection = %d\n", connected);
    return 0;
}

bool server_handler::on_rx_msg(const std::string &tmp)
{
    nr_v2x_msg_header_t *header = (nr_v2x_msg_header_t *)tmp.c_str();

    switch (header->payload_id)
    {
    case PLATOONING_SERVER_LV_MSG:
        return on_rx_lv_msg(*((platooning_server_lv_msg_t *)tmp.c_str()));
    case PLATOONING_SERVER_FV_MSG:
        return on_rx_fv_msg(*((platooning_server_fv_msg_t *)tmp.c_str()));
    }
    return true;
}

bool server_handler::on_rx_lv_msg(const platooning_server_lv_msg_t &tmp)
{

    local_lock l(lock);

    if (cache[tmp.src_dev_id].tick < tmp.timestamp ||
        (cache[tmp.src_dev_id].timestamp + 1000) < get_epoch_time_msec())
    {
        cache[tmp.src_dev_id].lv = tmp.msg;
        cache[tmp.src_dev_id].type = PLATOONING_SERVER_LV_MSG;
        cache[tmp.src_dev_id].tick = tmp.timestamp;
        cache[tmp.src_dev_id].timestamp = get_epoch_time_msec();
        printf("lv update %d \n", tmp.src_dev_id);
    }

    cache[tmp.src_dev_id].rx_status[tmp.rx_dev_id].tick = get_epoch_time_msec();
    cache[tmp.src_dev_id].rx_status[tmp.rx_dev_id].id = tmp.rx_dev_id;
    cache[tmp.src_dev_id].rx_status[tmp.rx_dev_id].latency = tmp.rx.latency;
    cache[tmp.src_dev_id].rx_status[tmp.rx_dev_id].per = tmp.rx.per;
    cache[tmp.src_dev_id].rx_status[tmp.rx_dev_id].prr = tmp.rx.prr;
    cache[tmp.src_dev_id].rx_status[tmp.rx_dev_id].rcpi = tmp.rx.rcpi;
    cache[tmp.src_dev_id].rx_status[tmp.rx_dev_id].rssi = tmp.rx.rssi;
    cache[tmp.src_dev_id].update = true;

    return true;
}

bool server_handler::on_rx_fv_msg(const platooning_server_fv_msg_t &tmp)
{

    local_lock l(lock);

    if (cache[tmp.src_dev_id].tick < tmp.timestamp ||
        (cache[tmp.src_dev_id].timestamp + 1000) < get_epoch_time_msec())
    {
        cache[tmp.src_dev_id].fv = tmp.msg;
        cache[tmp.src_dev_id].type = PLATOONING_SERVER_FV_MSG;
        cache[tmp.src_dev_id].tick = tmp.timestamp;
        cache[tmp.src_dev_id].timestamp = get_epoch_time_msec();
        printf("fv update %d \n", tmp.src_dev_id);
    }

    cache[tmp.src_dev_id].rx_status[tmp.rx_dev_id].tick = get_epoch_time_msec();
    cache[tmp.src_dev_id].rx_status[tmp.rx_dev_id].id = tmp.rx_dev_id;
    cache[tmp.src_dev_id].rx_status[tmp.rx_dev_id].latency = tmp.rx.latency;
    cache[tmp.src_dev_id].rx_status[tmp.rx_dev_id].per = tmp.rx.per;
    cache[tmp.src_dev_id].rx_status[tmp.rx_dev_id].prr = tmp.rx.prr;
    cache[tmp.src_dev_id].rx_status[tmp.rx_dev_id].rcpi = tmp.rx.rcpi;
    cache[tmp.src_dev_id].rx_status[tmp.rx_dev_id].rssi = tmp.rx.rssi;
    cache[tmp.src_dev_id].update = true;

    return true;
}

bool server_handler::update_db(uint32_t id, const rx_data_tmp_t &val)
{
    if (!pq.connected())
        return false;

    std::string query;

    query = "INSERT INTO nr_v2x.platooning_status \
(timestamp, dev_id) \
VALUES (to_timestamp(%lf), %d) \
ON CONFLICT (dev_id) \
DO UPDATE SET timestamp=EXCLUDED.timestamp, vehicle_id=%d, vehicle_number='%s', dev_type=%d, \
latitude=%d, longitude=%d, elevation=%d, velocity=%d, heading=%u, sig_left=%d, sig_right=%d, sig_harzard=%d, \
brake=%d, accel_x=%d, accel_y=%d, yaw=%d, link_id='%s', link_pos=%d, path_plan='{%s}', rx_dev_list='{%s}'";

    std::string rx_status_str = "";

    std::string path_plan = "";

    for (auto it = val.rx_status.begin(); it != val.rx_status.end(); it++)
    {

        // dev_id, prr, per , latency, rssi, rcpi
        std::string tmp = "\"(%d,%d,%d,%d,%d,%d)\"";
        tmp = string_format(tmp, it->second.id, it->second.prr, it->second.per, it->second.latency, it->second.rssi, it->second.rcpi);

        if (rx_status_str.size() > 0)
        {
            tmp = "," + tmp;
        }

        rx_status_str += tmp;
    }

    if (val.type == PLATOONING_SERVER_FV_MSG)
    {
        for (int i = 0; i < DB_V2X_PT_FV_PATH_PLAN_MAX_LEN; i++)
        {
            if (val.fv.stFvPathPlan.adFvLatitude[i] == 0.0 && val.fv.stFvPathPlan.adFvLongitude[i] == 0.0)
                break;

            std::string tmp = "\"(%d,%d)\"";

            tmp = string_format(tmp, (int)(val.fv.stFvPathPlan.adFvLatitude[i] / 100000.0), (int)(val.fv.stFvPathPlan.adFvLongitude[i] / 100000.0));

            if (path_plan.size() > 0)
                tmp = "," + tmp;

            path_plan += tmp;
        }

        query = string_format(query, get_epoch_time_msec() * 0.001, id,
                              std::atoi(std::string((char *)val.fv.szFvVehicleId, 10).c_str()),
                              std::string((char *)val.fv.szFvVehicleNum, 20).c_str(),
                              2,
                              (int)(val.fv.dFvLatitude / 100000),
                              (int)(val.fv.dFvLongitude / 100000),
                              0,
                              J2735_DATA::Velocity_MPS::convert((uint16_t)((double)val.fv.usFvSpeed * J2735_VELOCITY_KMS_TO_MS)),
                              J2735_DATA::Heading::convert((double)val.fv.usFvHeading),
                              val.fv.stFvCan.bTurnLeftEn,
                              val.fv.stFvCan.bTurnRightEn,
                              val.fv.stFvCan.bHazardEn,
                              (int)val.fv.stFvCan.fBrakeCylinder,
                              J2735_DATA::Acceleation::convert(val.fv.stFvCan.fLongAccel),
                              J2735_DATA::Acceleation::convert(val.fv.stFvCan.fLatAccel),
                              J2735_DATA::Acceleation::convert(val.fv.stFvCan.fYawRate),
                              "", // std::string((char *)val.fv.szFvDriveLaneId, 12).c_str(),
                              0,
                              path_plan.c_str(),
                              rx_status_str.c_str());
    }
    else if (val.type == PLATOONING_SERVER_LV_MSG)
    {
        for (int i = 0; i < DB_V2X_PT_FV_PATH_PLAN_MAX_LEN; i++)
        {
            if (val.lv.stLvPathPlan.adLvLatitude[i] == 0.0 && val.lv.stLvPathPlan.adLvLongitude[i] == 0.0)
                break;

            std::string tmp = "\"(%d,%d)\"";

            tmp = string_format(tmp, (int)(val.lv.stLvPathPlan.adLvLatitude[i] / 100000.0), (int)(val.lv.stLvPathPlan.adLvLongitude[i] / 100000.0));

            if (path_plan.size() > 0)
                tmp = "," + tmp;

            path_plan += tmp;
        }

        query = string_format(query, get_epoch_time_msec() * 0.001, id,
                              std::atoi(std::string((char *)val.lv.szLvVehicleId, 10).c_str()),
                              std::string((char *)val.lv.szLvVehicleNum, 20).c_str(),
                              1,
                              (int)(val.lv.dLvLatitude / 100000),
                              (int)(val.lv.dLvLongitude / 100000),
                              0,
                              J2735_DATA::Velocity_MPS::convert((uint16_t)((double)val.lv.usLvSpeed * J2735_VELOCITY_KMS_TO_MS)),
                              J2735_DATA::Heading::convert(val.lv.usLvHeading),
                              val.lv.stLvCan.bTurnLeftEn,
                              val.lv.stLvCan.bTurnRightEn,
                              val.lv.stLvCan.bHazardEn,
                              (int)val.lv.stLvCan.fBrakeCylinder,
                              J2735_DATA::Acceleation::convert(val.lv.stLvCan.fLongAccel),
                              J2735_DATA::Acceleation::convert(val.lv.stLvCan.fLatAccel),
                              J2735_DATA::Acceleation::convert(val.lv.stLvCan.fYawRate),
                              "", // std::string((char *)val.lv.szLvLaneId, 12).c_str(),
                              0,
                              path_plan.c_str(),
                              rx_status_str.c_str());
    }

    printf("query = %s\n", query.c_str());

    pq.push_query(query);

    return true;
}

bool server_handler::insert_db(uint32_t id, const rx_data_tmp_t &val)
{
    if (!pq.connected())
        return false;

    std::string query;

    query = "INSERT INTO nr_v2x.platooning_status_log \
(timestamp, dev_id, vehicle_id, vehicle_number, dev_type, \
latitude, longitude, elevation, velocity, heading, \
sig_left, sig_right, sig_harzard, brake, accel_x, accel_y, yaw, \
link_id, link_pos, path_plan, rx_dev_list, avg_prr, avg_rssi, avg_rcpi) \
VALUES (to_timestamp(%lf), %d, %d, '%s', %d, \
%d, %d, %d, %d, %u, \
%d, %d, %d, %d, %d, %d, %d, \
'%s', %d, '{%s}', '{%s}', %d, %d, %d)";

    std::string rx_status_str = "";

    std::string path_plan = "";

    for (auto it = val.rx_status.begin(); it != val.rx_status.end(); it++)
    {

        // dev_id, prr, per , latency, rssi, rcpi
        std::string tmp = "\"(%d,%d,%d,%d,%d,%d)\"";
        tmp = string_format(tmp, it->second.id, it->second.prr, it->second.per, it->second.latency, it->second.rssi, it->second.rcpi);

        if (rx_status_str.size() > 0)
        {
            tmp = "," + tmp;
        }

        rx_status_str += tmp;
    }

    if (val.type == PLATOONING_SERVER_FV_MSG)
    {

        for (int i = 0; i < DB_V2X_PT_FV_PATH_PLAN_MAX_LEN; i++)
        {
            if (val.fv.stFvPathPlan.adFvLatitude[i] == 0.0 && val.fv.stFvPathPlan.adFvLongitude[i] == 0.0)
                break;

            std::string tmp = "\"(%d,%d)\"";

            tmp = string_format(tmp, (int)(val.fv.stFvPathPlan.adFvLatitude[i] / 100000.0), (int)(val.fv.stFvPathPlan.adFvLongitude[i] / 100000.0));

            if (path_plan.size() > 0)
                tmp = "," + tmp;

            path_plan += tmp;
        }

        query = string_format(query, get_epoch_time_msec() * 0.001, id,
                              std::atoi(std::string((char *)val.fv.szFvVehicleId, 10).c_str()),
                              std::string((char *)val.fv.szFvVehicleNum, 20).c_str(),
                              2,
                              (int)(val.fv.dFvLatitude / 100000),
                              (int)(val.fv.dFvLongitude / 100000),
                              0,
                              J2735_DATA::Velocity_MPS::convert((uint16_t)((double)val.fv.usFvSpeed * J2735_VELOCITY_KMS_TO_MS)),
                              J2735_DATA::Heading::convert((double)val.fv.usFvHeading),
                              val.fv.stFvCan.bTurnLeftEn,
                              val.fv.stFvCan.bTurnRightEn,
                              val.fv.stFvCan.bHazardEn,
                              (int)val.fv.stFvCan.fBrakeCylinder,
                              J2735_DATA::Acceleation::convert(val.fv.stFvCan.fLongAccel),
                              J2735_DATA::Acceleation::convert(val.fv.stFvCan.fLatAccel),
                              J2735_DATA::Acceleation::convert(val.fv.stFvCan.fYawRate),
                              std::string((char *)val.fv.szFvDriveLaneId, 12).c_str(),
                              0,
                              path_plan.c_str(),
                              rx_status_str.c_str(),
                              val.fv.unAvgPrr,
                              val.fv.nAvgRssi,
                              val.fv.ucAvgRcpi);
    }
    else if (val.type == PLATOONING_SERVER_LV_MSG)
    {
        for (int i = 0; i < DB_V2X_PT_FV_PATH_PLAN_MAX_LEN; i++)
        {
            if (val.lv.stLvPathPlan.adLvLatitude[i] == 0.0 && val.lv.stLvPathPlan.adLvLongitude[i] == 0.0)
                break;

            std::string tmp = "\"(%d,%d)\"";

            tmp = string_format(tmp, (int)(val.lv.stLvPathPlan.adLvLatitude[i] / 100000.0), (int)(val.lv.stLvPathPlan.adLvLongitude[i] / 100000.0));

            if (path_plan.size() > 0)
                tmp = "," + tmp;

            path_plan += tmp;
        }

        query = string_format(query, get_epoch_time_msec() * 0.001, id,
                              std::atoi(std::string((char *)val.lv.szLvVehicleId, 10).c_str()),
                              std::string((char *)val.lv.szLvVehicleNum, 20).c_str(),
                              1,
                              (int)(val.lv.dLvLatitude / 100000),
                              (int)(val.lv.dLvLongitude / 100000),
                              0,
                              J2735_DATA::Velocity_MPS::convert((uint16_t)((double)val.lv.usLvSpeed * J2735_VELOCITY_KMS_TO_MS)),
                              J2735_DATA::Heading::convert((double)val.lv.usLvHeading),
                              val.lv.stLvCan.bTurnLeftEn,
                              val.lv.stLvCan.bTurnRightEn,
                              val.lv.stLvCan.bHazardEn,
                              (int)val.lv.stLvCan.fBrakeCylinder,
                              J2735_DATA::Acceleation::convert(val.lv.stLvCan.fLongAccel),
                              J2735_DATA::Acceleation::convert(val.lv.stLvCan.fLatAccel),
                              J2735_DATA::Acceleation::convert(val.lv.stLvCan.fYawRate),
                              std::string((char *)val.lv.szLvLaneId, 12).c_str(),
                              0,
                              path_plan.c_str(),
                              rx_status_str.c_str(),
                              val.lv.unAvgPrr,
                              val.lv.nAvgRssi,
                              val.lv.ucAvgRcpi);
    }

    printf("query = %s\n", query.c_str());

    pq.push_query(query);

    return true;
}

void progress_thread(void *argv)
{ 

    local_lock l(lock);

    for (auto it = cache.begin(); it != cache.end(); it++)
    {
        uint32_t src_id = it->first;

        if (!it->second.update)
            continue;

        update_db(it->first, it->second);
        insert_db(it->first, it->second);

        it->second.rx_status.clear();
        it->second.update = false;
    }

    sleep_for(db_upate_tick.left());
}
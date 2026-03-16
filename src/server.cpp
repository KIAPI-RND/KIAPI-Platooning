

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
    

    return true;
}

bool server_handler::insert_db(uint32_t id, const rx_data_tmp_t &val)
{
     

    return true;
}

void progress_thread(void *argv)
{
  
}